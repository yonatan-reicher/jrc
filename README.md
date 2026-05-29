# JRC

A set of useful C utilities, for personal use, packaged aus a library.

For building and installing, you may checkout `make help`. You may just clone
this repo to your project, and compile the C files on your own (`gcc main.c
./jrc/src/*.c`)

## Code Style

When constructing structures, use an initializer list, but not designated
initializers, when intending to initialize all fields.
This is because compilers do not warn when fields are missing when using
designated field initializers.

## Provides

### General Purpose
- **array.h** - Growing arrays, like in a modern language.
- **basic.h** - Some basic utilities to make you want to always include this
  file.
- **macro.h** - Advanced macro utilities, not yet ready for use.
- **path.h** - A path type for working with files and file systems.
- **slice.h** - A slice type, that is, a pointer and a length.
- **str.h** - Working with strings.

### Programming Language Tools
- **text_pos.h** - Definitions for tracking source code positions.
- **token.h** - A token fit for a programming language.
- **lexer.h** - Tokenizes a string.
- **ast.h** - An Abstract Syntax Tree with some source info.
- **parser.h** - Parses tokens into an AST.
- **value.h** - A thing able to hold many kinds of general values.
- **type.h** - Types for value objects.
- **interpreter.h** - A simple interpreter.
- **type_check.h** - A type checker.

## More Documentation

Checkout [currently working on](currently-working-on.md) to see what I've
started and haven't finished. [Todo](todo.md) has a list of things I want to do
but haven't gotten to yet. The file [document style](documentation-style.md) has
notes on how to read and write documentation for this project.
