Phoenix [![Build Status](https://travis-ci.org/phoenix-build/phoenix.svg?branch=master)](https://travis-ci.org/phoenix-build/phoenix) [![Gitter](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/phoenix-build/phoenix?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)
====================================
Phoenix is a high-level build system with refreshingly familiar syntax, designed
for speed and functionality.

### Really Simple Example
Assuming the `src` directory contains the source code for a C++
"Hello, world!" program, the following `Phoenixfile.phnx` will tell Phoenix how
to compile it:
```bash
$$Phoenix.checkVersion(minimum: "0.0.1");
$helloworld = CreateTarget("hello", language: "C++");
$helloworld.addSourceDirectory(["src"]);
```
