# JRC

A set of useful C utilities, for personal use, packaged as a library.

For building and installing, you may checkout `make help`. You may just clone
this repo to your project, and compile the C files on your own (`gcc main.c
./jrc/src/*.c`)

## Provides

### General Purpose
- **array.h** - Growing arrays, like in a modern language.
- **basic.h** - Some basic utilities to make you want to always include this file.
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
- **interpreter.h** - A simple interpreter.

## Currently Working On

Arrays, Slices and Paths. Paths are the biggest priority
Starting work on lexer.

## TODO

Integrate with "c-stream", maybe eat it.
Make an allocator interface, with a malloc allocator, a dynamic arena allocator,
and a stack arena allocator.
Make a Str type which holds it's length.
