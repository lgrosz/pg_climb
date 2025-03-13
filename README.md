# Getting started

This extension uses [PGSX](https://git.postgresql.org/gitweb/?p=postgresql.git;a=blob;f=src/makefiles/pgxs.mk) to build.

```sh
make               # build
make check-unit    # run unit tests
make install       # install
make installcheck  # run regression tests
```

Coverage is disabled by default... with a clean build, get coverage by

```sh
COVERAGE=yes make coverage
```
