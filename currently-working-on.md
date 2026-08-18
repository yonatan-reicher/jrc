Currently working on some ML stuff. I just wrote down the MLModel interface. I
would like to finish refactoring the DenseLayer and write some activation
function, and make a sequential model.

Then, I would like to change MLModel to be an enum instead of a vtable, then we
could test if that's faster than the vtable. You can even have a case for a
vtable model.
