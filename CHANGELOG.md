# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [unreleased]

## [1.0.0] - 2025-08-17

### Added

- Add type for `grade`
- Add `typemod` support for `grade`. This allows grades to be restricted to
  particular formats when creating columns.
- `grade_type` function allows querying the grade types. These values are akin
  to the `typmod` values.
- Implement B-tree operators for `grade`. Comparisons between grade types have
  an undefined sorting order.
- Grade formats (and their `typmod`s) supported are
  - V-grade (`'verm'`)
  - Font grade (`'font'`)
  - Yosemite Decimal System (`'yds'`)

[unreleased]: https://github.com/lgrosz/pg_climb/compare/v1.0.0...HEAD
[1.0.0]: https://github.com/lgrosz/pg_climb/releases/tag/v1.0.0
