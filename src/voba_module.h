#pragma once
// return an array of search directories.
voba_value_t voba_module_path();
voba_value_t voba_import(voba_value_t module_name,voba_value_t pwd);
voba_value_t* voba_module_symbol(voba_value_t module, voba_value_t symbol);

