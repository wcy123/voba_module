#pragma once
#include <exec_once.h>
#ifdef __cplusplus
extern "C" {
#endif
static const char VOBA_MODULE_LANG_ID[]="__lang__";
static const char VOBA_MODULE_LANG_MATCH[]="__match__";
extern voba_str_t * voba_c_id_encode(voba_str_t * str);
extern voba_str_t * voba_c_id_decode(voba_str_t * str);
extern voba_value_t voba_modules;
extern voba_value_t voba_import_module(const char * module_name, const char * module_id, const char * symbols[]);
extern voba_value_t voba_load_module(const char * filename,voba_value_t module);
extern voba_value_t voba_module_path();
extern voba_str_t* voba_find_file(voba_value_t search_path, // array of search path
                                   voba_str_t * name, // name
                                   voba_str_t * cwd,// current working directory
                                   voba_str_t * prefix,
                                   voba_str_t * suffix,
                                   int resolv_realpath
    );
extern voba_value_t voba_init_path_from_env(const char*);
#define VOBA_DEFINE_MODULE_SYMBOL(s,v) voba_define_module_symbol(s,v,__FILE__,__LINE__)
extern void voba_define_module_symbol(voba_value_t symbol, voba_value_t value, const char * file , int line);
#ifdef __cplusplus
}
#endif

#define VOBA_DECLARE_SYMBOL_TABLE_SYM_NAME(sym) #sym,
#define VOBA_DECLARE_SYMBOL_TABLE(SYMBOL_TABLE)                         \
    EXEC_ONCE_PROGN {                                                   \
        static const char * symbols[] = {                               \
            SYMBOL_TABLE(VOBA_DECLARE_SYMBOL_TABLE_SYM_NAME)            \
            NULL                                                        \
        };                                                              \
        voba_import_module(VOBA_MODULE_NAME,VOBA_MODULE_ID,symbols);    \
    }                                                                   \
    SYMBOL_TABLE(VOBA__DECLARE_SYMBOL_TABLE_2)                          


#define VOBA__SYM_VAR(sym) VOBA_MODULE_NAME_SPACE(VOBA_MODULE_SYMBOL_PREFIX(sym))

#define VOBA__DECLARE_SYMBOL_TABLE_2(sym)                               \
static voba_value_t VOBA__SYM_VAR(sym) = VOBA_NIL;                      \
EXEC_ONCE_PROGN{                                                        \
    VOBA__SYM_VAR(sym) = voba_module_var(VOBA_MODULE_NAME, VOBA_MODULE_ID, #sym); \
}

static inline voba_value_t voba_module_var(const char * name, const char * module_id, const char * symbol_name)
{
    voba_value_t id = voba_make_string(voba_str_from_cstr(module_id));
    voba_value_t m = voba_hash_find(voba_modules,id);
    if(voba_is_nil(m)){
        fprintf(stderr,__FILE__ ":%d:[%s] module `%s(%s)' should already be there.", __LINE__, __FUNCTION__
                ,name,module_id);
        fprintf(stderr,__FILE__ );
        // abort(0);
    }
    voba_str_t * symbol_name0 = voba_str_from_cstr(symbol_name);
    voba_str_t * symbol_name1 = voba_c_id_decode(symbol_name0);
    voba_value_t symbol_name2 = voba_make_string(symbol_name1);
    voba_value_t s = voba_lookup_symbol(symbol_name2,voba_tail(m));
    if(voba_is_nil(s)){
        fprintf(stderr,__FILE__ ":%d:[%s] module `%s(%s)' module should contain symbol `%s'\n", __LINE__, __FUNCTION__,
                name,module_id,symbol_name);
        //abort(0);
    }
    return s;
}

