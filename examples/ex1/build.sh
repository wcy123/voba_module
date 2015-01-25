gcc -I ../../../build/include -std=c99 -fPIC -o libhello.so -shared hello.c
gcc -I ../../../build/include -std=c99 -o ex1 ex1.c \
    -L ../../../build/lib -lexec_once -lgc -lvoba_value -lvoba_module -ldl
export EXEC_ONCE_DEBUG=1
#export VOBA_PATH=.
export CONFIG=release
export YY_FLEX_DEBUG=0
export VOBA_MODULE_DEBUG=1
LD_LIBRARY_PATH=../../../build/lib ./ex1
