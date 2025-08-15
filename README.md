# Getting started

This extension uses [PGXS](https://www.postgresql.org/docs/current/extend-pgxs.html) to build.

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
