# voba_module

The module system for voba language.

see the doxygen version at <https://wcy123.github.io/voba_module>

## The first module 

### create a module header file

see `examples/ex1/hello.h`

~~~{.c}
#pragma once
#define VOBA_MODULE_ID "eaebdb3c-a436-11e4-bf65-0800272ae0cc"
#define VOBA_SYMBOL_TABLE(XX)                           \
    XX(hello_world)                                     \

#define VOBA_MODULE_NAME "hello"
#include <voba/module_end.h>
~~~

It is a simple C header file and it defines 3 macros.
1. `VOBA_MODULE_ID`: it is a unique id. It is generated, e.g. by
   ```uuidgen -t```
2. `VOBA_SYMBOL_TABLE(XX)`: A macro to define a set of symbols.
3. `VOBA_MODULE_NAME`: name of a module.
4. we must have `voba/module_end.h` in the end.

### define the module

see `examples\ex1\hello.c`
~~~{.c}
#define EXEC_ONCE_TU_NAME "hello"
#include <voba/value.h>
#include "hello.h"

VOBA_FUNC static voba_value_t hello_world(voba_value_t self, voba_value_t args)
{
    printf("Hello Module\n");
    return VOBA_NIL;
}

EXEC_ONCE_PROGN{
    VOBA_DEFINE_MODULE_SYMBOL(s_hello_world, voba_make_func(hello_world));
}
// the main entry
voba_value_t voba_init(voba_value_t this_module)
{
    return VOBA_NIL;
}
~~~

For `EXEC_ONCE_PROGN` and `EXEC_ONCE_PROGN`, refer to <https://github.com/wcy123/exec_once.git>

The module definition must import/include the module head file by
itself.

The symbol `hello_world` is prefixed with `s_` for C identifier.

`VOBA_DEFINE_MODULE_SYMBOL` is used to define the value of a symbol.

`voba_init` is the entry function of the module when module is loaded,
in this example, it does nothing.

### build the module

~~~
gcc -I ../../../build/include -std=c99 -fPIC -o libhello.so -shared hello.c
~~~

now, we have `libhello.so`, the module.

### how to use the module

see `examples\ex1\ex1.c`

~~~{.c}
#define EXEC_ONCE_TU_NAME "main"
#include <voba/value.h>
#include "hello.h"
int main(int argc, char *argv[])
{
    exec_once_init();
    voba_value_t args[] = {0};
    voba_apply(voba_symbol_value(s_hello_world),voba_make_tuple(args));
    return 0;
}
~~~

For `EXEC_ONCE_TU_NAME`, refer to  <https://github.com/wcy123/exec_once.git>.
We must define it before including `voba/value.h`

To import the module, just `include "hello.h"`

`exec_once_init()` must be invoked in the main function in order to
load the module.

Because `"hello.h"` is included, so that the C variable,
`s_hello_world` is declared, and it will be defined after loading the
module, `libhello.so`.

If there is something wrong, set environment variable
`VOBA_MODULE_DEBUG` to `1` for troubleshooting.

### how to find the module's shared library

There two types of module, public and private. If a module name starts
with `"."`, it is a private module, otherwise it is a public module.

There is an array of search path, called, `voba_path`, which is
initialized by the environment variable, `VOBA_PATH`.  For public
module, every element is tried for searching the shared library. For
example,

if `voba_path` is

~~~
path1
path2
~~~

And if the module name is `a/b/c/your_module_name`, then the following
files are searched in order.

~~~
path1/a/b/c/libyour_module_name.so
path2/a/b/c/libyour_module_name.so
~~~

A private module name starts with `"."`, for example,
`"./my/private_module"`.  `voba_path` is not used for private module,
the path is relative to the importing module. Here is an example in `examples/ex2`

The directory structure of `examples/ex2`
<pre>

├── build.sh         (how to build everything)
├── ex2              (the executable file build from ex2.c)
├── ex2.c            (main entry point)
├── m1               (m1 module)
│   ├── libmain.so   (shared library for m1/main)
│   ├── main.c
│   ├── main.h
│   └── _private     (directory for private modules)
│       ├── foo.c
│       ├── foo.h
│       └── libfoo.so (shared library for ./_private/foo)
└── m2
    ├── libmain.so
    ├── main.c
    ├── main.h
    └── _private
        ├── foo.c
        ├── foo.h
        └── libfoo.so (shared library for ./_private/foo)
