
# Where are include/zlib.h and lib/libz.so installed?
ZLIB_PATH = /usr
ZLIB_INC_PATH = $(ZLIB_PATH)/include
ZLIB_LIB_PATH = $(ZLIB_PATH)/lib

# This should not require modification
MODULE_big = gzip
OBJS = pg_gzip.o
EXTENSION = gzip
DATA = gzip--1.0.sql
REGRESS = gzip
EXTRA_CLEAN =

CURL_CONFIG = curl-config
PG_CONFIG = pg_config

CFLAGS += -I$(ZLIB_INC_PATH)/include
LIBS += -L$(ZLIB_LIB_PATH)/lib -lz
SHLIB_LINK := $(LIBS)

ifdef DEBUG
COPT			+= "-O0 -g"
endif

PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)

