#!/bin/bash

PGVERSION=12

set -x

curl "https://salsa.debian.org/postgresql/postgresql-common/-/raw/master/pgdg/apt.postgresql.org.sh" \
	> /usr/local/bin/apt.postgresql.org.sh
chmod 755 /usr/local/bin/apt.postgresql.org.sh
/usr/local/bin/apt.postgresql.org.sh -i -v $PGVERSION
pg_createcluster --start $PGVERSION test -p 5432 -- -A trust

