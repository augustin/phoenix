---
title: Introduction to Phoenix

language_tabs:

toc_footers:
  - Spotted something off-color? [Fork us on GitHub].

includes:
  - errors

search: true
---

Introduction to Phoenix
=====================================
Welcome! This guide serves as an introduction to the Phoenix build system. We expect you at least
understand the fundamental concepts of build systems, and hopefully have used at least one or two
in the past. If not, you might want to [read up a bit](https://en.wikipedia.org/wiki/Build_automation)
before beginning.

<aside class="notice">
It's best if you *do* this guide (not just read it) all in a single sitting. It'll
probably take you around 20 minutes.
</aside>

Installed Phoenix? Good. Let's begin.

# The Basics

## Hello, world!

```shell
print("Hello, world!");
```

Phoenix's syntax is similar to that of C. Lines don't begin with anything special, and end in
semicolons (`;`). Strings are defined by double-quotes (`"`) and single-quotes (`'`, for literal
strings). And `print` is a global, built-in function.

If you save this program as `Phoenixfile.phnx` and run it (using `phoenix .`), you should see the
following output:
```
$ phoenix .
Hello, world!
```

If you don't, something's wrong! You might not have installed Phoenix correctly.

## Hello, variable world!

```shell
$thing = "Phoenix";
print("Hello, $thing!");
# prints: "Hello, Phoenix!"
```

Variables are denoted by a preceding dollar-sign `$`, like in Shell or PHP. All alphanumeric
characters as well as `_` are acceptable for use in variable names. Inside strings, the end of a
variable is denoted by any character that's not legal to have in a variable name, but in case you need
to explicitly denote the end of a variable, you can use curly braces (`{}`):

```shell
$thing = "car";
print("The plural of '$thing' is '${thing}s'.");
# prints: "The plural of 'car' is 'cars'."
```

## Arrays, operators, built-in types

```shell
$array = ["Hello"];
$array += "array";
print("$array");
# prints two lines, one with "Hello" and one with "array"
print($array);
# prints: "Array(2): ['Hello', 'array']"
print($array.size - $array.length);
# prints "0" (`size` and `length` are synonyms)

$string = $array.join(", ") + "!";
print("Hello, $string!");
# prints: "Hello, array!"
```

Arrays are created with brackets (`[]`). They can be joined using the `join` function, which returns
a string.

# Creating targets

Up until now we've only toyed with Phoenix as a language. Now let's use it to actually
build something.

Create a "Hello, world!" demo in one of the languages that Phoenix supports (you pick which one!)
and save it somewhere. For example, here's one for C++:
```cpp
#include <iostream>

int main()
{
	std::cout << "Hello, world!";
	return 0;
}
```
Create a `Phoenixfile.phnx` alongside it and edit it. We want to create one executable called
`hello` from our source file, so we use the `CreateTarget` function:

```shell
$hello = CreateTarget("hello", language: "C++");
$hello.addSourceDirectory(".");
```

There are a couple new concepts here, so let's review them:
 - `CreateTarget` is a global, built-in function that returns an object
 - The second parameter of our call to `CreateTarget` is indeed named. By default, the first
 parameter to any function does *not* need to be named, but you can name it if you want to. All
 other parameters must be named, and can be **in any order you like**.
 - By default, `CreateTarget` creates an application target, so we don't need to explicitly tell it
 we want an application (as opposed to a static or dynamic library).

Run `phoenix .` to generate build files for our "hello" application. By default, Phoenix generates
[Ninja](https://github.com/martine/ninja) files. (If you don't have Ninja installed, or prefer to use
some other build system, run `phoenix --generators` to list the available generators.).
