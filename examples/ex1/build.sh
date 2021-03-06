set -e
CC=${CC:-gcc}
$CC -I ../../../build/include $CFLAGS -std=c99 -fPIC -ggdb -O0 -o libhello.so -shared hello.c
$CC -I ../../../build/include $CFLAGS -std=c99 -ggdb -O0 -o ex1 ex1.c \
    -L ../../../build/lib $LDFLAGS -lexec_once -lgc -lvoba_value -ldl -lvoba_module
export EXEC_ONCE_DEBUG=1
export VOBA_PATH=.
export CONFIG=release
export YY_FLEX_DEBUG=0
export VOBA_MODULE_DEBUG=1
LD_LIBRARY_PATH=../../../build/lib ./ex1
