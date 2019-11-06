DEBUG = 1

# Detect pkg-config on the path
PKGCONFIG := $(shell type -p pkg-config || echo NONE)

ifeq ($(PKGCONFIG), NONE)
# Hard code paths if necessary
ZLIB_PATH = /usr
ZLIB_INC = -I$(ZLIB_PATH)/include
ZLIB_LIB = -L$(ZLIB_PATH)/lib -lz
else
# Use pkg-config to detect zlib if possible
ZLIB_INC = $(shell pkg-config zlib --cflags)
ZLIB_LIB = $(shell pkg-config zlib --libs)
endif


# These should not require modification
MODULE_big = gzip
OBJS = pg_gzip.o
EXTENSION = gzip
DATA = gzip--1.0.sql
REGRESS = gzip
EXTRA_CLEAN =

PG_CONFIG = pg_config

CFLAGS += $(ZLIB_INC)
LIBS += $(ZLIB_LIB)
SHLIB_LINK := $(LIBS)

ifdef DEBUG
COPT			+= -O0 -g
endif

PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)

