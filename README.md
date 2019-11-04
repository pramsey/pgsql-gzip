# PostgreSQL GZIP/GUNZIP Functions

[![Build Status](https://api.travis-ci.org/pramsey/pgsql-gzip.svg?branch=master)](https://travis-ci.org/pramsey/pgsql-gzip)

## Motivation

Sometimes you just need to compress your `bytea` object before you return it to the client.

Sometimes you receive a compressed binary object from the client, and you have to uncompress it to do something useful.

This extension is for that.

## Examples

    > SELECT gzip('this is my this is my this is my this is my text');

                           gzip
    --------------------------------------------------
     \x789c2bc9c82c5600a2dc4a851282ccd48a1200a382112e


    > SELECT encode(gunzip('\x789c2bc9c82c5600a2dc4a851282ccd48a1200a382112e'), 'escape')

                          encode
    --------------------------------------------------
     this is my this is my this is my this is my text
    (1 row)


## Functions

* `gzip(uncompressed BYTEA, [compression_level INTEGER])` returns `BYTEA`
* `gunzip(compressed BYTEA)` returns `BYTEA`


## Installation

### UNIX

If you have PostgreSQL devel packages and zlib installed, you should have `pg_config` on your path, so you should be able to just run `make`, then `make install`, then in your database `CREATE EXTENSION gzip`.

If your `libz` is installed in a non-standard location, you may need to edit `ZLIB_PATH`.
