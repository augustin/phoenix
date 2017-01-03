This document contains a basic introduction to Phoenix's internal architecture and
implementation.

### Directory layout
 
 - `data`: Data files which Phoenix uses at runtime (and are thus installed alongside the Phoenix binary).
  - `languages`: The built-in programming language definition files.
 - `docs`: All internal and external documentation for Phoenix, which is mirrored as its GitHub wiki.
 - `src`: The source code.
  - `build`: Core build system logic.
    - `generators`: All the build-system generators, both primary and secondary.
  - `script`: The scripting engine.
  - `util`: Utility classes implementing generic functionality that C++ lacks.
 - `tests`: Source code for the unit tests.
  - `script-tests`: Testcases for the scripting engine.

### Utility classes
At present, the utility classes are:

 - `FSUtil`: Filesystem utilites (file I/O, directory traversing, path normalization, filesearch, "which").
 - `OSUtil`: Operating system utilities (OS name, subprocess execution, environment variables).
 - `PrintUtil`: stdout/stderr management (colored error/warning messages, "checking..." messages).
 - `StringUtil`: `std::string` manipulation (split/join, trim, startsWith/endsWith, replaceAll).
 - `XmlUtil`: Quick'n'easy generation of XML files.

All of the classes are entirely composed of static members, with the exception of `XmlUtil`. They all have their unit tests in the `UtilTest.cpp` file.

### Scripting engine
Phoenix's scripting engine is composed of a reference-counted object system (by using `std::shared_ptr<>` to wrap a custom Object class), a trivial `std::vector<>` based variable stack, and a hand-written interpreter.

The interpreter has no abstract syntax tree *per se*, but operates specific syntax lists. It is single-stage, meaning that the expression tokenizing, parsing, and evaluating all takes place at the same time, with no intermediate representations. Thus it is more or less a recursive-descent tokenizer-parser with evaluation taking place at the same time parsing does.

This design has a number of odd side effects, such as that it is practically impossible without (ab)using C++ exceptions to unwind the stack in the case of `return`, `break`, and other scope-changing keywords; and that it has to re-tokenize functions and loops every time they are executed. But it is very compact (~900 SLoC for the entire interpreter) and easy to modify and maintain, which is why this model was chosen.
