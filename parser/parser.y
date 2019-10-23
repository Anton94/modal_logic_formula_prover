%{
#include <cstdio>
#include <iostream>

#include "parser_API.h"

using namespace std;

// stuff from flex that bison needs to know about:
extern int yylex();
extern int yyparse();

void yyerror(const char *s);

std::unique_ptr<NFormula> parsed_formula;
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
        parsed_formula.reset($1);
    }
  ;
formula
    : 'T' {
        $$ = new NFormula(formula_operation_t::constant_true);
    }
    | 'F' {
        $$ = new NFormula(formula_operation_t::constant_false);
    }
    | 'C' '(' term ',' term ')' {
        $$ = new NFormula(formula_operation_t::contact, $3, $5);
    }
    | T_LESS_EQ '(' term ',' term ')'  {
        $$ = new NFormula(formula_operation_t::less_eq, $3, $5);
    }
    | T_MEASURED_LESS_EQ '(' term ',' term ')'  {
        $$ = new NFormula(formula_operation_t::measured_less_eq, $3, $5);
    }
    | term T_EQ_ZERO {
        $$ = new NFormula(formula_operation_t::eq_zero, $1);
    }
    | '(' formula '&' formula ')' {
        $$ = new NFormula(formula_operation_t::conjunction, $2, $4);
    }
    | formula '&' formula {
        $$ = new NFormula(formula_operation_t::conjunction, $1, $3);
    }
    | '(' formula '|' formula ')' {
        $$ = new NFormula(formula_operation_t::disjunction, $2, $4);
    }
    | formula '|' formula {
        $$ = new NFormula(formula_operation_t::disjunction, $1, $3);
    }
    | '~' formula {
        $$ = new NFormula(formula_operation_t::negation, $2);
    }
    | '(' formula T_FORMULA_OP_IMPLICATION formula ')' {
        $$ = new NFormula(formula_operation_t::implication, $2, $4);
    }
    | formula T_FORMULA_OP_IMPLICATION formula {
        $$ = new NFormula(formula_operation_t::implication, $1, $3);
    }
    | '(' formula T_FORMULA_OP_EQUALITY formula ')' {
        $$ = new NFormula(formula_operation_t::equality, $2, $4);
    }
    | formula T_FORMULA_OP_EQUALITY formula {
        $$ = new NFormula(formula_operation_t::equality, $1, $3);
    }
  ;
term
    : '1' {
        $$ = new NTerm(term_operation_t::constant_true);
    }
    | '0' {
        $$ = new NTerm(term_operation_t::constant_false);
    }
    | T_STRING {
        $$ = new NTerm(term_operation_t::variable);
        $$->variable = $1;
        free($1);
    }
    | '(' term '*' term ')' {
        $$ = new NTerm(term_operation_t::intersaction, $2, $4);
    }
    | term '*' term {
        $$ = new NTerm(term_operation_t::intersaction, $1, $3);
    }
    | '(' term '+' term ')' {
        $$ = new NTerm(term_operation_t::union_, $2, $4);
    }
    | term '+' term {
        $$ = new NTerm(term_operation_t::union_, $1, $3);
    }
    | '-' term {
        $$ = new NTerm(term_operation_t::complement, $2);
    }
  ;
%%

void set_input_string(const char* in);
void end_lexical_scan();

std::unique_ptr<NFormula> parse_from_input_string(const char* in)
{
    parsed_formula.reset(nullptr);

    set_input_string(in);
    int rv = yyparse();
    end_lexical_scan();

    (void) rv;
    return std::unique_ptr<NFormula>(parsed_formula.release());
}

void yyerror(const char *s)
{
    cout << "EEK, parse error!  Message: " << s << endl;
    // might as well halt now:
    exit(-1);
}
