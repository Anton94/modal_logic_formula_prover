%{
    #include <cstdio>
    #include <iostream>

    #include "modal_logic_formula_ast.h"

    using namespace std;

    // stuff from flex that bison needs to know about:
    extern int yylex();
    extern int yyparse();

    void yyerror(const char *s);

%}

%union {
    char *sval;
    struct ast_node * ast;
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

%type <ast> formula term

%%
modal_logic_formula
    : formula {
        cout << "done with the formula!" << endl;
    }
  ;
formula
    : 'T' {
        cout << "read atomic formula T!" << endl;
        $$ = new_ast_atomic_formula_node(formula_operation_t::constant_true, nullptr, nullptr);
    }
    | 'F' {
        cout << "read atomic formula F!" << endl;
        $$ = new_ast_atomic_formula_node(formula_operation_t::constant_false, nullptr, nullptr);
    }
    | 'C' '(' term ',' term ')' {
        cout << "read atomic formula C" << endl;
        $$ = new_ast_atomic_formula_node(formula_operation_t::contact, $3, $5);
    }
    | T_LESS_EQ '(' term ',' term ')'  {
        cout << "read atomic formula <=" << endl;
        $$ = new_ast_atomic_formula_node(formula_operation_t::less_eq, $3, $5);
    }
    | '(' formula '*' formula ')' {
        cout << "read binary formula &" << endl;
        $$ = new_ast_formula_node(formula_operation_t::conjunction, $2, $4);
    }
    | '(' formula '|' formula ')' {
        cout << "read binary formula |" << endl;
        $$ = new_ast_formula_node(formula_operation_t::disjunction, $2, $4);
    }
    | '~' '(' formula ')' {
        cout << "read unary formula ~" << endl;
        $$ = new_ast_formula_node(formula_operation_t::negation, $2, nullptr);
    }
    |  '(' formula T_FORMULA_OP_IMPLICATION formula ')' {
        cout << "read binary formula ->" << endl;
        $$ = new_ast_formula_node(formula_operation_t::implication, $2, $4);
    }
    |  '(' formula T_FORMULA_OP_EQUALITY formula ')' {
        cout << "read binary formula <->" << endl;
        $$ = new_ast_formula_node(formula_operation_t::equality, $2, $4);
    }
  ;
term
    : '1' {
        cout << "read atomic term 1!" << endl;
        $$ = new_ast_term_atomic_node(term_operation_t::constant_true, nullptr);
    }
    | '0' {
        cout << "read atomic term 0!" << endl;
        $$ = new_ast_term_atomic_node(term_operation_t::constant_false, nullptr);
    }
    | T_STRING {
        cout << "read variable: " << $1 << endl; free($1);
        $$ = new_ast_term_atomic_node(term_operation_t::variable_, $1);
    }
    |  '(' term '*' term ')' {
        cout << "read binary term *" << endl;
        $$ = new_ast_term_node(term_operation_t::intersaction_, $2, $4);
    }
    |  '(' term '+' term ')' {
        cout << "read binary term +" << endl;
        $$ = new_ast_term_node(term_operation_t::union_, $2, $4);
    }
    | '-' '(' term ')'{
        cout << "read unary term -" << endl;
        $$ = new_ast_term_node(term_operation_t::star_, $3, nullptr);
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
