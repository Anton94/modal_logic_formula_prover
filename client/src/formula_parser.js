// TODO
module.exports = {formula_to_json};

var Operations = {
    formula: {
        LESS: "<=",
        EQUAL_ZERO: "=0",
        CONTACT: "C",
        DISJUNCTION: "|",
        CONJUNCTION: "&",
        IMPLICATION: "->",
        EQUIVALENCY: "<->",
        NEGATION: "~",
    },
    term: {
        UNION: "+",
        INTERSECTION: "*",
        STAR: "-"
    },
    constant: {
        TRUE: "T",
        FALSE: "F"
    },
    help: {
        STRING: "string",
        LPAREN: "(",
        RPAREN: ")",
        COMMA: ","
    }
};

const symbol_to_explanation =  {
    "<=": "less",
    "=0": "equal0",
    "C": "contact",
    "|": "disjunction",
    "&": "conjunction",
    "->": "implication",
    "<->": "equivalence",
    "~": "negation",
    "*": "Tand",
    "+": "Tor",
    "-": "Tstar",
    "T": "true",
    "F": "false",
    "string": "string"
};

const ONE_ARG_OPERATIONS = new Set([
    symbol_to_explanation[Operations.formula.NEGATION],
    symbol_to_explanation[Operations.term.STAR],
    symbol_to_explanation[Operations.formula.EQUAL_ZERO]
]);

const NEG_OPERATIONS = new Set([
    symbol_to_explanation[Operations.formula.NEGATION],
    symbol_to_explanation[Operations.term.STAR]
]);

const TWO_ARG_OPERATIONS = new Set([
    symbol_to_explanation[Operations.formula.LESS],
    symbol_to_explanation[Operations.formula.CONTACT]
]);

const N_ARG_OPERATIONS = new Set([
    symbol_to_explanation[Operations.formula.CONJUNCTION],
    symbol_to_explanation[Operations.formula.DISJUNCTION],
    symbol_to_explanation[Operations.formula.IMPLICATION],
    symbol_to_explanation[Operations.formula.EQUIVALENCY],
    symbol_to_explanation[Operations.term.UNION],
    symbol_to_explanation[Operations.term.INTERSECTION]
])

const ZERO_ARG_OPERATIONS = new Set([
    symbol_to_explanation[Operations.constant.TRUE],
    symbol_to_explanation[Operations.constant.FALSE],
    symbol_to_explanation[Operations.help.STRING]
])



const createToken = chevrotain.createToken;
const Lexer = chevrotain.Lexer;
const CstParser = chevrotain.CstParser;

// ----------------- lexer -----------------

const Less = createToken({ name: "Less", pattern: Operations.formula.LESS })
const Contact = createToken({ name: "Contact", pattern: Operations.formula.CONTACT })

const Dis = createToken({ name: "Dis", pattern: Operations.formula.DISJUNCTION })
const Con = createToken({ name: "Con", pattern: Operations.formula.CONJUNCTION })
const Imp = createToken({ name: "Imp", pattern: Operations.formula.IMPLICATION })
const Equ = createToken({ name: "Equ", pattern: Operations.formula.EQUIVALENCY })
const Neg = createToken({ name: "Neg", pattern: Operations.formula.NEGATION })

// Term operations
const TCon = createToken({ name: "TCon", pattern: Operations.term.INTERSECTION })
const TDis = createToken({ name: "TDis", pattern: Operations.term.UNION })
const TStar = createToken({ name: "TStar", pattern: Operations.term.STAR })

const True = createToken({ name: "True", pattern: Operations.constant.TRUE })
const False = createToken({ name: "False", pattern: Operations.constant.FALSE })
const StringLiteral = createToken({
    name: "StringLiteral",
    pattern: /[a-z0-9]+/
})
const LParen = createToken({ name: "LParen", pattern: Operations.help.LPAREN })
const RParen = createToken({ name: "RParen", pattern: Operations.help.RPAREN })
const Comma = createToken({ name: "Comma", pattern: Operations.help.COMMA })
const WhiteSpace = createToken({
    name: "WhiteSpace",
    pattern: /\s+/,
    group: Lexer.SKIPPED
});

