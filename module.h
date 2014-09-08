#pragma once
#include <exec_once.h>
#ifdef __cplusplus
extern "C" {
#endif

extern voba_value_t voba_modules;
extern voba_value_t voba_load_module(const char * filename,voba_value_t module);
extern voba_value_t voba_module_path();
extern voba_str_t* voba_find_file(voba_value_t search_path, // array of search path
                                  voba_str_t * name, // name
                                  voba_str_t * cwd,// current working directory
                                  voba_str_t * prefix,
                                  voba_str_t * suffix,
                                  int resolv_realpath
    );
#define VOBA_DEFINE_MODULE_SYMBOL(s,v) voba_define_module_symbol(s,v,__FILE__,__LINE__)
extern void voba_define_module_symbol(voba_value_t symbol, voba_value_t value, const char * file , int line);
#ifdef __cplusplus
}
#endif

#define VOBA_DECLARE_SYMBOL_TABLE(SYMBOL_TABLE)                         \
EXEC_ONCE_DO(                                                           \
    voba_value_t id = voba_make_string(voba_str_from_cstr(VOBA_MODULE_ID));  \
    voba_value_t m = voba_hash_find(voba_modules,id);                   \
    if(voba_is_nil(m)){                                                 \
        m = voba_make_symbol_table();                                   \
        voba_hash_insert(voba_modules,id,m);                            \
        SYMBOL_TABLE(VOBA__DECLARE_SYMBOL_TABLE_1);                     \
        voba_load_module(VOBA_MODULE_NAME,m);                                \
    }                                                                   \
    );                                                                  \
SYMBOL_TABLE(VOBA__DECLARE_SYMBOL_TABLE_2)

#define VOBA__DECLARE_SYMBOL_TABLE_1(sym)                               \
    do{                                                                 \
        voba_intern_symbol(voba_make_symbol_cstr(#sym,VOBA_NIL),m);     \
    }while(0);

#define VOBA__SYM_VAR(sym) VOBA_MODULE_NAME_SPACE(VOBA_MODULE_SYMBOL_PREFIX(sym))

#define VOBA__DECLARE_SYMBOL_TABLE_2(sym)                               \
static voba_value_t VOBA__SYM_VAR(sym) = VOBA_NIL;                      \
EXEC_ONCE_DO(                                                           \
    voba_value_t id = voba_make_string(voba_str_from_cstr(VOBA_MODULE_ID)); \
    voba_value_t m = voba_hash_find(voba_modules,id);                   \
    assert(!voba_is_nil(m) && "module " VOBA_MODULE_ID " should already be there."); \
    voba_value_t s = voba_lookup_symbol(voba_make_symbol_cstr(#sym,VOBA_NIL),voba_tail(m)); \
    assert(!voba_is_nil(s) && "module " VOBA_MODULE_ID " should contain symbol " #sym); \
    VOBA__SYM_VAR(sym) = s;                                             \
)




#define VOBA_DEF(v,expr)                                          \
    static voba_value_t v = VOBA_NIL;                             \
    static void init__begin__ ##v## __init_end()                  \
    {                                                             \
        v = expr;                                                 \
    }                                                             \
    EXEC_ONCE(init__begin__ ##v## __init_end)


