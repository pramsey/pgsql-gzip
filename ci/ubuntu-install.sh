#!/bin/bash

PGVERSION=$1

set -x

curl "https://salsa.debian.org/postgresql/postgresql-common/-/raw/master/pgdg/apt.postgresql.org.sh" \
	> /usr/local/bin/apt.postgresql.org.sh
chmod 755 /usr/local/bin/apt.postgresql.org.sh

for VER in 10 11 12 13; do

	apt-get -y --purge remove postgresql-$VER postgresql-server-dev-$VER

done

apt-get -y --purge remove postgresql-common

/usr/local/bin/apt.postgresql.org.sh -i -v $PGVERSION

pg_createcluster $PGVERSION main -p 5432 -- -A trust
pg_ctlcluster $PGVERSION main start
