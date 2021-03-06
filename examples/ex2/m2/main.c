#define EXEC_ONCE_TU_NAME "m2/main"
#include <voba/value.h>
#include "m2/main.h"
#include "_private/foo.h"
VOBA_FUNC static voba_value_t hello_m2(voba_value_t fun, voba_value_t args, voba_value_t* next_fun, voba_value_t next_args[])
{
    printf("hello from module 2\n");
    voba_value_t xargs[] = {0, VOBA_BOX_END};
    voba_apply(voba_symbol_value(s_foo),voba_make_tuple(xargs));
    return VOBA_NIL;
}
EXEC_ONCE_PROGN{
    VOBA_DEFINE_MODULE_SYMBOL(s_hello_m2, voba_make_func(hello_m2));
}
// the main entry
voba_value_t voba_init(voba_value_t this_module)
{
    return VOBA_NIL;
}
