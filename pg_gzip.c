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
* Wrap palloc in a signature that matches what zalloc
* expects
*/
static void*
pg_gzip_alloc(void* opaque, unsigned int items, unsigned int itemsize)
{
	return palloc(items * itemsize);
}

/**
* Wrap pfree in a signature that matches what zfree
* expects
*/
static void
pg_gzip_free(void* opaque, void* ptr)
{
	pfree(ptr);
	return;
}

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
	bytea *uncompressed = PG_GETARG_BYTEA_P(0);
	int32 compression_level = PG_GETARG_INT32(1);
	uint8* in = (uint8*)(uncompressed->vl_dat);
	size_t in_size = VARSIZE_ANY_EXHDR(uncompressed);
	uint8 out[ZCHUNK];
	bytea *compressed;

	initStringInfo(&si);

	/* Prepare the z_stream state */
	zs.zalloc = pg_gzip_alloc;
	zs.zfree = pg_gzip_free;
	zs.opaque = Z_NULL;
	if (deflateInit(&zs, compression_level) != Z_OK)
		elog(ERROR, "failed to deflateInit");

	zs.next_in = in;
	zs.avail_in = in_size;
	zs.next_out = out;
	zs.avail_out = ZCHUNK;

    /* compress until end of bytea */
	zs_rv = Z_OK;
	while (zs_rv == Z_OK)
	{
		if (zs.avail_out == 0)
		{
			appendBinaryStringInfo(&si, (char*)out, ZCHUNK);
			zs.avail_out = ZCHUNK;
			zs.next_out = out;
		}
		zs_rv = deflate(&zs, Z_FINISH);
	}
	assert(zs_rv == Z_STREAM_END);
	appendBinaryStringInfo(&si, (char*)out, ZCHUNK - zs.avail_out);
	compressed = palloc(si.len + VARHDRSZ);
	memcpy(compressed->vl_dat, si.data, si.len);
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

	bytea *compressed = PG_GETARG_BYTEA_P(0);
	uint8* in = (uint8*)(compressed->vl_dat);
	size_t in_size = VARSIZE_ANY_EXHDR(compressed);
	uint8 out[ZCHUNK];
	bytea *uncompressed;

	initStringInfo(&si);

	/* Prepare the z_stream state */
	zs.zalloc = pg_gzip_alloc;
	zs.zfree = pg_gzip_free;
	zs.opaque = Z_NULL;
	if (inflateInit(&zs) != Z_OK)
		elog(ERROR, "failed to inflateInit");

	zs.next_in = in;
	zs.avail_in = in_size;
	zs.next_out = out;
	zs.avail_out = ZCHUNK;

    /* decompress until end of bytea */
	zs_rv = Z_OK;
	while (zs_rv == Z_OK)
	{
		if (zs.avail_out == 0)
		{
			appendBinaryStringInfo(&si, (char*)out, ZCHUNK);
			zs.avail_out = ZCHUNK;
			zs.next_out = out;
		}
		zs_rv = inflate(&zs, Z_FINISH);
	}
	assert(zs_rv == Z_STREAM_END);
	appendBinaryStringInfo(&si, (char*)out, ZCHUNK - zs.avail_out);
	uncompressed = palloc(si.len + VARHDRSZ);
	memcpy(uncompressed->vl_dat, si.data, si.len);
	SET_VARSIZE(uncompressed, si.len + VARHDRSZ);
	PG_FREE_IF_COPY(compressed, 0);
	PG_RETURN_POINTER(uncompressed);
}
