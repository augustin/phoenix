Phoenix [![Build Status](https://travis-ci.org/phoenix-build/phoenix.svg?branch=master)](https://travis-ci.org/phoenix-build/phoenix) [![Gitter](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/phoenix-build/phoenix?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)
====================================
Phoenix is a high-level build system with refreshingly familiar syntax, designed for speed and functionality.

### Simple example
Assuming the `src` directory contains the source code for a C++ "Hello, world!" program, the following `Phoenixfile.phnx` will tell Phoenix how to compile it:
```bash
$$Phoenix.checkVersion(minimum: "0.0.1");
$helloworld = CreateTarget("hello", language: "C++");
$helloworld.addSourceDirectory("src");
```

## Getting started
### Installing
Phoenix requires a limited subset of C++11, but should still compile with GCC 4.6 or better. Phoenix should build and run on all systems where this dependency is met, but it may require tweaking for OS-specific paths and locations.

One-liner templates to compile Phoenix:
 * Any UNIX shell: `g++ $(find src -name "*.cpp") -Isrc -o phoenix_bootstrapped -std=c++0x -O2`
 * Windows GCC/PowerShell: `g++ $((Get-ChildItem src -Filter *.cpp -Recurse | % { $_.FullName } | Resolve-Path -Relative) -replace '\s+', ' ').split() -Isrc -o phoenix_bootstrapped -std=c++0x -O2`

### Documentation
Currently, the only documentation is the [Complete Syntax and Function Reference](https://github.com/phoenix-build/phoenix/wiki/Complete-Syntax-and-Function-Reference).
