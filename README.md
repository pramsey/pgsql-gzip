# PostgreSQL gzip/gunzip Functions

[![Build Status](https://api.travis-ci.org/pramsey/pgsql-gzip.svg?branch=master)](https://travis-ci.org/pramsey/pgsql-gzip)

## Motivation

Sometimes you just need to compress your `bytea` object before you return it to the client.

Sometimes you receive a compressed `bytea` from the client, and you have to uncompress it before you can work with it.

This extension is for that.

This extension is **not** for storage compression. PostgreSQL already does [tuple compression](https://www.postgresql.org/docs/current/storage-toast.html) on the fly if your tuple gets large enough, manually pre-compressing your data using this function won't make things smaller.

## Examples

    > SELECT gzip('this is my this is my this is my this is my text');

                                       gzip
    --------------------------------------------------------------------------
     \x1f8b08000000000000132bc9c82c5600a2dc4a851282ccd48a12002e7a22ff30000000

Wait, what, the compressed output is longer?!? No, it only **looks** that way, because in hex every byte is represented with two hex digits. The original string looks like this in hex:

    > SELECT 'this is my this is my this is my this is my text'::bytea;

                                                   bytea
    ----------------------------------------------------------------------------------------------------
     \x74686973206973206d792074686973206973206d792074686973206973206d792074686973206973206d792074657874

For really long, repetitive things, compression naturally works like a charm:

    > SELECT gzip(repeat('this is my ', 100));

                                                   bytea
    ----------------------------------------------------------------------------------------------------
     \x1f8b08000000000000132bc9c82c5600a2dc4a859251e628739439ca24970900d1341c5c4c040000

To convert a `bytea` back into an equivalent `text` you must use the `encode()` function with the `escape` encoding.

    > SELECT encode('test text'::bytea, 'escape');
       encode
    -----------
     test text

    > SELECT encode(gunzip(gzip('this text has been compressed and then decompressed')), 'escape')

                          encode
    -----------------------------------------------------
     this text has been compressed and then decompressed


## Functions

* `gzip(uncompressed BYTEA, [compression_level INTEGER])` returns `BYTEA`
* `gzip(uncompressed TEXT, [compression_level INTEGER])` returns `BYTEA`
* `gunzip(compressed BYTEA)` returns `BYTEA`


## Installation

### UNIX

If you have PostgreSQL devel packages (`postgresql-server-dev-all`) and zlib (`zlib1g-dev`) installed, you should have `pg_config` on your path, so you should be able to just run `make`, then `make install`, then in your database `CREATE EXTENSION gzip`.

If your `libz` is installed in a non-standard location, you may need to edit `ZLIB_PATH` in the `Makefile`.

### Debain/Ubuntu

```bash
sudo apt-get install build-essential zlib1g-dev postgresql-server-dev-all pkg-config
make
make install
psql ... -c "CREATE EXTENSION gzip"
```

To build the DEB package you will also need `fakeroot` and `devscripts`. See [Dockerfile](./Dockerfile) for a full list.

```bash
sudo apt-get install build-essential zlib1g-dev postgresql-server-dev-all pkg-config fakeroot devscripts
make
make deb
dpkg -i <path to package>.deb
```

And you will be able to run the `make deb` and get the packege wich can be installed with ``


#### deb package build using Docker

Makefile has targets for building the DEB package using Docker image with different base images. This approach only requires `make` and `docker` to be available on the host.

```bash
make deb-latest   # Uses debian:latest
make deb-sid      # Uses debian:sid
```

A custom base image and extra dependencies could also be supplied with a parameters:

```bash
make deb-docker base=ubuntu:latest extras="pkg1 pkg2 ..."
```
