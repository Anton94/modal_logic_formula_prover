%{
#include <cstdio>
#include <iostream>

#include "modal_logic_formula_ast.h"

using namespace std;

// stuff from flex that bison needs to know about:
extern int yylex();
extern int yyparse();

void yyerror(const char *s);

NFormula* whole_formula = nullptr;
%}

%union {
    char *sval;
    NFormula *formula;
    NTerm *term;
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

%type <formula> formula
%type <term> term

%%
modal_logic_formula
    : formula {
		whole_formula = $1;
        cout << "done with the formula!" << endl;
    }
  ;
formula
    : 'T' {
        cout << "read atomic formula T!" << endl;
		$$ = new NFormula(formula_operation_t::constant_true);
    }
    | 'F' {
        cout << "read atomic formula F!" << endl;
		$$ = new NFormula(formula_operation_t::constant_false);
    }
    | 'C' '(' term ',' term ')' {
        cout << "read atomic formula C" << endl;
        $$ = new NFormula(formula_operation_t::contact, $3, $5);
    }
    | T_LESS_EQ '(' term ',' term ')'  {
        cout << "read atomic formula <=" << endl;
        $$ = new NFormula(formula_operation_t::less_eq, $3, $5);
    }
    | '(' formula '*' formula ')' {
        cout << "read binary formula &" << endl;
        $$ = new NFormula(formula_operation_t::conjunction, $2, $4);
    }
    | '(' formula '|' formula ')' {
        cout << "read binary formula |" << endl;
        $$ = new NFormula(formula_operation_t::disjunction, $2, $4);
    }
    | '~' '(' formula ')' {
        cout << "read unary formula ~" << endl;
        $$ = new NFormula(formula_operation_t::negation, $3);
    }
    |  '(' formula T_FORMULA_OP_IMPLICATION formula ')' {
        cout << "read binary formula ->" << endl;
        $$ = new NFormula(formula_operation_t::implication, $2, $4);
    }
    |  '(' formula T_FORMULA_OP_EQUALITY formula ')' {
        cout << "read binary formula <->" << endl;
        $$ = new NFormula(formula_operation_t::equality, $2, $4);
    }
  ;
term
    : '1' {
        cout << "read atomic term 1!" << endl;
        $$ = new NTerm(term_operation_t::constant_true);
    }
    | '0' {
        cout << "read atomic term 0!" << endl;
        $$ = new NTerm(term_operation_t::constant_false);
    }
    | T_STRING {
        cout << "read variable: " << $1 << endl;
        $$ = new NTerm(term_operation_t::variable_);
		$$->variable = $1;
		free($1);
    }
    |  '(' term '*' term ')' {
        cout << "read binary term *" << endl;
        $$ = new NTerm(term_operation_t::intersaction_, $2, $4);
    }
    |  '(' term '+' term ')' {
        cout << "read binary term +" << endl;
        $$ = new NTerm(term_operation_t::union_, $2, $4);
    }
    | '-' '(' term ')'{
        cout << "read unary term -" << endl;
        $$ = new NTerm(term_operation_t::star_, $3);
    }
  ;
%%

int main(int, char**) {
  // lex through the input:
  yyparse();

  delete whole_formula;
}

void yyerror(const char *s) {
  cout << "EEK, parse error!  Message: " << s << endl;
  // might as well halt now:
  exit(-1);
}
