[![Build Status](https://travis-ci.org/overpass-vpn/overpass.svg)](https://travis-ci.org/overpass-vpn/overpass)
[![Coverage Status](https://coveralls.io/repos/overpass-vpn/overpass/badge.svg?branch=master&service=github)](https://coveralls.io/github/overpass-vpn/overpass?branch=master)

# Overpass

## Hacking

Overpass uses the CMake build system. Build like so:

    $ mkdir build && cd build
    $ cmake path/to/overpass
    $ make

In order to run the tests, you'll need Google Mock, Google Test, gcov, and lcov.
These will be automatically found if you're on Ubuntu 14.04 or later, but if you
have the gmock source elsewhere (or aren't on Ubuntu), build like so:

    $ mkdir build && cd build
    $ cmake -DGMOCK_INCLUDE_DIR=path/to/gmock/include \
            -DGMOCK_SOURCE_DIR=path/to/gmock/ path/to/overpass

Run tests with:

    $ make test

Run tests and display coverage results with:

    $ make coverage
