%option noinput nounput noyywrap 8bit nodefault
%option yylineno
%option reentrant bison-bridge bison-locations

%option never-interactive

%{
    #include "../ast.h"
    #include "parser.hpp"

    #define YY_USER_ACTION                                             \
      yylloc->first_line = yylloc->last_line;                          \
      yylloc->first_column = yylloc->last_column;                      \
      if (yylloc->last_line == yylineno)                               \
        yylloc->last_column += yyleng;                                 \
      else {                                                           \
        yylloc->last_line = yylineno;                                  \
        yylloc->last_column = yytext + yyleng - strrchr(yytext, '\n'); \
      }

    // Custom strdup because the one in cstdio is not standard(when compiling strictly C)
    char* custom_strdup(const char * c, int length)
    {
        char* dup = (char*)malloc(length + 1);

        if (dup != NULL)
           strcpy(dup, c);

        return dup;
    }
%}

%%
[ \t\n]            ; // ignore all whitespace
[,TF01()C&|~*+-]   {return yytext[0];} // all single charecter tokens will be passed as their ASCIIs for an easier use in bison (in my opinion, better readability)
"<="               {return T_LESS_EQ;}
"<=m"              {return T_MEASURED_LESS_EQ;}
"=0"               {return T_EQ_ZERO;}
"= 0"              {return T_EQ_ZERO;}
"->"               {return T_FORMULA_OP_IMPLICATION;}
"<->"              {return T_FORMULA_OP_EQUALITY;}
[a-zA-Z0-9]+       {yylval->T_STRING = custom_strdup(yytext, yyleng); return T_STRING;}
.                  {return yytext[0];} // bison will trigger an error if it's unrecognized symbol
%%
