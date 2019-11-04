# PostgreSQL GZIP/GUNZIP Functions

[![Build Status](https://api.travis-ci.org/pramsey/pgsql-gzip.svg?branch=master)](https://travis-ci.org/pramsey/pgsql-gzip)

## Motivation

Sometimes you just need to compress your `bytea` object before you return it to the client.

Sometimes you receive a compressed binary object from the client, and you have to uncompress it to do something useful.

This extension is for that.

## Examples

    > SELECT gzip('this is my this is my this is my this is my text');

                                       gzip
    --------------------------------------------------------------------------
     \x1f8b08000000000000132bc9c82c5600a2dc4a851282ccd48a12002e7a22ff30000000

What, it's longer? No, it only looks that way, because in hex every character requires two hex digits. The original string looks like this in hex (it's longer):

    > SELECT 'this is my this is my this is my this is my text'::bytea;

                                                   bytea
    ----------------------------------------------------------------------------------------------------
     \x74686973206973206d792074686973206973206d792074686973206973206d792074686973206973206d792074657874

And for really long, repetitive things, compression naturally works like a charm:

    > SELECT gzip(repeat('this is my ', 100));

                                                   bytea
    ----------------------------------------------------------------------------------------------------
     \x1f8b08000000000000132bc9c82c5600a2dc4a859251e628739439ca24970900d1341c5c4c040000

Converting a `bytea` back into an equivalent `text` uses the `encode()` function with the `escape` encoding.

    > SELECT encode(gunzip(gzip('this is my this is my this is my this is my text')), 'escape')

                          encode
    --------------------------------------------------
     this is my this is my this is my this is my text


## Functions

* `gzip(uncompressed BYTEA, [compression_level INTEGER])` returns `BYTEA`
* `gzip(uncompressed TEXT, [compression_level INTEGER])` returns `BYTEA`
* `gunzip(compressed BYTEA)` returns `BYTEA`


## Installation

### UNIX

If you have PostgreSQL devel packages and zlib installed, you should have `pg_config` on your path, so you should be able to just run `make`, then `make install`, then in your database `CREATE EXTENSION gzip`.

If your `libz` is installed in a non-standard location, you may need to edit `ZLIB_PATH` in the `Makefile`.
