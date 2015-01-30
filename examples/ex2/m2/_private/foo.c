#define EXEC_ONCE_TU_NAME "m2/foo"
#include <voba/value.h>
#include "foo.h"
VOBA_FUNC static voba_value_t foo(voba_value_t self, voba_value_t args)
{
    printf("hello from m2/foo.\n");
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
