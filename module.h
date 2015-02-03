#pragma once
#include <assert.h>
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
/** the all modules, a symbol table */    
extern voba_value_t voba_modules;
/** import a module 
    
    if the module is already loaded, do nothing but returns the module.
    if the module does not exists, then 
      - create a new symbol table, i.e. a module
      - insert it into ::voba_modules with hash key `module_id`
      - create all symbols based on `symbol_names`

    @param module_name the name
    @param module_id  the id
    @param symbol_names  a tuple of symbols
    
    @return the module object, a hash table.

    @note `symbol_names` could be arbitrary strings, now it is a tuple, could be an array?

 */
extern voba_value_t voba_import_module(const char * module_name, const char * module_id, voba_value_t symbols);
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
        static voba_value_t                                             \
            symbols2[sizeof(symbols)/sizeof(const char*)] = {           \
            sizeof(symbols)/sizeof(const char*) - 1, VOBA_NIL,          \
        };                                                              \
        for(size_t i = 0 ; symbols[i]!=NULL; ++i){                      \
            symbols2[i+1] =                                             \
                voba_make_string(                                       \
                    voba_c_id_decode(                                   \
                        voba_str_from_cstr(symbols[i])));               \
        }                                                               \
        voba_import_module(VOBA_MODULE_NAME,                            \
                           VOBA_MODULE_ID,                              \
                           /* tuple or array?*/                         \
                           voba_make_tuple(symbols2));                  \
    }                                                                   \
    SYMBOL_TABLE(VOBA__DECLARE_SYMBOL_TABLE_2)                          


#define VOBA__SYM_VAR(sym) VOBA_MODULE_NAME_SPACE(VOBA_MODULE_SYMBOL_PREFIX(sym))

#define VOBA__DECLARE_SYMBOL_TABLE_2(sym)                               \
static voba_value_t VOBA__SYM_VAR(sym) = VOBA_NIL;                      \
EXEC_ONCE_PROGN{                                                        \
    VOBA__SYM_VAR(sym) = \
        voba_module_var(VOBA_MODULE_NAME,                               \
                        VOBA_MODULE_ID,                                 \
                        voba_make_string(                               \
                            voba_c_id_decode(                           \
                                voba_str_from_cstr(#sym))));            \
}
/** return a symbol of in a module
    
    @param module_name
    @param module_id
    @param symbol_name an arbitrary string for a symbol name, this
           symbol must be exists in the module.
    @return a symbol
 */
static inline voba_value_t voba_module_var(const char * module_name, const char * module_id, voba_value_t symbol_name)
{
    voba_value_t id = voba_make_string(voba_str_from_cstr(module_id));
    voba_value_t m = voba_hash_find(voba_modules,id);
    assert(voba_is_a(symbol_name,voba_cls_str));
    if(voba_is_nil(m)){
        fprintf(stderr,__FILE__ ":%d:[%s] module `%s(%s)' should already be there.", __LINE__, __FUNCTION__
                ,module_name,module_id);
        fprintf(stderr,__FILE__ );
        abort();
    }
    voba_value_t s = voba_lookup_symbol(symbol_name,voba_tail(m));
    if(voba_is_nil(s)){
        fprintf(stderr,__FILE__ ":%d:[%s] module `%s(%s)' module should contain symbol `%s'\n", __LINE__, __FUNCTION__,
                module_name, module_id, voba_str_to_cstr(voba_value_to_str(symbol_name)));
        abort();
    }
    return s;
}

