// this file will be included multiple times by every module so that
// there is no `#pragram once` protection.
//#include <voba/include/module.h>
#ifndef VOBA__COMPILE_HEADER // only defined for the internal tool compile_voba_header.sh
#include "module.h"
// mandatory input macros MODULE_NAME and SYMBOL_TABLE
#ifndef VOBA_MODULE_ID
#error "VOBA_MODULE_ID is not defined"
#endif
#ifndef VOBA_SYMBOL_TABLE
#error "VOBA_SYMBOL_TABLE is not defined"
#endif
#ifndef VOBA_MODULE_NAME
#error "VOBA_MODULE_NAME is not defined"
#endif

// optional input macros
#ifndef VOBA_MODULE_NAME_SPACE
#define VOBA_MODULE_NAME_SPACE(s) s
#endif
#ifndef VOBA_MODULE_SYMBOL_PREFIX
#define VOBA_MODULE_SYMBOL_PREFIX(sym) s_##sym
#endif

// do it
VOBA_DECLARE_SYMBOL_TABLE(VOBA_SYMBOL_TABLE)

// clear input macros
#undef VOBA_MODULE_ID
#undef VOBA_SYMBOL_TABLE
#undef VOBA_MODULE_NAME
#undef VOBA_MODULE_NAME_SPACE
#undef VOBA_MODULE_SYMBOL_PREFIX
#endif

