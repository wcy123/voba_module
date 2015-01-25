#define EXEC_ONCE_TU_NAME "main"
#include <voba/value.h>
#include "m1/main.h" // import m1/main
#include "m2/main.h" // import m2/main
int main(int argc, char *argv[])
{
    exec_once_init();
    voba_value_t args[] = {0};
    voba_apply(voba_symbol_value(s_hello_m1),voba_make_tuple(args)); // invoke s_hello_m1
    voba_apply(voba_symbol_value(s_hello_m2),voba_make_tuple(args)); // invoke s_hello_m2
    return 0;
}
