%define api.pure full
%locations
%param { yyscan_t scanner }

%code top {
    #include <cstdio>
    #include <memory>
    #include <functional>
    #include <unordered_set>

    #include "../ast.h"
}

%code requires {
  typedef void* yyscan_t;
}

%code {
    int yylex(YYSTYPE* yylvalp, YYLTYPE* yyllocp, yyscan_t scanner);
    void yyerror(YYLTYPE* yyllocp, yyscan_t unused, const char* msg);

    extern thread_local std::unique_ptr<NFormula> parsed_formula;
    extern thread_local std::function<void(int/*line*/, int/*column*/, const char*/*msg*/)> on_error;

    NFormula* create_formula_node(formula_operation_t op, Node* left = nullptr, Node* right = nullptr);
    NTerm* create_term_node(term_operation_t op, NTerm* left = nullptr, NTerm* right = nullptr);
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
        $$ = create_formula_node(formula_operation_t::constant_true);
    }
    | 'F' {
        $$ = create_formula_node(formula_operation_t::constant_false);
    }
    | 'C' '(' term ',' term ')' {
        $$ = create_formula_node(formula_operation_t::contact, $3, $5);
    }
    | "<=" '(' term ',' term ')' {
        $$ = create_formula_node(formula_operation_t::less_eq, $3, $5);
    }
    | "<=m" '(' term ',' term ')' {
        $$ = create_formula_node(formula_operation_t::measured_less_eq, $3, $5);
    }
    | term "=0" {
        $$ = create_formula_node(formula_operation_t::eq_zero, $1);
    }
    | '(' formula '&' formula ')' {
        $$ = create_formula_node(formula_operation_t::conjunction, $2, $4);
    }
    | formula '&' formula {
        $$ = create_formula_node(formula_operation_t::conjunction, $1, $3);
    }
    | '(' formula '|' formula ')' {
        $$ = create_formula_node(formula_operation_t::disjunction, $2, $4);
    }
    | formula '|' formula {
        $$ = create_formula_node(formula_operation_t::disjunction, $1, $3);
    }
    | '~' formula {
        $$ = create_formula_node(formula_operation_t::negation, $2);
    }
    | '(' formula "->" formula ')' {
        $$ = create_formula_node(formula_operation_t::implication, $2, $4);
    }
    | formula "->" formula {
        $$ = create_formula_node(formula_operation_t::implication, $1, $3);
    }
    | '(' formula "<->" formula ')' {
        $$ = create_formula_node(formula_operation_t::equality, $2, $4);
    }
    | formula "<->" formula {
        $$ = create_formula_node(formula_operation_t::equality, $1, $3);
    }
    | '(' formula ')' {
        $$ = $2;
    }
  ;
term
    : '1' {
        $$ = create_term_node(term_operation_t::constant_true);
    }
    | '0' {
        $$ = create_term_node(term_operation_t::constant_false);
    }
    | "string" {
        $$ = create_term_node(term_operation_t::variable);
        $$->variable = $1;
        free((void*)$1);
    }
    | '(' term '*' term ')' {
        $$ = create_term_node(term_operation_t::intersection, $2, $4);
    }
    | term '*' term {
        $$ = create_term_node(term_operation_t::intersection, $1, $3);
    }
    | '(' term '+' term ')' {
        $$ = create_term_node(term_operation_t::union_, $2, $4);
    }
    | term '+' term {
        $$ = create_term_node(term_operation_t::union_, $1, $3);
    }
    | '-' term {
        $$ = create_term_node(term_operation_t::complement, $2);
    }
    | '(' term ')' {
        $$ = $2;
    }
  ;
%%


/// A bit ugly memory managment, but for not it will do the trick...
/// If the parsing fails we need to garbage collect all dangling nodes
static thread_local std::unordered_set<NTerm*> created_ast_term_nodes;
static thread_local std::unordered_set<NFormula*> created_ast_formula_nodes;

NFormula* create_formula_node(formula_operation_t op, Node* left, Node* right)
{
    auto n = new NFormula(op, left, right);
    created_ast_formula_nodes.insert(n);
    return n;
}

NTerm* create_term_node(term_operation_t op, NTerm* left, NTerm* right)
{
    auto n = new NTerm(op, left, right);
    created_ast_term_nodes.insert(n);
    return n;
}

void on_before_parsing()
{
    created_ast_term_nodes.clear();
    created_ast_formula_nodes.clear();
    parsed_formula.reset();
}

void free_all_nodes()
{
    for(const auto& n : created_ast_term_nodes)
    {
        n->left = n->right = nullptr;
        delete n;
    }
    for(const auto& n : created_ast_formula_nodes)
    {
        n->left = n->right = nullptr;
        delete n;
    }
    created_ast_term_nodes.clear();
    created_ast_formula_nodes.clear();
    parsed_formula.release();
}

void yyerror(YYLTYPE* yyllocp, yyscan_t, const char* msg)
{
    if(on_error)
    {
        on_error(yyllocp->first_line, yyllocp->first_column, msg);
        free_all_nodes();
    }
}
