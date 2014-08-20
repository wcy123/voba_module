%option nodebug
%option noyywrap
%{
#include <math.h>
#include <string.h>
#include <assert.h>
#define YY_DECL voba_value_t module_lex(voba_value_t module, voba_str_t * content, yyscan_t yyscanner)
#define YY_INPUT(buf,result,max_size)                           \
             {                                                  \
             int c = getchar();                                 \
             result = (c == EOF) ? YY_NULL : (buf[0] = c, 1);   \
             }
int fileno(FILE *stream);
%}
%option outfile="voba_module_lex.c"
%option reentrant
%x comment str
DIGIT    [0-9]
ID       [a-zA-Z_><+*/-][a-zA-Z0-9?_><+*/-]*
%%

^[[:space:]]*XX\({ID}\).*$  {
     int i = 0; while(yytext[i++] != '(');
     int j = i; while(yytext[++j] != ')');
     voba_str_t* symbol_name = (voba_str_from_data(yytext + i, j - i));
     voba_value_t s = voba_make_symbol(symbol_name,module);
     voba_symbol_set_value(s,s);
}
^#define[[:space:]]+IMP[[:space:]]*\".*\".*$  {
     int i = 0; while(yytext[i++] != '"');
     int j = i; while(yytext[++j] != '"');
     voba_str_t* imp = voba_strdup(voba_str_from_data(yytext + i, j - i));
     voba_value_t s = voba_make_symbol_cstr("__imp__",module);
     voba_symbol_set_value(s, voba_make_string(imp));
}
\n   {
}
. {
   //fprintf(stderr,"unrecoganized token %d %s\n", yytext[0], yytext);
}
%%
void v_______foo()
{
   // avoid warning, unused function yyuninput
   if(0){
     yyunput(1,NULL,NULL);
     input(NULL);
   }
}
