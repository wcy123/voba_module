/** @file
VOBA MODULE C APIS
==================
 */    
#pragma once
#include <assert.h>
#include <exec_once.h>
#ifdef __cplusplus
extern "C" {
#endif
/** @brief encode to a C identifier 

    a valid C identifier character is not changed, e.g. `[0-9a-zA-Z_]`
    For other characters, it is translated into `_XX`, where `XX` is
    hex value of the character. Only ASCII code is supported now.

    for example:

    | c identifier | character |
    ---------------|-----------| 
    | _2B          |  +        |
    | _3E          |  >        |
    | _3C          |  <        |


 */    
extern voba_str_t * voba_c_id_encode(voba_str_t * str);
/** @brief decode from a C identifier 
     
 reverse the operation of ::voba_c_id_encode

 */
extern voba_str_t * voba_c_id_decode(voba_str_t * str);
/** the all modules, a symbol table */    
extern voba_value_t voba_modules;
/** @brief import a module 
    
    if the module is already loaded, do nothing but returns the module.
    if the module does not exists, then 
      - create a new symbol table, i.e. the module
      - insert it into ::voba_modules with hash key `module_id`
      - create all symbols based on `symbol_names`, symbol values are initially `undef`.
      - search module's shared library according to the name of the module.
         - private module
              - module name begins with "./"
         - public module
              - module name begins with something other than "./"
         - @sa ::voba_find_file 
      - load the module
         - load the shared library with `dlopen`
         - execute EXEC_ONCE_PROGN blocks in the shared library.
         - invoke `voba_init`
      - check if all symbols are defined, abort otherwise.
      - return the module

    @param module_name the module name
    @param module_id  the module id
    @param symbol_names  a tuple of symbols
    
    @return the module object, a hash table.

    @note 

      - symbols in \a symbol_names must be defined, otherwise it is an error.
      - \a `module_id` is the key
      - `symbol_names` could be arbitrary strings, now it is a tuple,
        could be an array?
      - all symbols must defined after loading the shared library.
      - symbol values are defined with
        ::VOBA_DEFINE_MODULE_SYMBOL,::voba_define_module_symbol, or
        ::voba_symbol_set_value

 */
extern voba_value_t voba_import_module(const char * module_name, const char * module_id, voba_value_t symbols);
/** @brief return the search path for modules.
 * an array of string, initialized with environment variable `VOBA_PATH`
 */    
extern voba_value_t voba_module_path();
/** @brief find a file in a search path in a array of search path 
 * 
 * if `module_name` begin with a dot, e.g. `./a_relative_module`, only
 * the current directory is searched, otherwise, `search_path` is used
 * for searching.
 *
 * The current directory is the directory of the loading module.
 *
 * The actually file name is as `<path>/<prefix><module_name><suffix>`
 * 
 * realpath is used if `resolve_realpath` is not zero.
 * 
 * @param search_path The array of a search path
 * @param module_name The basename of a file
 * @param current_module_directory the directory name of the loading module
 * @param prefix The prefix
 * @param suffix The suffix
 * @param resolve_realpath see below
 * @param attempts An empty array for error reporting.
 *
 * @return The file name if it is found, otherwise nil.  `attempts` is
 * filled with file names which are tried, it is for error reporting.
 */
extern voba_str_t* voba_find_file(voba_value_t search_path, // array of search path
                                  voba_str_t * module_name, // name
                                  voba_str_t * current_module_directory,
                                  voba_str_t * prefix,
                                  voba_str_t * suffix,
                                  int resolve_realpath,
                                  voba_value_t attempts
    );

/** @brief simple wrapper macro to attach `__FILE__` and `__LINE__` to a
    symbol definition.
    
    `VOBA_DEFINE_MODULE_SYMBOL(s,v)` is almost identical to 
<code>
    voba_symbol_set_value(s,v)
</code>
    where `s` is a symbol in a module.
 */    
#define VOBA_DEFINE_MODULE_SYMBOL(s,v) voba_define_module_symbol(s,v,__FILE__,__LINE__)
/** define module symbol

  @param symbol The symbol in a module
  @param value The symbol value
  @param file The file name where the symbol is defined.
  @param line The line number where the symbol is defined.

  This function checks if the symbol is already initialized, i.e. its
  value is something other than ::VOBA_UNDEF. This function uses
  ::voba_symbol_set_value to set the symbol value.

 */
extern void voba_define_module_symbol(voba_value_t symbol, voba_value_t value, const char * file , int line);
#ifdef __cplusplus
}
#endif

#define VOBA_DECLARE_SYMBOL_TABLE_SYM_NAME(sym) #sym,
#define VOBA_DECLARE_SYMBOL_TABLE(SYMBOL_TABLE)                         \
    EXEC_ONCE_PROGN {                                                   \
        static const char * symbols[] = {                               \
            SYMBOL_TABLE(VOBA_DECLARE_SYMBOL_TABLE_SYM_NAME)            \
            NULL							\
        };                                                              \
        static voba_value_t                                             \
            symbols2[sizeof(symbols)/sizeof(const char*) + 1] = {	\
            sizeof(symbols)/sizeof(const char*) - 1, VOBA_NIL,          \
        };                                                              \
        size_t i;							\
	for( i = 0 ; symbols[i]!=NULL; ++i){				\
            symbols2[i+1] =                                             \
                voba_make_string(                                       \
                    voba_c_id_decode(                                   \
                        voba_str_from_cstr(symbols[i])));               \
        }								\
	symbols2[i+1] = VOBA_BOX_END;					\
        voba_import_module(VOBA_MODULE_NAME,                            \
                           VOBA_MODULE_ID,                              \
                           /* tuple or array?*/                         \
                           voba_make_tuple(symbols2));                  \
    }                                                                   \
    SYMBOL_TABLE(VOBA__DECLARE_SYMBOL_TABLE_2)                          


#define VOBA__SYM_VAR(sym) VOBA_MODULE_NAME_SPACE(VOBA_MODULE_SYMBOL_PREFIX(sym))

#define VOBA__DECLARE_SYMBOL_TABLE_2(sym)                               \
static voba_value_t VOBA__SYM_VAR(sym) = VOBA_UNDEF;                    \
EXEC_ONCE_PROGN{                                                        \
    VOBA__SYM_VAR(sym) =                                                \
        voba_module_var(VOBA_MODULE_NAME,                               \
                        VOBA_MODULE_ID,                                 \
                        voba_make_string(                               \
                            voba_c_id_decode(                           \
                                voba_str_from_cstr(#sym))));            \
}
static inline
/** @brief return a symbol of in a module
    
    @param module_name The module name, for error reporting
    @param module_id  The module id
    @param symbol_name an arbitrary string for a symbol name, this
           symbol must be exists in the module.
    @return The symbol

    @note it is a fatal error if the symbol does not exists.

 */
voba_value_t voba_module_var(const char * module_name, const char * module_id, voba_value_t symbol_name)
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

