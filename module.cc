#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <iomanip>
#include <map>
#include <cstdint>
#include <cstring>
#include <gc.h>
#include <vhash.h>
#include <exec_once.h>
#include <voba/include/value.h>
using namespace std;


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
        ret = voba_strcat_data(ret,".so",3);
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
            ret = voba_strcat_data(ret,".so",3);
            if(is_file_readable(ret)){
                break;
            }else{
                ret = NULL;
            }
        }
    }
    if(ret != NULL){
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
static inline void voba__builtin_init_module(voba_value_t module,voba_value_t cwd,voba_value_t basename)
{
    voba_symbol_set_value(VOBA_SYMBOL("__dir__",module), cwd);
    voba_symbol_set_value(VOBA_SYMBOL("__file__",module), basename);
}
static voba_value_t module_cwd = VOBA_NIL;
EXEC_ONCE_DO(
    const size_t sz  = 64*1024;
    char * p = (char*)malloc(sz);
    if(!p) abort();
    char * cwd = getcwd(p,sz);
    if(!cwd) abort();
    module_cwd = voba_make_array_0();
    voba_array_push(module_cwd,voba_make_string(voba_strdup(voba_str_from_cstr(cwd))));
    free(p);
    );

extern "C" voba_value_t voba_load_module(const char * module_name,voba_value_t module)
{
    voba_value_t ret = VOBA_NIL;
    int64_t len = voba_array_len(module_cwd);
    assert(len > 0);
    voba_value_t cwd = voba_array_at(module_cwd,len-1);
    voba_str_t* os_file = find_file(voba_str_from_cstr(module_name),voba_value_to_str(cwd));
    cerr <<  __FILE__ << ":" << __LINE__ << ": [" << __FUNCTION__<< "] "
         << "module_name "  << module_name << " "
         << "cwd "  << hex << "0x" << cwd << dec << " "
         << "len "  << len << " "
         << "os_file "  << os_file << " "
         << endl;
    if(!os_file){
        VOBA_THROW(
            VOBA_CONST_CHAR("cannot find module so file."
                            " module_name = " ),
            voba_str_from_cstr(module_name)
            );
    }
    voba_value_t filename =  voba_make_string(os_file);
    voba_value_t dir_name_base_name = dir_and_base_name(voba_value_to_str(filename));
    voba_value_t dir_name = voba_head(dir_name_base_name);
    voba_value_t basename = voba_tail(dir_name_base_name);
    // push cwd
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
    try{
        cerr <<  __FILE__ << ":" << __LINE__ << ": [" << __FUNCTION__<< "] "
             << "dir_name "  << hex << "0x" << dir_name << dec << " "
             << endl;
        voba_array_push(module_cwd,dir_name);
        voba__builtin_init_module(module,cwd,basename);
        ret = init(module);
        voba_array_pop(module_cwd);
    }catch(voba::exception e) {
        voba_array_pop(module_cwd);
        throw(e);
    }
    return ret;
}

voba_value_t voba_modules = VOBA_NIL;
EXEC_ONCE_DO(voba_modules = voba_make_hash(););

EXEC_ONCE_START;