const allTokens = [
    WhiteSpace,
    Comma,

// atomic formula
    Less,
    Contact,

// formula operation
    Dis,
    Con,
    Imp,
    Equ,
    Neg,

// term operation
    TCon,
    TDis,
    TStar,

    LParen,
    RParen,

// atomic term
    StringLiteral,
    True,
    False
]

const JsonLexer = new Lexer(allTokens)

// ----------------- parser -----------------

class JsonParser extends CstParser {
    constructor() {
        super(allTokens)

        const $ = this

    // Formula Operations
        $.RULE("disjunction", () => {
            $.SUBRULE($.conjunction, { LABEL : "lhs" })
            $.MANY(() => {
                $.CONSUME(Dis, { LABEL : "mid" })
                $.SUBRULE2($.conjunction, { LABEL : "rhs" })  
            })
        })

        $.RULE("conjunction", () => {
            $.SUBRULE($.implication, { LABEL : "lhs" })
            $.MANY(() => {
                $.CONSUME(Con, { LABEL : "mid" })
                $.SUBRULE2($.implication, { LABEL : "rhs" })
            })
        })

        $.RULE("implication", () => {
            $.SUBRULE($.equivalence, { LABEL : "lhs" })
            $.MANY(() => {
                $.CONSUME(Imp, { LABEL : "mid" })
                $.SUBRULE2($.equivalence, { LABEL : "rhs" })
            })
        })

        $.RULE("equivalence", () => {
            $.SUBRULE($.negation, { LABEL : "lhs" })
            $.MANY(() => {
                $.CONSUME(Equ, { LABEL : "mid" })
                $.SUBRULE2($.negation, { LABEL : "rhs" })
            })
        })

        $.RULE("negation", () => {
            $.MANY(() => {
                $.CONSUME(Neg, { LABEL: "neg" })
            })
            $.SUBRULE($.atomic_formula, { LABEL: "lhs" })
        })

        $.RULE("atomic_formula", () => {
            $.OR([
                {
                    ALT: () => $.SUBRULE($.parenthesis_formula, { LABEL: "lhs" })
                },
                { ALT: () => {
                        $.OR1([
                            { ALT: () => $.CONSUME(Less, { LABEL: "mid" }) },
                            { ALT: () => $.CONSUME(Contact, { LABEL: "mid" }) }
                        ])
                        $.CONSUME(LParen)
                        $.SUBRULE($.Tdisjunction, { LABEL: "lhs" })
                        $.CONSUME(Comma)
                        $.SUBRULE2($.Tdisjunction, { LABEL: "rhs" })
                        $.CONSUME(RParen)
                    }
                }
            ])
        })

        $.RULE("parenthesis_formula", () => {
            $.CONSUME(LParen)
            $.SUBRULE($.disjunction, { LABEL : "lhs" })
            $.CONSUME(RParen)
        })

    // Term operations
        $.RULE("Tdisjunction", () => {
            $.SUBRULE($.Tconjunction, { LABEL: "lhs" })
            $.MANY(() => {
                $.CONSUME(TDis, { LABEL: "mid" })
                $.SUBRULE2($.Tconjunction, { LABEL: "rhs" })  
            })
        })

        $.RULE("Tconjunction", () => {
            $.SUBRULE($.star, { LABEL: "lhs" })
            $.MANY(() => {
                $.CONSUME(TCon, { LABEL: "mid" })
                $.SUBRULE2($.star, { LABEL: "rhs" })
            })
        })

        $.RULE("star", () => {
            $.MANY(() => {
                $.CONSUME(TStar, { LABEL: "star" })
            })
            $.SUBRULE($.atomic_term, { LABEL: "lhs" })
        })

        // atomic
        $.RULE("atomic_term", () => {
            $.OR([
                { ALT: () => $.SUBRULE($.parenthesis_term, { LABEL: "lhs" }) },
                { ALT: () => $.CONSUME(True, { LABEL: "lhs" }) },
                { ALT: () => $.CONSUME(False, { LABEL: "lhs" }) },
                { ALT: () => $.CONSUME(StringLiteral, { LABEL: "lhs" }) }
            ])
        })

        $.RULE("parenthesis_term", () => {
            $.CONSUME(LParen)
            $.SUBRULE($.Tdisjunction, { LABEL : "lhs" })
            $.CONSUME(RParen)
        })


        this.performSelfAnalysis()
    }
}

