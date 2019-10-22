%{
    #include <cstdio>
    #include <iostream>
    using namespace std;

    // stuff from flex that bison needs to know about:
    extern int yylex();
    extern int yyparse();

    void yyerror(const char *s);
%}

%union {
    char *sval;
}

%token <sval> T_STRING

%token T_LESS_EQ
%token T_MEASURED_LESS_EQ
%token T_EQ_ZERO
%token T_FORMULA_OP_IMPLICATION T_FORMULA_OP_EQUALITY

// the operators precedence is from low to hight (w.r.t. the order (by line) in which they are defined)
%left T_FORMULA_OP_IMPLICATION T_FORMULA_OP_EQUALITY
%left '|' '+'
%left '&' '*'
%right '~' '-'

%%
modal_logic_formula
    : formula {
        cout << "done with the formula!" << endl;
    }
  ;
formula
    : 'T' {
        cout << "read atomic formula T!" << endl;
    }
    | 'F' {
        cout << "read atomic formula F!" << endl;
    }
    | 'C' '(' term ',' term ')' {
        cout << "read atomic formula C" << endl;
    }
    | T_LESS_EQ '(' term ',' term ')'  {
        cout << "read atomic formula <=" << endl;
    }
    | '(' formula '*' formula ')' {
        cout << "read binary formula &" << endl;
    }
    | '(' formula '|' formula ')' {
        cout << "read binary formula |" << endl;
    }
    | '~' '(' formula ')' {
        cout << "read unary formula ~" << endl;
    }
    |  '(' formula T_FORMULA_OP_IMPLICATION formula ')' {
        cout << "read binary formula ->" << endl;
    }
    |  '(' formula T_FORMULA_OP_EQUALITY formula ')' {
        cout << "read binary formula <->" << endl;
    }
  ;
term
    : '1' {
        cout << "read atomic term 1!" << endl;
    }
    | '0' {
        cout << "read atomic term 0!" << endl;
    }
    | T_STRING {
        cout << "read variable: " << $1 << endl; free($1);
    }
    |  '(' term '*' term ')' {
        cout << "read binary term *" << endl;
    }
    |  '(' term '+' term ')' {
        cout << "read binary term +" << endl;
    }
    | '-' '(' term ')'{
        cout << "read unary term -" << endl;
    }
  ;
%%

int main(int, char**) {
  // lex through the input:
  yyparse();
}

void yyerror(const char *s) {
  cout << "EEK, parse error!  Message: " << s << endl;
  // might as well halt now:
  exit(-1);
}
