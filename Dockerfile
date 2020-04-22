# This argument will usually be set by the Makefile. Override example:
#  make deb-docker base=debian:latest
ARG BASE_IMAGE=debian:sid
FROM ${BASE_IMAGE}

ARG BASE_IMAGE
RUN set -eux  ;\
    echo "Using BASE_IMAGE=$BASE_IMAGE"  ;\
    DEBIAN_FRONTEND=noninteractive apt-get update -qq  ;\
    DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
        build-essential \
        fakeroot \
        pkg-config \
        debhelper \
        devscripts \
        zlib1g-dev \
        postgresql-server-dev-all

WORKDIR /build/pgsql-gzip
