#define EXEC_ONCE_TU_NAME "m1/foo"
#include <voba/value.h>
#include "foo.h"
VOBA_FUNC static voba_value_t foo(voba_value_t fun, voba_value_t args, voba_value_t* next_fun, voba_value_t next_args[])
{
    printf("hello from m1/foo.\n");
    return VOBA_NIL;
}
EXEC_ONCE_PROGN{
    VOBA_DEFINE_MODULE_SYMBOL(s_foo, voba_make_func(foo));
}
// the main entry
voba_value_t voba_init(voba_value_t this_module)
{
    return VOBA_NIL;
}
