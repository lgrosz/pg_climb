PGFILEDESC = "Extension for rock climbing"
EXTENSION = pg_climb
DATA = pg_climb--0.1.sql
MODULE_big = pg_climb
OBJS = pg_climb.o pg_climb_module.o
REGRESS = pg_climb
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
