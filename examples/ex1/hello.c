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
