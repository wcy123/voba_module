#pragma once
// return an array of search directories.
voba_value_t voba_module_path();
// return a module, a module is a symbol table.
voba_value_t voba_import(voba_value_t module_name,voba_value_t pwd);

// the following macros are used by all other modules, but not used by
// voba_module itself
#include "exec_once.h"
static voba_value_t voba___current_module __attribute__((used)) = VOBA_NIL;
static voba_value_t voba___current_working_directory __attribute__((used))  = VOBA_NIL;

#define DECLARE_SYMBOL_TABLE(SYMBOL_TABLE)                              \
    EXEC_ONCE_DO(voba___current_module =                                \
                 voba_import(voba_make_string(voba_str_from_cstr(       \
                                                  MODULE_NAME           \
                                                  )),                   \
                             voba___current_working_directory);         \
                 if(voba_is_nil(voba___current_module)){                \
                     voba_throw_exception(voba_make_string(             \
                                              voba_str_from_cstr("cannot load library " MODULE_NAME))); \
                 }                                                      \
        )                                                               \
    SYMBOL_TABLE(DECLARE_SYMBOL_TABLE_1)

    
#define DECLARE_SYMBOL_TABLE_1(sym)                                     \
    static voba_value_t MODULE_NAME_SPACE(MODULE_SYMBOL_PREFIX(sym)) = VOBA_NIL; \
    static void                                                         \
    MODULE_NAME_SPACE(MODULE_SYMBOL_INIT_PREFIX(sym))(void)             \
    {                                                                   \
        MODULE_NAME_SPACE(MODULE_SYMBOL_PREFIX(sym)) =                  \
            voba_make_symbol_cstr(#sym,voba___current_module);          \
        return;                                                         \
    }                                                                   \
    EXEC_ONCE(MODULE_NAME_SPACE(MODULE_SYMBOL_INIT_PREFIX(sym)))

#define DEF(v,expr)                                               \
    static voba_value_t v = VOBA_NIL;                             \
    static void init__begin__ ##v## __init_end()                  \
    {                                                             \
        v = expr;                                                 \
    }                                                             \
    EXEC_ONCE(init__begin__ ##v## __init_end)



