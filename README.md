[![Build Status](https://travis-ci.org/overpass-vpn/overpass.svg)](https://travis-ci.org/overpass-vpn/overpass)
[![Coverage Status](https://coveralls.io/repos/overpass-vpn/overpass/badge.svg?branch=master&service=github)](https://coveralls.io/github/overpass-vpn/overpass?branch=master)

# Overpass

This is the Overpass VPN. It's very much in alpha. It barely works. Don't use it.

## How to install

The easiest way to install is by using the snap:

    $ sudo snap install --beta overpass

Note that, by default, the snap won't have permission to manage the network
the way it needs to. Grant it these permissions by connecting the
`network-control` interface:

    $ sudo snap connect overpass:network-control

Now you can run Overpass with:

    $ sudo overpass.overpassd <args>


An alternative to installing the snap is to build it from source, in which case
see Hacking, below.

## How to run

Overpass requires `sudo` as it creates a virtual network interface in order to
route traffic through the VPN. The command in the snap is `overpass.overpassd`,
but if you're running from source this is just the `overpassd` binary.

`overpassd` takes a number of parameters (use `-h` to see them all), the most
important of which are:

- `--address <Overpass address>` (required)

  The address to use on the Overpass network. Note that this address needs to
  be unique among your Overpass peers (i.e. the clients you add via --client).

- `--client <Overpass IP>:<external IP>` (can be specified multiple times)

  Seed Overpass with a list of clients, mapping each client's Overpass IP
  address to their external IP address.


### Example

Say there are two clients in the Overpass network. You want Client A to use
11.11.11.2 for its Overpass IP address, and its address on the real network is
192.168.1.2. Client B will use 11.11.11.3 for its Overpass IP address, and its
address on the real network is 192.168.1.3.

On Client A, run:

    $ sudo overpass.overpassd --address 11.11.11.2 \
                              --client 11.11.11.3:192.168.1.3

Notice, you must inform it of Client B, it will not discover it automatically
(yet). Similarly, on Client B, run:

    $ sudo overpass.overpassd --address 11.11.11.3 \
                              --client 11.11.11.2:192.168.1.2

Now you should be able to visit Client A from Client B and vice-versa using
only their Overpass IP addresses.


## Hacking

Overpass has a few dependencies:

- libboost-program-options-dev
- libboost-system-dev
- libpcap-dev
- [libtins](https://github.com/mfontanini/libtins)

You can install each of these from the Ubuntu archives, except for libtins,
which must be built from source (instructions are in its README).

Once you have its dependencies, Overpass uses the CMake build system. Build like
so:

    $ mkdir build && cd build
    $ cmake path/to/overpass
    $ make

In order to run the tests, you'll need Google Mock, Google Test, gcov, and lcov.
These will be automatically found if you're on Ubuntu 16.04 or later, but if you
have the gmock source elsewhere (or aren't on Ubuntu), build like so:

    $ mkdir build && cd build
    $ cmake -DGMOCK_INCLUDE_DIR=path/to/gmock/include \
            -DGMOCK_SOURCE_DIR=path/to/gmock/ path/to/overpass

Run tests and display coverage results with:

    $ make test

Run tests and display coverage results with:

    $ make coverage

This will also generate an HTML coverage report in
build/coverage-html/index.html