function simplify(cst) {
    if (!cst.hasOwnProperty("name")) {
        return { 
            "name": "string", 
            "value": cst.image
        };
    } else if (cst.children.hasOwnProperty("star")) {
        var starValue = {
            "name": symbol_to_explanation["-"],
            "value": simplify(cst.children["lhs"][0])
        };
        for (var i = 1; i <cst.children["star"].length; ++i) {
            starValue = {
                "name": symbol_to_explanation["-"],
                "value": starValue
            }
        }
        return starValue;
    } else if (cst.children.hasOwnProperty("neg")) {
        var negValue = {
            "name": symbol_to_explanation["~"], 
            "value": simplify(cst.children["lhs"][0])
        };
        for (var i = 1; i < cst.children["neg"].length; ++i) {
            negValue = {
                "name": symbol_to_explanation["~"],
                "value": negValue
            }
        }
        return negValue;
    } else if (cst.children.hasOwnProperty("rhs")) {
        var children = [simplify(cst.children["lhs"][0])];
        cst.children["rhs"].forEach((x) => {
            children.push(simplify(x));
        });
        return {
            "name": symbol_to_explanation[cst.children["mid"][0].image],
            "value": children
        };
    } else if (cst.children.hasOwnProperty("lhs")) {
        return simplify(cst.children["lhs"][0]);
    }
    throw Error("This is not permitted");
}

// ----------------- wrapping it all together -----------------

// reuse the same parser instance.
const parser = new JsonParser()

var jsonToken = allTokens;
var res_par = JsonParser;

function parse(formula) {
    const lexResult = JsonLexer.tokenize(formula)
    // setting a new input will RESET the parser instance's state.
    parser.input = lexResult.tokens
    // any top level rule may be used as an entry point
    const cst = parser.disjunction();

    console.log("Lex errors: " + lexResult.errors)
    console.log("Parse errors: " + parser.errors)

    return {
        cst: cst,
        lexErrors: lexResult.errors,
        parseErrors: parser.errors
    }
}

function formula_to_json(formula) {
    simplified = simplify(parse(formula).cst);
    //formula_traverse_top_to_bottom(simplified, new Set(["less"]), remove_equal_TDis_in_less);
    formula_traverse_top_to_bottom(simplified, new Set([symbol_to_explanation[Operations.formula.LESS]]), decompose_less);
    formula_traverse_top_to_bottom(simplified, N_ARG_OPERATIONS, decompose_max_two_childs);
    formula_traverse_top_to_bottom(simplified, new Set([symbol_to_explanation[Operations.formula.EQUIVALENCY]]), decompose_equivalency);
    formula_traverse_top_to_bottom(simplified, new Set([symbol_to_explanation[Operations.formula.IMPLICATION]]), decompose_implication);
    formula_traverse_top_to_bottom(simplified, NEG_OPERATIONS, remove_double_negations);
    formula_traverse_top_to_bottom(simplified, new Set([symbol_to_explanation[Operations.formula.CONTACT]]), decompose_contant_on_Tdis);
    return simplified;
}

// Run the func for all objects which name comes up in the filter_names.
function formula_traverse_top_to_bottom(node, filter_names, func) {
    if (filter_names.has(node.name)) {
        func(node);
    }

    if (ZERO_ARG_OPERATIONS.has(node.name)) {
        return;
    } else if (ONE_ARG_OPERATIONS.has(node.name)) {
        formula_traverse_top_to_bottom(node.value, filter_names, func);
    } else {
        node.value.forEach((x) => {
            formula_traverse_top_to_bottom(x, filter_names, func);
        });
    }
} 

