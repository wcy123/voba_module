#define EXEC_ONCE_TU_NAME "main"
#define EXEC_ONCE_DEPENDS {"voba.module",NULL}
#include <voba/value.h>
#include "hello.h"
int main(int argc, char *argv[])
{
    exec_once_init();
    voba_value_t args[] = {0};
    voba_apply(voba_symbol_value(s_hello_world),voba_make_tuple(args));
    return 0;
}
