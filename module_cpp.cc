#include <voba/include/value.h>
#include <iostream>
using namespace std;
extern "C" voba_value_t load_module_cc(voba_value_t (*init)(voba_value_t), voba_value_t module, voba_value_t module_cwd, voba_value_t dirname)
{
    voba_value_t ret = VOBA_NIL;
    try{
        voba_array_push(module_cwd,dirname);
        ret = init(module);
        voba_array_pop(module_cwd);
    }catch(voba::exception e) {
        voba_array_pop(module_cwd);
        throw(e);
    }
    return ret;
}



