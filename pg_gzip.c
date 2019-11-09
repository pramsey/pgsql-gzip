/***********************************************************************
 *
 * Project:  PgSQL gzip/gunzip
 * Purpose:  Main file.
 *
 ***********************************************************************
 * Copyright 2019 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 ***********************************************************************/

/* Constants */
#define ZCHUNK 262144 /* 256K */

/* System */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* PostgreSQL */
#include <postgres.h>
#include <fmgr.h>
#include <funcapi.h>

/* LibZ */
#include <zlib.h>

/* Set up PgSQL */
PG_MODULE_MAGIC;

/**
* Wrap palloc in a signature that matches what zalloc expects
*/
static void*
pg_gzip_alloc(void* opaque, unsigned int items, unsigned int itemsize)
{
	return palloc(items * itemsize);
}

/**
* Wrap pfree in a signature that matches what zfree expects
*/
static void
pg_gzip_free(void* opaque, void* ptr)
{
	pfree(ptr);
	return;
}

/* Zlib defines */
#define WINDOW_BITS 15
#define ENABLE_ZLIB_GZIP 32
#define GZIP_ENCODING 16

/**
* gzip an uncompressed bytea
*/
Datum pg_gzip(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(pg_gzip);
Datum pg_gzip(PG_FUNCTION_ARGS)
{
	StringInfoData si;
	int zs_rv;
	z_stream zs;
	uint8 out[ZCHUNK];
	bytea* compressed;

	bytea* uncompressed = PG_GETARG_BYTEA_P(0);
	int32 compression_level = PG_GETARG_INT32(1);
	uint8* in = (uint8*)(VARDATA(uncompressed));
	size_t in_size = VARSIZE_ANY_EXHDR(uncompressed);

	/* compression level -1 is default best effort (approx 6) */
	/* level 0 is no compression, 1-9 are lowest to highest */
	if (compression_level < -1 || compression_level > 9)
		elog(ERROR, "invalid compression level: %d", compression_level);

	initStringInfo(&si);

	/* Prepare the z_stream state */
	zs.zalloc = pg_gzip_alloc;
	zs.zfree = pg_gzip_free;
	zs.opaque = Z_NULL;
	zs.next_in = in;
	zs.avail_in = in_size;

	if (deflateInit2(&zs,
	                 compression_level, Z_DEFLATED,
	                 WINDOW_BITS|GZIP_ENCODING, /* Magic to initialize in gzip mode */
	                 8, Z_DEFAULT_STRATEGY) != Z_OK)
		elog(ERROR, "failed to deflateInit2");

	zs.next_out = out;
	zs.avail_out = ZCHUNK;

	/* Compress until deflate stops returning output */
	zs_rv = Z_OK;
	while (zs_rv == Z_OK)
	{
		if (zs.avail_out == 0)
		{
			/* build up output in stringinfo */
			appendBinaryStringInfo(&si, (char*)out, ZCHUNK);
			zs.avail_out = ZCHUNK;
			zs.next_out = out;
		}
		zs_rv = deflate(&zs, Z_FINISH);
	}
	if (zs_rv != Z_STREAM_END)
		elog(ERROR, "compression error: %s", zs.msg ? zs.msg : "");

	appendBinaryStringInfo(&si, (char*)out, ZCHUNK - zs.avail_out);

	/* Construct output bytea */
	compressed = palloc(si.len + VARHDRSZ);
	memcpy(VARDATA(compressed), si.data, si.len);
	SET_VARSIZE(compressed, si.len + VARHDRSZ);
	PG_FREE_IF_COPY(uncompressed, 0);
	PG_RETURN_POINTER(compressed);
}


Datum pg_gunzip(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(pg_gunzip);
Datum pg_gunzip(PG_FUNCTION_ARGS)
{
	StringInfoData si;
	int zs_rv;
	z_stream zs;
	uint8 out[ZCHUNK];
	bytea* uncompressed;

	bytea* compressed = PG_GETARG_BYTEA_P(0);
	uint8* in = (uint8*)(VARDATA(compressed));
	size_t in_size = VARSIZE_ANY_EXHDR(compressed);

	initStringInfo(&si);

	/* Prepare the z_stream state */
	zs.zalloc = pg_gzip_alloc;
	zs.zfree = pg_gzip_free;
	zs.opaque = Z_NULL;
	/* Magic to initialize in gzip mode */
	if (inflateInit2(&zs, WINDOW_BITS|ENABLE_ZLIB_GZIP) != Z_OK)
		elog(ERROR, "failed to inflateInit");

	/* Point z_stream to input and output buffers */
	zs.next_in = in;
	zs.avail_in = in_size;
	zs.next_out = out;
	zs.avail_out = ZCHUNK;

	/* Decompress until inflate stops returning output */
	zs_rv = Z_OK;
	while (zs_rv == Z_OK)
	{
		if (zs.avail_out == 0)
		{
			/* build up output in stringinfo */
			appendBinaryStringInfo(&si, (char*)out, ZCHUNK);
			zs.avail_out = ZCHUNK;
			zs.next_out = out;
		}
		zs_rv = inflate(&zs, Z_FINISH);
	}

	if (zs_rv != Z_STREAM_END)
		elog(ERROR, "decompression error: %s", zs.msg ? zs.msg : "");

	appendBinaryStringInfo(&si, (char*)out, ZCHUNK - zs.avail_out);

	/* Construct output bytea */
	uncompressed = palloc(si.len + VARHDRSZ);
	memcpy(VARDATA(uncompressed), si.data, si.len);
	SET_VARSIZE(uncompressed, si.len + VARHDRSZ);
	PG_FREE_IF_COPY(compressed, 0);
	PG_RETURN_POINTER(uncompressed);
}
