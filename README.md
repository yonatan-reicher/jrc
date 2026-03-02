# JRC

A set of useful C utilities, for personal use, packaged as a library.

For building and installing, you may checkout `make help`. You may just clone
this repo to your project, and compile the C files on your own (`gcc main.c
./jrc/src/*.c`)

## Provides

### `basic.h`
- `UNUSED(X)`
- `STR(X)`
- `ARRAY_LEN(A)`
- `PANIC(PRINTF_FORMAT, ...ARGS)`
- `WARNING(PRINTF_FORMAT, ...ARGS)`

### `string.h`
- `str_starts_with(str, prefix)`
- `str_remove_newline(str)`
- `str_eq(a, b)`
