## 2015/03/09 12:03:05 whether a module implemention know its module id?

- For C module, currently, it does not know the module id of itself.
- The main module could have no header file, so that there is no module id.
- `__main__` could be the id of the main module
- Then the main module could potentially be loaded twice.
- One module could contains many tranlate units, but only one of them
  is able to define its symbols, because `voba_import_module` requires
  that all symbols are defined in one go.
  - the module definition (the translation unit) itself needs to
    import the module firstly (`voba_import_module`), then set each
    symbol value.

### a macro could simply avoid a module be loaded twice?

No, because symbols must be defined. When a module is loaded twice
with different module id, the symbol table must be same by looking up
the different module id.

### treat the `__main__` module differently.

Whenever a new module is loaded, checking the file name is as same as
the file name of `__main__` module, if it is, replace the symbol table
(module).

No, it doesn't solve the problem, see below.

### use the sha1 of the shared library as a module id?

No, we cannot know the id at compilation time.

### use the file name of the shared library as a module id?

No, it could be loaded twice because of hard or soft link.  It takes
time to resolve real path.


### add a new macro to specify the own module id?

extra work is not preferred.

No good solution yet.
