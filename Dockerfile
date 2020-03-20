# This argument will usually be set by the Makefile. Override example:
#  BASE_IMAGE=debian:latest make deb-in-docker
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

ARG EXTRA_DEPS=""
RUN sh -c "[ -z '$EXTRA_DEPS' ] && echo 'No exrta dependencies were specified with EXTRA_DEPS build argument' || \
    ( \
        set -eux ;\
        echo 'Installing extra dependencies $EXTRA_DEPS' ;\
        export DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
          $EXTRA_DEPS  ;\
    )"

WORKDIR /build/pgsql-gzip
