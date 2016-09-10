This document contains a complete syntax and API reference to Phoenix's built-in
scripting language, as well as all build system functions and their usage.

Syntax
---------------------------------------
The grammar is primarily based on C and C++, with a type system that mimics that
of other interpreted languages (mainly Python and JavaScript).

The basics:
 - Lines **must** end with semicolons.
 - A hash `#` begins a comment that lasts until the end of the current line.
 - Every variable must be one of `undefined`, `boolean`, `integer`, `string`,
   `function`, `list`, or `map` in type.
 - The control-flow keywords `if`, `while`, and `return` behave identically to
   their C/C++/JS counterparts.
 - **Setting a first variable to a second variable *copies* the contents of the**
   **second variable overtop of the first,** unlike JavaScript, where only primitive
   types (strings, integers, etc.) behave this way.
 - All references to variables must begin with `$`, e.g. `$a = 5; $b = $a;`.
 - Variables may be referenced by name by placing `[]`s immediately following
   the `$`; e.g. `$['a'] = 5;` and `$a = 5;` are functionally identical code,
   as is `$var = "a"; $[$var] = 5;`.
 - Referencing child objects can be done using the `.` or `[]` tokens.
 - Functions are defined using `function () { ... }`.
  - Nothing ever goes between the `function` and the `()` in the declaration. Functions may be
   assigned to variables the same way as any other type.
  - **All parameters to functions must be named** except for the first, which
   may be left unnamed (e.g. `func("this is the first argument", secondarg: 5, whatever: $a)`).
  - Since parameters are named, order is irrelevant, and so they may be specified in any order.
  - Any parameter that does not have a specified value (meaning, the `:` and statement
    following it are left out) will be assumed to be a true boolean
    (e.g. `func("thing", recursive)` is equivalent to `func("thing", recursive: true)`).
  - All function parameters are passed in to the function as locals, e.g. for the above example,
   `$recursive` will be `true` inside the function body. Additionally, all the arguments to the
   function are also present in map form in a local that can be accessed using `$__arguments`.
 - Strings enclosed by double-quotes (`"`) are normal strings, and strings enclosed by
   single-quotes (`'`) are literal strings.
   - (e.g. if the two characters `\` and `n` occur one after another, they will be treated
     as a UNIX newline character in normal strings, and a `\` and an `n` in literal strings.)
   - Variables may be stringified and inserted into normal strings at any point by using `${}`,
     e.g. `$a = 5; $b = "I have ${a} things.";`
 - Lists can be created using the typical syntax, e.g. `$a = [1, 2, 3, "blah"];`, and
   also accessed with the typical syntax, e.g. `$a[0];`.
 - Maps can be created using the special `Map()` function, which has the same syntax as any
   other function but creates a map, e.g. `$a = Map(one: 1, two: 2);`.
 - Superglobals are denoted by an extra `$` (e.g. `$$Phoenix`). They are created by Phoenix
   itself, and can *only* be accessed, not created, destroyed, modified, or copied.

Superglobals
---------------------------------------
### `$$Phoenix: map`
The "Phoenix" superglobal. Contains various information about this Phoenix instance.
Members:
 - `checkVersion: function(minimum: string)`
   This should be the first call at the top of any root Phoenixfile. It ensures that
   the running version of Phoenix is at least `minimum`, and exits immediately if it
   is not. `minimum` must be a string in the format `0`, `0.0`, or `0.0.0`.

### `$$Compilers: map`
Contains a map of programming languages to compilers (e.g. `$$Compilers['C']` might
equal `"GNU"`). **NOTE** that if a specific programming language has not yet been used
by any target nor specified in a call to `Project()`, its `$$Compilers` entry will be
undefined, as Phoenix only detects compilers when it needs to.

Additionally, for all the values specified inside `$$Compilers`, they are also added to superglobal
namespace as a true boolean (e.g., so if `$$Compilers['C']` is `"Clang"`, then `$$Clang` will be `true`).

### `$$OS: string`
Contains the user-friendly name of the operating system that the software is
being built for. Possible values are: `"Windows"`, `"Linux"`, `"FreeBSD"`,
`"Haiku"`, or `"Apple"`.

Additionally, the OS' "raw" name will be added to the superglobals as a true
boolean (one of `$$WIN32`, `$$LINUX`, `$$FREEBSD`, `$$HAIKU`, or `$$APPLE`).
And on UNIX (POSIX) systems, `$$UNIX` will also be set to `true`.

Functions
---------------------------------------

### `print(0: any)`
Prints the first (unnamed) argument to standard-output, in "raw" form.

### `dump(0: any)`
Pretty-prints the first (unnamed) argument to standard-output.

### `fatal(0: any)`
Immediately aborts, printing the message from the first (unnamed) argument as the
exit reason.

### `parseInt(0: string)`
Parses the first argument as an integer and returns the result. If it cannot
be parsed as such, it returns `undefined`.

### `File(0: string)`
Creates a new `File` object. Members:
 - `exists: function()`
   - Returns whether or not the file presently exists and is actually a file.
     (If it is a directory, this function will return false).
 - `getContents: function()`
   - Returns the file's contents as a string.
 - `setContents: function(0: string)`
   - Sets the file's contents to `0`.
 - `remove: function(0: string)`
   - Permanently deletes the file.

------

### `Project(0: string, [languages: list | language: string])`
Sets the name of the project to `0`, loads the language definition files for the
languages specified in `languages` or `language`, and locates working compilers
for all of them. Note that if no languages are specified here, then the
language definition files and compilers will be located when `CreateTarget`
is called.

### `CreateTarget(0: string, language: string)`
Creates a new build target with the name `0`, written in the programming language
`language`. The returned Target object will have the following members:
 - `setStandardsMode: function(0: string, strict: boolean)`
   - Sets the standards mode for this target to that specified by `0`. If the mode
     has a "strict" flavor, it will be enabled if and only if `strict` is set to true.
 - `addDefinitions: function(...)`
   - If one or more of the current targets' languages has a preprocessor that accepts
     arbitrary definitions, the keys & values of the arguments to this function will be
     appended to them, if they are strings. (If a key's value is a true boolean [meaning it
     was passed without a value], the value will not be passed, only the key.)
 - `addSources: function(0: array)`
   - Adds the source files specified by `0` to the target.
 - `addSourceDirectory: function(0: string, recursive: boolean)`
   - Searches the directory specified by `0` for source files in any of the languages
     this target was initialized with, optionally recursively.
 - `addIncludeDirectories: function(0: array)`
   - If one or more of the current targets' languages has a preprocessor that accepts
     arbitrary include directories, then the ones specified by `0` will be appended
     to them.
