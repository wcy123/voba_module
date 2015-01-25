set -e

gcc -I ../../../build/include -I. -std=c99 -fPIC -o m1/libmain.so -shared m1/main.c
gcc -I ../../../build/include -I. -std=c99 -fPIC -o m1/_private/libfoo.so -shared m1/_private/foo.c
gcc -I ../../../build/include -I. -std=c99 -fPIC -o m2/libmain.so -shared m2/main.c
gcc -I ../../../build/include -I. -std=c99 -fPIC -o m2/_private/libfoo.so -shared m2/_private/foo.c
gcc -I ../../../build/include -std=c99 -o ex2 ex2.c \
    -L ../../../build/lib -lexec_once -lgc -lvoba_value -lvoba_module -ldl
#export EXEC_ONCE_DEBUG=1
#export VOBA_PATH=.
#export CONFIG=release
#export YY_FLEX_DEBUG=0
#export VOBA_MODULE_DEBUG=1
LD_LIBRARY_PATH=../../../build/lib ./ex2
