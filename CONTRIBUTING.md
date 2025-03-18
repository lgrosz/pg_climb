## How to release

### Patch versions

No release process

### Major and minor versions

1. Modify the `default_version` field in [pg_climb.control](./pg_climb.control)
2. Create a null-to-new version upgrade script, e.g. `pg_climb--<new>.sql`
3. Create old-to-new version upgrade script(s), e.g. `pg_climb--<old>--<new>.sql`
4. Add regress test(s) for the upgrade

## Versioning considerations

This project must follow a strict versioning philosophy to ensure upgrade stability.

1. Patch versions are for backward compatible bug fixes or internal changes and must
    - have no schema changes
    - be binary compatible with previous patch versions released under the same minor version
2. Minor versions are for backward compatible functionality changes to the public API and must
    - serve any backward compatible schema changes in the form of upgrade scripts
    - be binary compatible with previous minor versions released under the same major version
3. Major versions are for any backward incompatible changes to the public API
