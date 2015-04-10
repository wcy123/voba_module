#include <dlfcn.h>
#include <limits.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#define EXEC_ONCE_TU_NAME "voba.module"
#define EXEC_ONCE_DEPENDS {"voba.value"}
#include <exec_once.h>
#include <voba/value.h>
#include "module.h"
static int voba_module_debug = 0;
EXEC_ONCE_PROGN{
    if(getenv("VOBA_MODULE_DEBUG") != NULL){
        voba_module_debug = atoi(getenv("VOBA_MODULE_DEBUG"));
    }
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


static inline voba_value_t dir_and_base_name(voba_str_t* filename);
static inline voba_str_t* compose_filename(voba_str_t* path,
                                           voba_str_t* module_dirname,
                                           voba_str_t* module_basename,
                                           voba_str_t* prefix,
                                           voba_str_t* suffix);
voba_str_t* voba_find_file(voba_value_t search_path,
                           voba_str_t * module_name,
                           voba_str_t * pwd,
                           voba_str_t * prefix,
                           voba_str_t * suffix,
                           int resolv_realpath,
                           voba_value_t attempts)
{
    voba_str_t * ret = NULL;
    voba_value_t tmp = dir_and_base_name(module_name);
    voba_value_t module_dirname = voba_head(tmp);
    voba_value_t module_basename = voba_tail(tmp);
    if(module_name->data[0] == '.'){
        search_path = voba_make_array_1(voba_make_string(pwd));
    }
    int64_t len = voba_array_len(search_path);
    voba_value_t * p = voba_array_base(search_path);
    for(int64_t i = 0; i < len ; ++i){
        ret = compose_filename(voba_value_to_str(p[i]),
                               voba_value_to_str(module_dirname),
                               voba_value_to_str(module_basename),
                               prefix,
                               suffix);
        int is_ok = is_file_readable(ret);
        if(voba_module_debug){
            fprintf(stderr,__FILE__ ":%d:[%s] trying to load so file %s for module %s %s.\n"
                    , __LINE__, __FUNCTION__
                    ,voba_str_to_cstr(ret)
                    ,voba_str_to_cstr(module_name)
                    ,is_ok?" is found":" is not found");
        }
        if(is_ok){
            break;
        }else{
            voba_array_push(attempts, voba_make_string(ret));
            ret = NULL;
        }
    }
    if(ret != NULL && resolv_realpath){
        char * p = realpath(voba_str_to_cstr(ret), NULL);
        if(p == NULL){
            VOBA_THROW(
                VOBA_CONST_CHAR("cannot resolve realpath of "),
                ret);
        }
        ret = voba_strdup(voba_str_from_cstr(p));
        free(p);
    }
    return ret;
}
static inline voba_str_t* compose_filename(voba_str_t* path,
                                           voba_str_t* module_dirname,
                                           voba_str_t* module_basename,
                                           voba_str_t* prefix,
                                           voba_str_t* suffix)
{
    voba_str_t* ret =NULL;
    ret = voba_strdup(path);
    ret = voba_strcat_char(ret,'/');
    ret = voba_strcat(ret,module_dirname);
    ret = voba_strcat_char(ret,'/');
    ret = voba_strcat(ret,prefix);
    ret = voba_strcat(ret,module_basename);
    ret = voba_strcat(ret,suffix);
    return ret;
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
extern voba_value_t voba_init_path_from_env(const char* path)
{
    return split(voba_str_from_cstr(path));
}
static voba_value_t voba_path = VOBA_NIL;
EXEC_ONCE_PROGN{
    const char * env = getenv("VOBA_PATH");
    if(!env){ env = ".";}
    voba_path = voba_init_path_from_env(env);
}
voba_value_t voba_module_path()
{
    return voba_path;
}
static inline
voba_value_t dir_and_base_name(voba_str_t* filename)
{
    // return a pair, with dir name and basename.
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
static voba_value_t module_cwd = VOBA_NIL;
EXEC_ONCE_PROGN{
    const size_t sz  = 64*1024;
    char * p = (char*)malloc(sz);
    if(!p) abort();
    char * cwd = getcwd(p,sz);
    if(!cwd) abort();
    module_cwd = voba_make_array_0();
    voba_array_push(module_cwd,voba_make_string(voba_strdup(voba_str_from_cstr(cwd))));
    free(p);
};
static VOBA_FUNC voba_value_t voba_init_module(voba_value_t fun, voba_value_t args, voba_value_t* next_fun, voba_value_t next_args[])
{
    voba_value_t (*init)(voba_value_t);
    *(void **) (&init) = (void*)(voba_tuple_at(fun,0));
    voba_value_t module = voba_tuple_at(fun,1);
    return init(module);
}
static VOBA_FUNC voba_value_t pop_cwd(voba_value_t fun, voba_value_t args, voba_value_t* next_fun, voba_value_t next_args[])
{
    voba_value_t cwd = voba_array_at(fun,0);
    voba_array_pop(cwd);
    return VOBA_NIL;
}
static voba_value_t voba_load_module(const char * module_name,voba_value_t module)
{
    int64_t len = voba_array_len(module_cwd);
    assert(len > 0);
    voba_value_t cwd = voba_array_at(module_cwd,len-1);
    voba_str_t* os_file = NULL;
    if(voba_module_debug){
        fprintf(stderr,__FILE__ ":%d:[%s] start loading module %s, cwd = %s\n", __LINE__, __FUNCTION__,module_name,
                voba_str_to_cstr(voba_value_to_str(cwd)));
    }
    voba_value_t attempts = voba_make_array_0();
    os_file = voba_find_file(voba_module_path(),
                             voba_str_from_cstr(module_name),
                             voba_value_to_str(cwd),
                             VOBA_CONST_CHAR("lib"),
                             VOBA_CONST_CHAR(".so"),
                             0, // resolve realpath
                             attempts);
    if(!os_file){
        int64_t len2 = voba_array_len(attempts);
        voba_str_t * s = voba_str_empty();
        for(int64_t i = 0; i < len2; ++i){
            s = voba_strcat(s, VOBA_CONST_CHAR("    "));
            s = voba_strcat(s, voba_value_to_str(voba_array_at(attempts,i)));
            s = voba_strcat(s, VOBA_CONST_CHAR("\n"));
        }
        VOBA_THROW(
            VOBA_CONST_CHAR("cannot find module so file."
                            " module_name = " ),
            voba_str_from_cstr(module_name),
            VOBA_CONST_CHAR(", attemps to load from :\n"),
            s
            );
    }
    if(voba_module_debug){
        fprintf(stderr,__FILE__ ":%d:[%s] found module %s\n", __LINE__, __FUNCTION__,voba_str_to_cstr(os_file));
    }
    voba_value_t filename =  voba_make_string(os_file);
    voba_value_t dir_name_base_name = dir_and_base_name(voba_value_to_str(filename));
    voba_value_t dir_name = voba_head(dir_name_base_name);
    voba_value_t basename = voba_tail(dir_name_base_name);
    void *handle;
    handle = dlopen(voba_str_to_cstr(voba_value_to_str(filename)), RTLD_LAZY);
    if(!handle){
        VOBA_THROW(VOBA_CONST_CHAR("dlopen failure:"),
                   voba_str_from_cstr(dlerror()));
    }
    dlerror();
    char *error = NULL;
    voba_value_t (*init)(voba_value_t);
    *(void **) (&init) = dlsym(handle, "voba_init");
    if ((error = dlerror()) != NULL)  {
        VOBA_THROW(VOBA_CONST_CHAR("dlsym(voba_init) failure: "),
                   voba_str_from_cstr(dlerror()));
    }
    voba_symbol_set_value(VOBA_SYMBOL("__dir__",module), dir_name);
    voba_symbol_set_value(VOBA_SYMBOL("__file__",module), basename);
    voba_array_push(module_cwd,dir_name);
    exec_once_init(); // initialize EXEC_ONCE_PROGN in the module
    voba_value_t ret = voba_try_catch(
        voba_make_closure_2(voba_init_module,((voba_value_t)init),module),
        voba_make_closure_1(pop_cwd,module_cwd)
        ); /// @bug, add try.catch.final instead of try.catch?
    voba_array_pop(module_cwd);
    return ret;
}
static inline void voba_check_symbol_defined(voba_value_t m, voba_value_t symbols);
/**
 
 @bug there is a problem, one implementation could potentially be loaded
 twice with different ids. `dlopen` might still return the same instance

 */
voba_value_t voba_import_module(const char * module_name, const char * module_id, voba_value_t symbol_names)
{
    voba_value_t id = voba_make_string(voba_str_from_cstr(module_id));
    voba_value_t name = voba_make_string(voba_str_from_cstr(module_name));
    voba_value_t m = voba_hash_find(voba_modules,id);
    if(voba_module_debug){
        fprintf(stderr,__FILE__ ":%d:[%s] start importing module %s(%s)\n", __LINE__, __FUNCTION__,
                module_name,module_id);

    }
    if(voba_is_nil(m)){
        m = voba_make_symbol_table();
        voba_hash_insert(voba_modules,id,m);
        voba_symbol_set_value(VOBA_SYMBOL("__id__",m), id);  // id is voba_value_t of module_id
        voba_symbol_set_value(VOBA_SYMBOL("__name__",m), name); // name is voba_value_t of module_name
        int64_t len = voba_tuple_len(symbol_names);
        for(int64_t i = 0; i < len; ++i){
            // create an un-interned symbol
            voba_str_t * symbol_name = voba_value_to_str(voba_tuple_at(symbol_names,i));
            voba_value_t s = voba_make_symbol(symbol_name,m);
            // initialized it with undef
            voba_symbol_set_value(s,VOBA_UNDEF);
            if(voba_module_debug){
                fprintf(stderr,__FILE__ ":%d:[%s] create symbol %s(0x%lx) for module %s(%s), symbol value is undef.\n" 
                        ,__LINE__, __FUNCTION__,
                        symbol_name->data,s,
                        module_name,module_id);

            }
        }
        voba_load_module(module_name,m);
        voba_check_symbol_defined(m,symbol_names);
    }else{
        if(voba_module_debug){
            fprintf(stderr,__FILE__ ":%d:[%s] module %s(%s) is already loaded or being loaded\n", __LINE__, __FUNCTION__,
                module_name,module_id);
        }
    }
    if(voba_module_debug){
        fprintf(stderr,__FILE__ ":%d:[%s] importing module %s(%s) is done\n", __LINE__, __FUNCTION__,
                module_name,module_id);

    }
    return m;
}
static inline void voba_check_symbol_defined(voba_value_t m, voba_value_t symbols)
{
    voba_value_t undefined_symbols = voba_make_array_0();
    int64_t len = voba_tuple_len(symbols);
    for(int64_t i = 0; i < len; ++i){
        voba_value_t s = voba_lookup_symbol(voba_tuple_at(symbols,i),m);
        assert(voba_is_a(s,voba_cls_symbol));
        if(voba_is_undef(voba_symbol_value(s))){
            voba_array_push(undefined_symbols, s);
        }
    }
    len = voba_array_len(undefined_symbols);
    if(len > 0 ) {
        voba_str_t *s = voba_str_empty();
        for(int64_t i = 0; i < len ; ++i){
            s = voba_strcat_char(s,' ');
            s = voba_strcat(s,voba_value_to_str(voba_symbol_name(voba_array_at(undefined_symbols,i))));
        }
        VOBA_THROW(
            VOBA_CONST_CHAR("import_module: undefined symbol(s)."),
            s);
    }
    return;
}

voba_value_t voba_modules = VOBA_NIL;
EXEC_ONCE_PROGN{
    voba_modules = voba_make_hash();
}
static voba_value_t all_symbols = VOBA_NIL;
EXEC_ONCE_PROGN{
    all_symbols = voba_make_hash();
};
// this function is only useful for C module, I am not sure the use
// case. TODO, make it useful also in compiler.c
void voba_define_module_symbol(voba_value_t symbol, voba_value_t value, const char * file , int line)
{
    if(voba_module_debug){
        fprintf(stderr, "%s:%d: define symbol %s to 0x%lx\n", file, line,
                voba_str_to_cstr(voba_value_to_str(voba_symbol_name(symbol))),
                value);
    }
    voba_value_t v = voba_hash_find(all_symbols,symbol);
    if(voba_is_nil(v)){
        voba_hash_insert(all_symbols,symbol,voba_make_array_3(value,voba_make_string(voba_str_from_cstr(file)),voba_make_i32(line)));
        // good the symbol is not defined.
    }else{
        assert(0&&"TODO what if the symbol is already defined.");
    }
    voba_symbol_set_value(symbol,value);
}
inline static 
int is_id_char(char c)
{
    return 
        (c >='0' && c <= '9') ||
        (c >='A' && c <= 'Z') ||
        (c >='a' && c <= 'z') ||
        (c == '_');
}
inline static
char id_char_high(char c)
{
    return "0123456789ABCDEF"[(c&0xF0)>>4];
}
inline static
char id_char_low(char c)
{
    return "0123456789ABCDEF"[(c&0x0F)];
}
voba_str_t * voba_c_id_encode(voba_str_t * str)
{
    voba_str_t * ret = voba_str_empty();
    int32_t i = 0;
    assert(str->data!=NULL);
    for(i = 0; i < str->len ; ++i){
        if(is_id_char(str->data[i])){
            ret = voba_strcat_char(ret, str->data[i]);
        }else{
            ret = voba_strcat_char(ret, '_');
            ret = voba_strcat_char(ret, id_char_high(str->data[i]));
            ret = voba_strcat_char(ret, id_char_low(str->data[i]));
        }
    }
    return ret;
}
static inline 
int is_digit(char c)
{
    return ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F'));
}
static inline 
char d2char(char c)
{
    if(c >= '0' && c <= '9') return c - '0';
    else return c - 'A' + 10;
}
static inline 
char digit2char(char h,char l)
{
    return (d2char(h) << 4) + d2char(l) ;
}

voba_str_t * voba_c_id_decode(voba_str_t * str)
{
    voba_str_t * ret = voba_str_empty();
    int32_t i = 0;
    assert(str->data!=NULL);
    while(i < str->len ){
        if(str->data[i] == '_' && i + 2 < str->len && 
           is_digit(str->data[i+1]) && 
           is_digit(str->data[i+2])
           )
        {
            char h = str->data[i+1];
            char l = str->data[i+2];
            char c = digit2char(h,l);
            ret = voba_strcat_char(ret,c);
            i += 3;
        }else{
            ret = voba_strcat_char(ret,str->data[i]);
            ++i;
        }
    }
    return ret;
}


