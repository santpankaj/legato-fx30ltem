ima-support-tools
=================

This repository provide some scripts used to help with IMA.

Content
-------

- `ima-gen-keys.sh`:
   Script to manage the generation of keys & certificates

- `ima-sign.sh`:
   Script to sign files.

Requirements
------------

To use these scripts, you will need:

For `ima-gen-keys.sh`:
- [OpenSSL](https://www.openssl.org/)

For `ima-sign.sh`:
- [IMA/EVM Utils](https://sourceforge.net/projects/linux-ima/)
- [bsdtar](https://www.libarchive.org/)

For tests:
- [bats](https://github.com/sstephenson/bats)
- [fakeroot](https://wiki.debian.org/FakeRoot)

Tests
-----

As to test modifications, just run:
```bash
./tests.sh
```
