PGFILEDESC = "Extension for rock climbing"
EXTENSION = pg_climb
DATA = pg_climb--0.1.sql
MODULES = pg_climb
REGRESS = pg_climb
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