</pre>

#### ex2.c
<code>
@include ex2/ex2.c
</code>

it imports `m1/main` and `m2/main` respectively.

#### m1/main.h,m1/main.c, m2/main.h and m2/main.c

1. m1/main.h
<code>
@include ex2/m1/main.h
</code>
1. m2/main.h
<code>
@include ex2/m2/main.h
</code>

Note that the uuid of `m1/main` and `m2/main` must be different.

2. m1/main.c
<code>
@include ex2/m1/main.c
</code>
2. m2/main.c
<code>
@include ex2/m2/main.c
</code>

Note both `m1/main.c` and `m2/main.c` import the module
`./private/foo.h`, but they import different `./private/foo`.


### foo.h and foo.c

1. `m1/_private/foo.h`
<code>
@include ex2/m1/_private/foo.h
</code>
1. `m2/_private/foo.h`
<code>
@include ex2/m2/_private/foo.h
</code>

Note that the uuid is not same, but the module name is same,
`./_private/foo`. It is a private module so the names are not
conflicts, one `./_private/foo` is relative to `m1/main`, the other is
relative to `m2/main`

2. `m1/_private/foo.c`
<code>
@include ex2/m1/_private/foo.c
</code>
2. `m2/_private/foo.c`
<code>
@include ex2/m2/_private/foo.c
</code>

### build everything

<code>
@include ex2/build.sh
</code>

### output

~~~
+ set -e
+ gcc -I ../../../build/include -I. -std=c99 -fPIC -o m1/libmain.so -shared m1/main.c
+ gcc -I ../../../build/include -I. -std=c99 -fPIC -o m1/_private/libfoo.so -shared m1/_private/foo.c
+ gcc -I ../../../build/include -I. -std=c99 -fPIC -o m2/libmain.so -shared m2/main.c
+ gcc -I ../../../build/include -I. -std=c99 -fPIC -o m2/_private/libfoo.so -shared m2/_private/foo.c
+ gcc -I ../../../build/include -std=c99 -o ex2 ex2.c -L ../../../build/lib -lexec_once -lgc -lvoba_value -lvoba_module
+ LD_LIBRARY_PATH=../../../build/lib ./ex2
Hello from module 1
hello from m1/foo.
hello from module 2
hello from m2/foo.
~~~

## dependency

<pre>
@dot
digraph example {
node [shape=record, fontname=Helvetica, fontsize=10];
 voba_module [label="voba_module" URL="https://wcy123.github.io/voba_module"];
 voba_value [ label="voba_value" URL="https://wcy123.github.io/voba_value"];
 exec_once [ label="exec_once" URL="https://wcy123.github.io/exec_once"];
 voba_str [ label="voba_str" URL="https://wcy123.github.io/voba_str"];
 vhash [ label="vhash" URL="https://wcy123.github.io/vhash"];
 gc [ label="gc" URL="http://www.hboehm.info/gc"];
 voba_value -> exec_once [ arrowhead="open", style="dashed" ];
 voba_value -> voba_str [ arrowhead="open", style="dashed" ];
 voba_value -> vhash [ arrowhead="open", style="dashed" ];
 voba_value -> gc [ arrowhead="open", style="dashed" ];
 voba_module -> voba_value [ arrowhead="open", style="dashed" ];
 voba_module -> voba_str [ arrowhead="open", style="dashed" ];
 voba_module -> exec_once [ arrowhead="open", style="dashed" ];
 voba_str -> gc [ arrowhead="open", style="dashed" ];
}
@enddot
</pre>

In order to compatible with C, only C identifier is allowed. To
represent any other characters, we can use a underline followed by two
hexagons, for example,
   
~~~{.c}
   XX(_2B)/*+*/                                        \
   XX(_3E)/*>*/                                        \
   XX(_3C)/*<*/                                        \
   XX(_3E_3D)/*>=*/                                    \
   XX(_3C_3D)/*<=*/                                    \
   XX(_3D_3D)/*==*/                                    \
   XX(_25)/* % */                                      \
   XX(_3C_3C)/*<<*/                                    \
~~~


