#define EXEC_ONCE_TU_NAME "hello"
#include <voba/value.h>
#include "m1/main.h"
#include "_private/foo.h"
VOBA_FUNC static voba_value_t hello_m1(voba_value_t self, voba_value_t args)
{
    printf("Hello from module 1\n");
    voba_value_t xargs[] = {0};
    voba_apply(voba_symbol_value(s_foo),voba_make_tuple(xargs));
    return VOBA_NIL;
}
EXEC_ONCE_PROGN{
    VOBA_DEFINE_MODULE_SYMBOL(s_hello_m1, voba_make_func(hello_m1));
}
// the main entry
voba_value_t voba_init(voba_value_t this_module)
{
    return VOBA_NIL;
}
