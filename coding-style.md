# Coding Style

Code should supplement documentation. If you write a new function or feature,
you should write it in the following order:
1. Write signatures and prototypes.
2. Write the documentation.
3. Write tests.
4. Write implementation.
5. Refine all of the above, in the same order.

Well documented code should feel like a book. It should have a story with a
narrative, where characters appear and have goals and are introduced as they are
relevant. Each file should aim to be it's own stand-alone story, that you can
read from top-to-bottom.

A difference between reading a narrative book and well-documented code is that
code cannot be read completely linearly. While certain checkpoints can help make
it more linear (Write short-to-medium header files, make each file's
documentation stand-alone, etc), readers are going to jump around in
documentation as needed, and as things are interesting to their eyes. This makes
it very important to make jumping around easy, and the jumps (usually between
functions in the same file or to another file) should file like going from one
short story to another.


## Abbreviations and Shortening in Identifiers and Comments

Generally, words should not be abbreviated (or shortened). That is unless:
1. Explicitly specified in a relevant section. In that case, they should always
   be abbreviated in identifiers.
2. Are part of the abbreviated words table below. They should always be
   abbreviated in identifiers.


| Full | Abbreviation |
|:-----|:------|
| number | n |
| count | n |
| input | inp |
| output | out |
| buffer | buf |
| string | str |
| error | err |
| function     | func         |
| matrix       | mat          |
| context      | ctx          |
| derivative   | deriv        |


## Namespacing

Names should be namespaced. By that I mean, identifiers should be grouped
together by a common name feature that distinguishes them from other
identifiers. This serves both for avoiding name-collisions and for finding
relevant identifiers.

Identifier groups should be based on relevant types (example: `array_sort`,
`array_len`, `array_to_slice`). Identifier groups based on abstract concepts are
allowed (example: `err_correcting_get_n_errors`) but should be avoided.

Common name features can be a common prefix, or a common postfix. In certain
cases, they can be more complicated. For example, a `Stream` type might have a
namespace with a common postfix for related types such as `ArrayStream` and
`EmptyStream` but a common prefix for functions (`stream_clone`), with also
additional related namespace having a common prefix of a related type
`array_stream_clone`.

Namespacing rules for a group of identifiers does not have to be documented.

## Parameter Types

TODO:
Explain that some parameters are read only, some are write only, some are
read-write, and some are *move*.
Explain how they should be typed.
Explain borrowing and that it should be specified in documentation.

## Naming Idioms

- `of` - non-move conversions.
- `from` - moving conversions.
- `free` - free-like functions.
- `new` - most important constructor.
- `copy` - cheap copy.
- `clone` - may-not-be-cheap copy.
- `is` - returns `bool` and is pure.
