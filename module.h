#pragma once
#include <exec_once.h>
#ifdef __cplusplus
extern "C" {
#endif
/** should these 3 macros be moved to another file? */
static const char VOBA_MODULE_LANG_ID[]="__lang__";
static const char VOBA_MODULE_LANG_MATCH[]="__match__";
static const char VOBA_MODULE_LANG_ITER[]="__iter__";
/** a pair of functions for simple decode/encode of c identifier */    
extern voba_str_t * voba_c_id_encode(voba_str_t * str);
extern voba_str_t * voba_c_id_decode(voba_str_t * str);
/** the all modules, a hash table */    
extern voba_value_t voba_modules;
/** import a module 
    
    if the module is already loaded, do nothing but returns the module

    @module_name the name
    @module_id  the id
    @symbols  an array of symbols
    
    @return the module object, a hash table.
 */
extern voba_value_t voba_import_module(const char * module_name, const char * module_id, const char * symbols[]);
/** find the module and load it.
    this function should not be used by others, why it is here?
 */    
extern voba_value_t voba_load_module(const char * filename,voba_value_t module);
/** the search path for modules.
 * an array of string
 */    
extern voba_value_t voba_module_path();
/** util function to find a file in a search path of */    
extern voba_str_t* voba_find_file(voba_value_t search_path, // array of search path
                                  voba_str_t * module_name, // name
                                  voba_str_t * cwd,// current working directory
                                  voba_str_t * prefix,
                                  voba_str_t * suffix,
                                  int resolv_realpath,
                                  voba_value_t attempts
    );

/** simple wrapper macros to attach __FILE__ and __LINE__ to a symbol
    definition.
    
    `VOBA_DEFINE_MODULE_SYMBOL(s,v)` is almost identical to 
<code>
    voba_set_symbol_value(s,v)
</code>
    where `s` is a symbol by looking up a hash table, i.e. a module object.
    
 */    
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

