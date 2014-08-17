#include <stdlib.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "voba_value.h"
#include "voba_module.h"
#include "voba_module_lex.c"
static inline
voba_value_t make_module(voba_value_t module_name)
{
    return voba_make_symbol_table();
}
static inline int is_file_readable(voba_str_t* path)
{
    const char * p = voba_str_to_cstr(path);
    struct stat buf;
    int r = stat(p,&buf);
    if(r != 0) {
        return 0;
    }
    if(! (S_ISREG(buf.st_mode))) {
        return 0;
    }
    FILE * fp = fopen(p,"r");
    if(fp) {
        fclose(fp);
        return 1;
    }
    return 0;
}
static inline voba_value_t split(voba_str_t* s)
{
    voba_value_t ret = voba_make_array_0();
    uint32_t i  = 0, j = 0;
    s = voba_str_replace(s,':','\0');
    for(j = 0, i = 0; i < s->len + 1; ++i){
        if(s->data[i] == '\0'){
            uint32_t to = s->data[i-1] == '/'?i-1:i;
            voba_str_t * sj =
                voba_substr(s,j,to - j);
            j = i + 1;
            voba_array_push(ret,voba_make_string(sj));
        }
    }
    return ret;
}
voba_value_t voba_module_path()
{
    static voba_value_t voba_path = VOBA_NIL;
    if(!voba_is_nil(voba_path)){
        return voba_path;
    }
    const char * env = getenv("VOBA_PATH");
    if(env == NULL) {
        env = ".";
    }
    voba_path = split(voba_str_from_cstr(env));
    return voba_path;
}
static inline voba_str_t* find_file(voba_str_t * module_name, voba_str_t * pwd)
{
    voba_str_t * ret = NULL;
    module_name = voba_str_replace(module_name,'.','/');
    if(module_name->data[0] == '/'){
        ret = voba_strcat(voba_strdup(pwd),module_name);
        ret = voba_strcat_data(ret,".h",2);
        if(!is_file_readable(ret)){
            ret = NULL;
        }
    }else {
        voba_value_t voba_path = voba_module_path();
        int64_t len = voba_array_len(voba_path);
        voba_value_t * p = voba_array_base(voba_path);
        for(int64_t i = 0; i < len ; ++i){
            ret = voba_strdup(voba_value_to_str(p[i]));
            ret = voba_strcat_char(ret,'/');
            ret = voba_strcat(ret,module_name);
            ret = voba_strcat_data(ret,".h",2);
            if(is_file_readable(ret)){
                break;
            }else{
                ret = NULL;
            }
        }
    }
    return ret;
}
static inline
voba_value_t split_path(voba_str_t* filename)
{
    uint32_t i = 0, j = 0;
    for(i = 0 ; i < filename->len; ++i){
        j = filename->len - i - 1;
        if( filename->data[j] == '/'){
            break;
        }
    }
    if(i == filename->len){
        return voba_make_pair(voba_make_string(voba_str_from_data(".",1)),
                              voba_make_string(filename));
    }
    return voba_make_pair(voba_make_string(voba_substr(filename,0,j)),
                          voba_make_string(voba_substr(filename,j+1, filename->len - j -1)));
}
static inline
voba_str_t* slurp_file(voba_str_t* filename)
{
    int c = 0;
    voba_str_t * ret = voba_str_empty();
    FILE * fp = fopen(voba_str_to_cstr(filename),"r");
    if(fp == NULL) return ret;
    while((c = fgetc(fp))!= EOF){
        ret = voba_strcat_char(ret,(char)c);
    }
    return ret;
}
static inline
voba_value_t parse_module(voba_value_t name, voba_str_t* filename)
{
    voba_value_t module = make_module(name);
    voba_value_t dir_name_base_name = split_path(filename);
    voba_symbol_set_value(voba_make_symbol_cstr("__imp__",module), VOBA_NIL);
    voba_symbol_set_value(voba_make_symbol_cstr("__name__",module), name);
    voba_symbol_set_value(voba_make_symbol_cstr("__loaded__",module),
                          voba_make_symbol_cstr("not-loaded",module));
    voba_symbol_set_value(voba_make_symbol_cstr("__dir__",module), voba_head(dir_name_base_name));
    voba_symbol_set_value(voba_make_symbol_cstr("__file__",module), voba_tail(dir_name_base_name));
    voba_str_t * content = slurp_file(filename);
    voba_symbol_set_value(voba_make_symbol_cstr("__source__",module), voba_make_string(content));
    void * scanner;
    yylex_init(&scanner);
    yy_scan_bytes(content->data,content->len,scanner);
    module_lex(module,content,scanner);
    yylex_destroy(scanner);
    return module;
}
static inline
voba_value_t voba_load_module(voba_value_t module)
{
    voba_str_t * imp = voba_value_to_str(voba_symbol_value(voba_make_symbol_cstr("__imp__",module)));
    voba_str_t * fullpath = voba_strdup(voba_value_to_str(voba_symbol_value(voba_make_symbol_cstr("__dir__",module))));
    fullpath = voba_strcat_char(fullpath,'/');
    fullpath = voba_strcat(fullpath,imp);
    if(!is_file_readable(fullpath)){
        return VOBA_NIL;
    }
    void *handle;
    handle = dlopen(voba_str_to_cstr(fullpath), RTLD_LAZY);
    if(!handle){
        // TODO, throw exception
        fprintf(stderr,__FILE__ ":%d:[%s] dlopen error: %s", __LINE__, __FUNCTION__, dlerror());
        return VOBA_NIL;
    }
    dlerror();
    char *error = NULL;
    voba_value_t (*init)(voba_value_t);
    *(void **) (&init) = dlsym(handle, "voba_init");
    if ((error = dlerror()) != NULL)  {
        // TODO, throw exception
        fprintf(stderr,__FILE__ ":%d:[%s] dlsym(voba_init) error: %s", __LINE__, __FUNCTION__, error);
        return VOBA_NIL;
    }
    voba_value_t ret = init(module);
    voba_symbol_set_value(voba_make_symbol_cstr("__loaded__",module),ret);
    return ret;
}
voba_value_t voba_import(voba_value_t module_name,voba_value_t pwd)
{
    static voba_value_t the_hash_table = VOBA_NIL;
    fprintf(stderr,__FILE__ ":%d:[%s] %s\n", __LINE__, __FUNCTION__, voba_value_to_str(module_name)->data);

    if(voba_is_nil(the_hash_table)) {
        the_hash_table = voba_make_hash();
        fprintf(stderr,__FILE__ ":%d:[%s] the_hash_table 0x%lx\n", __LINE__, __FUNCTION__,
            the_hash_table);
    }
    voba_value_t filename =  voba_make_string(find_file(voba_value_to_str(module_name),voba_value_to_str(pwd)));
    if(voba_is_nil(filename)) return VOBA_NIL;
    fprintf(stderr,__FILE__ ":%d:[%s] find file %s\n", __LINE__, __FUNCTION__, voba_value_to_str(filename)->data);
    voba_value_t cache = voba_hash_find(the_hash_table,filename);
    if(!voba_is_nil(cache)){
        return voba_tail(cache);
    }
    fprintf(stderr,__FILE__ ":%d:[%s] init %s\n", __LINE__, __FUNCTION__, voba_value_to_str(filename)->data);
    voba_value_t module = parse_module(module_name,voba_value_to_str(filename));
    voba_value_t imp  = voba_symbol_value(voba_make_symbol_cstr("__imp__",module));
    if(voba_is_nil(imp)){
        fprintf(stderr,__FILE__ ":%d:[%s] module does not have implementation.", __LINE__, __FUNCTION__);
        return VOBA_NIL;
    }
    voba_hash_insert(the_hash_table,filename, module);
    voba_load_module(module);
    return module;
}
voba_value_t* voba_module_symbol(voba_value_t module, voba_value_t symbol)
{
    return NULL;
}
