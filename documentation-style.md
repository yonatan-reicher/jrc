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
