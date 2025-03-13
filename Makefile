# extension
PGFILEDESC = "Extension for rock climbing"
EXTENSION = pg_climb
DATA = pg_climb--0.1.sql
MODULE_big = pg_climb
OBJS = pg_climb.o pg_climb_module.o
REGRESS = pg_climb
PG_CONFIG = pg_config
ifeq ($(COVERAGE),yes)
PG_CFLAGS += -fprofile-arcs -ftest-coverage --coverage
endif
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)

# unit tests
PKG_CONFIG = pkg-config
UNIT_CFLAGS = $(shell $(PKG_CONFIG) -cflags check)
UNIT_LDFLAGS = $(shell $(PKG_CONFIG) -libs check)
ifeq ($(COVERAGE),yes)
UNIT_LDFLAGS += --coverage
endif
UNIT_EXEC = test_pg_climb
UNIT_SOURCES = test_pg_climb.c pg_climb.o

ifeq ($(COVERAGE),yes)
pg_climb.gcno: pg_climb.o

pg_climb.gcda: check-unit

.PHONY: coverage
coverage: pg_climb.gcno pg_climb.gcda
	gcovr --filter pg_climb.c
endif

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

clean-coverage:
	rm -f *.gcno *.gcda

.PHONY: clean-all
clean-all: clean clean-unit clean-coverage
