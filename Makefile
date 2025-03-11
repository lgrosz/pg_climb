# extension
PGFILEDESC = "Extension for rock climbing"
EXTENSION = pg_climb
DATA = pg_climb--0.1.sql
MODULE_big = pg_climb
OBJS = pg_climb.o pg_climb_module.o
REGRESS = pg_climb
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)

# unit tests
PKG_CONFIG = pkg-config
UNIT_CFLAGS = $(shell $(PKG_CONFIG) -cflags check)
UNIT_LDFLAGS = $(shell $(PKG_CONFIG) -libs check)
UNIT_EXEC = test_pg_climb
UNIT_SOURCES = test_pg_climb.c pg_climb.o

.PHONY: test
tests: $(UNIT_EXEC)

$(UNIT_EXEC): test_pg_climb.c pg_climb.o
	$(CC) $(UNIT_CFLAGS) $(UNIT_SOURCES) $(UNIT_LDFLAGS) -o $(UNIT_EXEC)

.PHONY: check-unit
check-unit: $(UNIT_EXEC)
	./$(UNIT_EXEC)

.PHONY: check-unit
clean-unit:
	rm -f ./$(UNIT_EXEC)

.PHONY: clean-all
clean-all: clean clean-unit