function remove_equal_TDis_in_less(node) {
    // remove problems in <= if 
    // for example <=(a*b*c, a*-b)
    // the resulting node should contain only
    // <=(b*c, -b)
}

function decompose_contant_on_Tdis(node) {
    if (node.value[0].name == symbol_to_explanation["+"]) {
        node.name = symbol_to_explanation["|"];
        node.value = [
            {
                "name": symbol_to_explanation["C"],
                "value": [
                    node.value[0].value[0],
                    node.value[1]
                ]
            },
            {
                "name": symbol_to_explanation["C"],
                "value": [
                    node.value[0].value[1],
                    node.value[1]
                ]
            }
        ];
        decompose_contant_on_Tdis(node.value[0]);
        decompose_contant_on_Tdis(node.value[1]);
    } else if (node.value[1].name == symbol_to_explanation["+"]) {
        node.name = symbol_to_explanation["|"];
        node.value = [
            {
                "name": symbol_to_explanation["C"],
                "value": [
                    node.value[0],
                    node.value[1].value[0]
                ]
            },
            {
                "name": symbol_to_explanation["C"],
                "value": [
                    node.value[0],
                    node.value[1].value[1]
                ]
            }
        ];
        decompose_contant_on_Tdis(node.value[0]);
        decompose_contant_on_Tdis(node.value[1]);
    }
}

function decompose_less(node) {
    if (node.value[0].name == symbol_to_explanation["+"]) {
        node.name = symbol_to_explanation["&"];
        node.value = [
            {
                "name": symbol_to_explanation["<="],
                "value": [
                    node.value[0].value[0],
                    node.value[1]
                ]
            },
            {
                "name": symbol_to_explanation["<="],
                "value": [
                    node.value[0].value[1],
                    node.value[1]
                ]
            }
        ];
        decompose_less(node.value[0]);
        decompose_less(node.value[1]);
    } else {
        node.name = symbol_to_explanation["=0"];
        node.value = {
            "name": symbol_to_explanation["*"],
            "value": [
                node.value[0],
                {
                    "name": symbol_to_explanation["-"],
                    "value": node.value[1]
                }
            ]
        }
    }
}

function remove_double_negations(node) {
    if (node.value.name != null && NEG_OPERATIONS.has(node.value.name)) {
        node.name = node.value.value.name;
        node.value = node.value.value.value;
    }
}

function decompose_max_two_childs(node) {
    if (node.value.length < 3) {
        return;
    }
    max_length = ( node.value.length % 2 ) ? node.value.length - 1 : node.value.length;
    children = [];
    for (i = 0; i < max_length; i += 2) {
        children.push({
            "name": node.name,
            "value": [node.value[i], node.value[i + 1]]
        });
    }
    node.value = children;
    decompose_max_two_childs(node);
}

function decompose_implication(node) {
    node.name = symbol_to_explanation["+"];
    left_child = {
        "name": symbol_to_explanation["-"],
        "value": node.value[0]
    }
    node.value = [left_child, node.value[1]]
}

function decompose_equivalency(node) {
    node.name = symbol_to_explanation["+"];
    left_child = {
        "name": symbol_to_explanation["*"],
        "value": [node.value[0], node.value[1]]
    }
    right_child = {
        "name": symbol_to_explanation["*"],
        "value": [
            {
                "name": symbol_to_explanation["-"],
                "value": node.value[0]
            }, 
            {
                "name": symbol_to_explanation["-"],
                "value": node.value[1]
            }
        ]
    }
    node.value = [left_child, right_child];
}

// (<=((a*b)+c, (b*m+c)) | C(a,b)) & C(b,m)
// <=(a,b)<->C(a,b)-><=(m,b)

// bug ! is front does not do nything