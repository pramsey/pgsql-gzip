#!/bin/bash

PG_VERSION=$1
UBUNTU=bionic

set -x

/etc/init.d/postgresql stop

for VER in 9.1 9.2 9.3 9.4 9.5 9.6 10 11 12 13; do
	apt-get -y --purge remove postgresql-9.1
done

apt-get -y --purge remove postgresql-common
apt-get -y autoremove
rm -rf /var/lib/postgresql

apt-key adv --fetch-keys https://www.postgresql.org/media/keys/ACCC4CF8.asc
add-apt-repository "deb http://apt.postgresql.org/pub/repos/apt/ ${UBUNTU}-pgdg main ${PG_VERSION}"
apt-get update -qq
apt-get -y install postgresql-${PG_VERSION} postgresql-server-dev-${PG_VERSION}
cp ci/pg_hba.conf /etc/postgresql/$PG_VERSION/main/pg_hba.conf
/etc/init.d/postgresql reload
