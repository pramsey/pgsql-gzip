FROM debian:sid

RUN apt-get update -qq && \
  apt-get install -y \
    build-essential \
    fakeroot \
    pkg-config \
    devscripts \
    zlib1g-dev \
    postgresql-server-dev-all

WORKDIR /build/pgsql-gzip
