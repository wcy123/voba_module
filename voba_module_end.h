// this file will be included multiple times by every module so that
// there is no `#pragram once` protection.
#include "voba_module.h"

// mandatory input macros MODULE_NAME and SYMBOL_TABLE
#ifndef MODULE_NAME
#error "MODULE_NAME is not defined"
#endif
#ifndef SYMBOL_TABLE
#error "SYMBOL_TABLE is not defined"
#endif
#ifndef IMP
#error "IMP is not defined"
#endif

// optional input macros
#ifndef MODULE_NAME_SPACE
#define MODULE_NAME_SPACE(s) s
#endif
#ifndef MODULE_SYMBOL_PREFIX
#define MODULE_SYMBOL_PREFIX(sym) s_##sym
#endif
#ifndef MODULE_SYMBOL_VALUE_PREFIX
#define MODULE_SYMBOL_VALUE_PREFIX(sym) v_##sym
#endif
#ifndef MODULE_SYMBOL_INIT_PREFIX
#define MODULE_SYMBOL_INIT_PREFIX(sym) sym ##__init
#endif
#ifndef MODULE_INIT_IMPORT
#define MODULE_INIT_IMPORT v_init_import
#endif

// do it
DECLARE_SYMBOL_TABLE(SYMBOL_TABLE)

// clear input macros
#undef MODULE_NAME
#undef SYMBOL_TABLE
#undef IMP
#undef MODULE_NAME_SPACE
#undef MODULE_NAME_SPACE
#undef MODULE_SYMBOL_PREFIX
#undef MODULE_SYMBOL_VALUE_PREFIX
#undef MODULE_SYMBOL_INIT_PREFIX


