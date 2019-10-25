%define api.pure full
%locations
%lex-param {yyscan_t scanner}
%parse-param {yyscan_t scanner}

%code top {
    #include <cstdio>
    #include <memory>
    #include <iostream> //todo: get rid of and redirect the error output somewhere...

    #include "../ast.h"
}

%code requires {
  typedef void* yyscan_t;
}

%code {
    int yylex(YYSTYPE* yylvalp, YYLTYPE* yyllocp, yyscan_t scanner);
    void yyerror(YYLTYPE* yyllocp, yyscan_t unused, const char* msg);

    extern thread_local std::unique_ptr<NFormula> parsed_formula;
}

%define api.value.type union
%token <const char*> T_STRING   "string"
%token T_LESS_EQ                "<="
%token T_MEASURED_LESS_EQ       "<=m"
%token T_EQ_ZERO                "=0"
%token T_FORMULA_OP_IMPLICATION "->"
%token T_FORMULA_OP_EQUALITY    "<->"

// the operators precedence is from low to hight (w.r.t. the order (by line) in which they are defined)
%left T_FORMULA_OP_IMPLICATION T_FORMULA_OP_EQUALITY
%left '|' '+'
%left '&' '*'
%right '~' '-'
%nonassoc '(' ')'

%type <NFormula*> formula
%type <NTerm*> term

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
    | "<=" '(' term ',' term ')' {
        $$ = new NFormula(formula_operation_t::less_eq, $3, $5);
    }
    | "<=m" '(' term ',' term ')' {
        $$ = new NFormula(formula_operation_t::measured_less_eq, $3, $5);
    }
    | term "=0" {
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
    | '(' formula "->" formula ')' {
        $$ = new NFormula(formula_operation_t::implication, $2, $4);
    }
    | formula "->" formula {
        $$ = new NFormula(formula_operation_t::implication, $1, $3);
    }
    | '(' formula "<->" formula ')' {
        $$ = new NFormula(formula_operation_t::equality, $2, $4);
    }
    | formula "<->" formula {
        $$ = new NFormula(formula_operation_t::equality, $1, $3);
    }
    | '(' formula ')' {
        $$ = $2;
    }
  ;
term
    : '1' {
        $$ = new NTerm(term_operation_t::constant_true);
    }
    | '0' {
        $$ = new NTerm(term_operation_t::constant_false);
    }
    | "string" {
        $$ = new NTerm(term_operation_t::variable);
        $$->variable = $1;
        free((void*)$1);
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
    | '(' term ')' {
        $$ = $2;
    }
  ;
%%

void yyerror(YYLTYPE* yyllocp, yyscan_t unused, const char* msg)
{
    std::cerr << "[" << yyllocp->first_line << ":" << yyllocp->first_column << "]: " << msg << std::endl;
}
