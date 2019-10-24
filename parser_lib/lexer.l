%{
#include "../ast.h"
#include "parser.hpp"

// Custom strdup because the one in cstdio is not standard(when compiling strictly C)
char* custom_strdup(const char * c, int length)
{
    char* dup = (char*)malloc(length + 1);

    if (dup != NULL)
       strcpy(dup, c);

    return dup;
}
%}

%option noyywrap
%option never-interactive

%%
[ \t\n]            ; // ignore all whitespace
[,TF01()C&|~*+-]   {return yytext[0];} // all single charecter tokens will be passed as their ASCIIs for an easier use in bison (in my opinion, better readability)
"<="               {return T_LESS_EQ;}
"<=m"              {return T_MEASURED_LESS_EQ;}
"=0"               {return T_EQ_ZERO;}
"= 0"              {return T_EQ_ZERO;}
"->"               {return T_FORMULA_OP_IMPLICATION;}
"<->"              {return T_FORMULA_OP_EQUALITY;}
[a-zA-Z0-9]+       {yylval.sval = custom_strdup(yytext, yyleng); return T_STRING;}
.                  {return yytext[0];} // bison will trigger an error if it's unrecognized symbol
%%

void set_input_string(const char* in)
{
    yy_scan_string(in);
}

void end_lexical_scan()
{
    yy_delete_buffer(YY_CURRENT_BUFFER);
}
