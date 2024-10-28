## This repo is archived for prosperity

While ethsnarks has been of use to many, and at the time did fit a niche in the Ethereum ecosystem, unfortunately I don't have time to maintain the project and would warn people away from using Ethsnarks as the basis for their project. Many other projects have been developed since its inception that do everything a lot better, have been reviewed by more developers, have more active support - or have simply jumped ahead leaps & bounds to take advantage of new developments made in the past 5 years.

For more information, try some of these awesome lists:

 * https://github.com/ventali/awesome-zk
 * https://github.com/LuozhuZhang/Awesome-zkEVM
 * https://github.com/matter-labs/awesome-zero-knowledge-proofs

Please note, the PyPi package at https://pypi.org/project/ethsnarks/ should also be considered deprecated.

Thanks to all who've been involved, everybody who provided inspiration and feedback, the Ethereum foundation, everybody who used it or was inspired to make better things. ZK is here, on desktop, on mobile, on-chain, in browsers it's amazing to see how much progress there has been, and I'm excited to see how much more progress there will be in the next 5 years.

----------------

# EthSnarks

[![Join the chat at https://gitter.im/ethsnarks](https://badges.gitter.im/ethsnarks.png)](https://gitter.im/ethsnarks?utm_source=share-link&utm_medium=link&utm_campaign=share-link)

Zero-Knowledge proofs are coming to Ethereum and Dapps in 2019!

EthSnarks is a collection of zkSNARK circuits and supporting libraries to use them with Ethereum smart contracts, it aims to help solve one of the biggest problems facing zkSNARKS on Ethereum - cross-platform on desktop, mobile and in-browser, cheap enough to run on-chain, and with algorithms that significantly reduces the time it takes to run the prover.

The notable advantages of using EthSnarks are:

 * Reduced cost, 500k gas with 1 input, using [Groth16](https://eprint.iacr.org/2016/260.pdf).
 * Prove zkSNARKs in-browser, with WebAssembly and Emscripten
 * Linux, Mac and Windows builds
 * Solidity, Python and C++ support in one place
 * A growing library of gadgets and algorithms

EthSnarks is participating in the Ethereum Foundation's grants program, development will continue full-time and we will be working with companies and developers to help overcome the common challenges and hurdles that we all face. Get in-touch for more information.

**WARNING: EthSnarks is beta quality software, improvements and fixes are made frequently, and documentation doesn't yet exist**

## Examples

 * [Miximus - a self-service coin mixer and anonymous transfer method for Ethereum](https://github.com/HarryR/ethsnarks-miximus)
 * [Hopper: an Open-Source Mixer for Mobile-friendly private transfers on Ethereum](https://github.com/argentlabs/hopper)
 * [Example implementations of ethsnarks](https://github.com/LayerXcom/ethsnarks-examples)
 * [An example of a zero-knowledge-proof of a SHA256 pre-image for Ethereum](https://github.com/Ethsnarks/ethsnarks-hashpreimage)

## Building

[![Build Status](https://travis-ci.org/HarryR/ethsnarks.svg?branch=master)](https://travis-ci.org/HarryR/ethsnarks) [![Build status](https://ci.appveyor.com/api/projects/status/yk08x7xtk9te10vo/branch/master?svg=true)](https://ci.appveyor.com/project/harryr/ethsnarks/branch/master)

### Unix Flavours (Linux, OSX, Ubuntu, CentOS etc.)

The following dependencies are required to build Ethsnarks:

 * cmake
 * g++ or clang++
 * gmp
 * npm / nvm

Check-out the source-code using:

```bash
git clone git@github.com:HarryR/ethsnarks.git && cd ethsnarks
```

After checking-out the repository you need to install the necessary dependencies, the `Makefile` includes pre-determined rules for different platforms, you need to run this as `root` or an administrator user (i.e. for Brew on OSX):

 * `make fedora-dependencies` (CentOS, Fedora, RHEL etc. requires `dnf`)
 * `make ubuntu-dependencies` (Ubuntu, Debian etc. requires `apt-get`)
 * `make mac-dependencies` (OSX, requires [Homebrew](https://brew.sh/))

Then install the Python dependencies, via Pip, into the local user directory:

 * `make python-dependencies`

Then build and test the project:

 * `git submodule update --init --recursive`
 * `make`

### Windows (64-bit)

Install MSYS2 from https://www.msys2.org/ then open the MSYS2 Shell and run:

```bash
pacman --noconfirm -S make gmp gmp-devel gcc git cmake
git clone git@github.com:HarryR/ethsnarks.git
cd ethsnarks
git submodule update --init --recursive
cmake -E make_directory build
cmake -E chdir build cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build build
```

Building under 32-bit MinGW32, MSYS (not MSYS2) and Microsoft Visual Studio may be supported in future depending upon demand, but currently it's probably broken...

### WASM / Browser

WebAssembly, WASM and JavaScript builds are partially supported via [ethsnarks-emscripten](https://github.com/harryr/ethsnarks-emscripten) and [ethsnarks-cheerp](https://github.com/Ethsnarks/ethsnarks-cheerp). The build process is similar, but using the Emscripten and Cheerp toolchains.

# Requests and Contributions

This project aims to help create an ecosystem where a small number of well tested but simple zkSNARK circuits can be easily integrated into your project without having to do all of the work up-front.

If you have any ideas for new components, please [Open an issue](https://github.com/HarryR/ethsnarks/issues/new), or submit a pull request.

# Gadgets

We are surely increasing the range of gadgets, supporting libraries, available documentation and examples; at the moment the best way to find out how to use something is to dig into the code or ask questions via a [new issue](https://github.com/HarryR/ethsnarks/issues/new?labels=question,help%20wanted)

The following gadgets are available

 * 1-of-N
 * [2-bit lookup table](src/gadgets/lookup_2bit.cpp)
 * [3-bit lookup table](src/gadgets/lookup_3bit.cpp)
 * [MiMC](https://eprint.iacr.org/2016/492) hash and cipher
 * [Poseidon](https://eprint.iacr.org/2019/458.pdf) hash function
 * [Miyaguchi-Preneel one-way function](https://en.wikipedia.org/wiki/One-way_compression_function)
 * Merkle tree
 * SHA256 (Ethereum compatible, full round)
 * [Shamir's Secret Sharing Scheme](https://en.wikipedia.org/wiki/Shamir%27s_Secret_Sharing)
 * 'Baby JubJub' twisted Edwards curve
   * EdDSA
   * Pedersen hash

## Maintainers

[@HarryR](https://github.com/HarryR)
