// TODO

var chevrotain = require('chevrotain/lib/chevrotain');
var $ = require('jquery/dist/jquery')

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
        constant: {
            TRUE: "T",
            FALSE: "F"
        }
    },
    term: {
        UNION: "+",
        INTERSECTION: "*",
        STAR: "-",
        constant: {
            TRUE: "1",
            FALSE: "0"
        }
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
    "T": "T",
    "F": "F",
    "1": "1",
    "0": "0",
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

const CONSTANT_OPERATIONS = new Set([
    symbol_to_explanation[Operations.formula.constant.TRUE],
    symbol_to_explanation[Operations.formula.constant.FALSE],
    symbol_to_explanation[Operations.term.constant.TRUE],
    symbol_to_explanation[Operations.term.constant.FALSE]
])

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
    symbol_to_explanation[Operations.formula.constant.TRUE],
    symbol_to_explanation[Operations.formula.constant.FALSE],
    symbol_to_explanation[Operations.term.constant.TRUE],
    symbol_to_explanation[Operations.term.constant.FALSE],
    symbol_to_explanation[Operations.help.STRING]
])

const ALL_OPERATIONS = new Set([
    symbol_to_explanation[Operations.formula.NEGATION],
    symbol_to_explanation[Operations.formula.EQUAL_ZERO],
    symbol_to_explanation[Operations.formula.LESS],
    symbol_to_explanation[Operations.formula.CONTACT],
    symbol_to_explanation[Operations.formula.CONJUNCTION],
    symbol_to_explanation[Operations.formula.DISJUNCTION],
    symbol_to_explanation[Operations.formula.IMPLICATION],
    symbol_to_explanation[Operations.formula.EQUIVALENCY],
    symbol_to_explanation[Operations.formula.constant.TRUE],
    symbol_to_explanation[Operations.formula.constant.FALSE],
    symbol_to_explanation[Operations.term.STAR],
    symbol_to_explanation[Operations.term.UNION],
    symbol_to_explanation[Operations.term.INTERSECTION],
    symbol_to_explanation[Operations.term.constant.TRUE],
    symbol_to_explanation[Operations.term.constant.FALSE],
    symbol_to_explanation[Operations.help.STRING]
])


const createToken = chevrotain.createToken;
const Lexer = chevrotain.Lexer;
const CstParser = chevrotain.CstParser;

// ----------------- lexer -----------------

const Less = createToken({ name: "Less", pattern: Operations.formula.LESS })
const EqualZero = createToken({ name: "EqualZero", pattern: Operations.formula.EQUAL_ZERO })
const Contact = createToken({ name: "Contact", pattern: Operations.formula.CONTACT })

const Dis = createToken({ name: "Dis", pattern: Operations.formula.DISJUNCTION })
const Con = createToken({ name: "Con", pattern: Operations.formula.CONJUNCTION })
const Imp = createToken({ name: "Imp", pattern: Operations.formula.IMPLICATION })
const Equ = createToken({ name: "Equ", pattern: Operations.formula.EQUIVALENCY })
const Neg = createToken({ name: "Neg", pattern: Operations.formula.NEGATION })

const FormulaTrue = createToken({ name: "FormulaTrue", pattern: Operations.formula.constant.TRUE })
const FormulaFalse = createToken({ name: "FormulaFalse", pattern: Operations.formula.constant.FALSE })

// Term operations
const TCon = createToken({ name: "TCon", pattern: Operations.term.INTERSECTION })
const TDis = createToken({ name: "TDis", pattern: Operations.term.UNION })
const TStar = createToken({ name: "TStar", pattern: Operations.term.STAR })

const TermTrue = createToken({ name: "TermTrue", pattern: Operations.term.constant.TRUE })
const TermFalse = createToken({ name: "TermFalse", pattern: Operations.term.constant.FALSE })
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
    EqualZero,
    Contact,

// formula operation
    Dis,
    Con,
    Imp,
    Equ,
    Neg,

    FormulaTrue,
    FormulaFalse,

// term operation
    TCon,
    TDis,
    TStar,

    LParen,
    RParen,

// atomic term,
    TermTrue,
    TermFalse,
    StringLiteral
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
                $.CONSUME(Neg, { LABEL: Operations.formula.NEGATION })
            })
            $.SUBRULE($.atomic_formula, { LABEL: "lhs" })
        })

        $.RULE("atomic_formula", () => {
            $.OR([
                {
                    ALT: () => $.SUBRULE($.parenthesis_formula, { LABEL: "lhs" })
                },
                { ALT: () => $.CONSUME(FormulaTrue, { LABEL: "constant" }) },
                { ALT: () => $.CONSUME(FormulaFalse, { LABEL: "constant" }) },
                { ALT: () => {
                        $.CONSUME(EqualZero, { LABEL: "mid" })
                        $.SUBRULE1($.Tdisjunction, { LABEL: "lhs" })
                    }
                },
                { ALT: () => {
                        $.OR1([
                            { ALT: () => $.CONSUME(Less, { LABEL: "mid" }) },
                            { ALT: () => $.CONSUME(Contact, { LABEL: "mid" }) }
                        ])
                        $.CONSUME(LParen)
                        $.SUBRULE2($.Tdisjunction, { LABEL: "lhs" })
                        $.CONSUME(Comma)
                        $.SUBRULE3($.Tdisjunction, { LABEL: "rhs" })
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
                $.CONSUME(TStar, { LABEL: Operations.term.STAR })
            })
            $.SUBRULE($.atomic_term, { LABEL: "lhs" })
        })

        // atomic
        $.RULE("atomic_term", () => {
            $.OR([
                { ALT: () => $.SUBRULE($.parenthesis_term, { LABEL: "lhs" }) },
                { ALT: () => $.CONSUME(TermTrue, { LABEL: "constant" }) },
                { ALT: () => $.CONSUME(TermFalse, { LABEL: "constant" }) },
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
            "name": Operations.help.STRING, 
            "value": cst.image
        };
    }

    if (cst.children.hasOwnProperty("constant")) {
        return {
            "name": symbol_to_explanation[cst.children["constant"][0].image]
        };
    }

    if (cst.children.hasOwnProperty(Operations.term.STAR) ||
            cst.children.hasOwnProperty(Operations.formula.NEGATION)) {
        var operation = cst.children.hasOwnProperty(Operations.term.STAR) 
                            ? Operations.term.STAR 
                            : Operations.formula.NEGATION;
        var value = {
            "name": symbol_to_explanation[operation],
            "value": simplify(cst.children["lhs"][0])
        };
        for (var i = 1; i < cst.children[operation].length; ++i) {
            value = {
                "name": symbol_to_explanation[operation],
                "value": value
            }
        }
        return value;
    } else if (cst.children.hasOwnProperty("rhs")) {
        var children = [simplify(cst.children["lhs"][0])];
        cst.children["rhs"].forEach((x) => {
            children.push(simplify(x));
        });
        return {
            "name": symbol_to_explanation[cst.children["mid"][0].image],
            "value": children
        };
    } else if (cst.children.hasOwnProperty("mid")) {
        return {
            "name": symbol_to_explanation[cst.children["mid"][0].image],
            "value": simplify(cst.children["lhs"][0])
        }
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
    cst = null;
    if (lexResult.errors.length == 0) {
        // setting a new input will RESET the parser instance's state.
        parser.input = lexResult.tokens
        // any top level rule may be used as an entry point
        cst = parser.disjunction();    
    }

    return {
        cst: cst,
        lexErrors: lexResult.errors,
        parseErrors: parser.errors
    }
}

/// END OF PARSER



/// START OF HTTP SERVICE

class http_service {

    is_satisfied(formula) {
        var IP_ADDRESS = "http://localhost:34567/satisfy";
        return $.post( IP_ADDRESS, formula );
    }

    is_satisfied_certificate_check(formula, params) {
        var IP_ADDRESS = "http://localhost:34567/is_satisfied_certificate" 
        if (params) {
            IP_ADDRESS += "?" + params;
        }
        return $.post( IP_ADDRESS, formula );
    }

    build_formula(formula) {
        var IP_ADDRESS = "http://localhost:34567/build_formula";
        return $.post( IP_ADDRESS, formula );
    }
}



/// END OF HTTP SERVICE
function json_to_formula(obj) {
    if (obj.name === "0" || obj.name === "1" || obj.name === "F" || obj.name === "T") {
        return obj.name;
    }
    if (obj.name === "string") {
        return obj.value;
    }
    if (obj.name === "Tstar") {
        return "-" + json_to_formula(obj.value);
    }
    if (obj.name === "Tor") {
        return "(" + json_to_formula(obj.value[0]) + "+" + json_to_formula(obj.value[1]) + ")";
    }
    if (obj.name === "Tand") {
        return "(" + json_to_formula(obj.value[0]) + "*" + json_to_formula(obj.value[1]) + ")";
    }

    if (obj.name === "negation") {
        return "~" + json_to_formula(obj.value);
    }
    if (obj.name === "disjunction") {
        return "(" +json_to_formula(obj.value[0]) + "|" + json_to_formula(obj.value[1]) + ")";
    }
    if (obj.name === "conjunction") {
        return "(" + json_to_formula(obj.value[0]) + "&" + json_to_formula(obj.value[1]) + ")";
    }

    if (obj.name === "contact") {
        return "C(" + json_to_formula(obj.value[0]) + "," + json_to_formula(obj.value[1]) + ")";
    }
    if (obj.name === "equal0") {
        return json_to_formula(obj.value) + "=0";
    }
    throw "THERE ARE NO MORE POSSIBLE conversions.";
}

function formula_to_json(formula) {
    parsed = parse(formula);
    if ((parsed.lexErrors && parsed.lexErrors.length != 0) || 
        (parsed.parseErrors && parsed.parseErrors.length != 0)) {
        console.log("Lex errors: " + parsed.lexErrors);
        console.log("Parse errors: " + parsed.parseErrors);
        parsed["hasErrors"] = true;
        return parsed;
    }

    simplified = simplify(parsed.cst);
    //formula_traverse_top_to_bottom(simplified, new Set(["less"]), remove_equal_TDis_in_less);
    formula_traverse_top_to_bottom(simplified, new Set([symbol_to_explanation[Operations.formula.LESS]]), decompose_less);
    formula_traverse_top_to_bottom(simplified, N_ARG_OPERATIONS, decompose_max_two_childs);
    formula_traverse_top_to_bottom(simplified, new Set([symbol_to_explanation[Operations.formula.EQUIVALENCY]]), decompose_equivalency);
    formula_traverse_top_to_bottom(simplified, new Set([symbol_to_explanation[Operations.formula.IMPLICATION]]), decompose_implication);
    formula_traverse_top_to_bottom(simplified, NEG_OPERATIONS, remove_double_negations);
    formula_traverse_top_to_bottom(simplified, new Set([symbol_to_explanation[Operations.formula.CONTACT]]), decompose_contact_on_Tdis);
    formula_traverse_top_to_bottom(simplified, ALL_OPERATIONS, calculate_constants);

    parsed["hasErrors"] = false;
    parsed["parsed_formula"] = simplified;
    return parsed;
}

function build_formula(formula) {
    parsed = formula_to_json(formula);
    if (parsed.hasErrors) {
        return parsed;
    }
    service = new http_service();

    parsed["build_formula"] = service.build_formula(JSON.stringify(parsed_formula));
    return parsed;
}

function is_satisfied(formula) {
    parsed = formula_to_json(formula);
    if (parsed.hasErrors) {
        return parsed;
    }
    service = new http_service();

    parsed["is_satisfied"] = service.is_satisfied(JSON.stringify(parsed.parsed_formula));
    return parsed;
}

function is_satisfied_certificate_check(formula) {
    parsed = formula_to_json(formula);
    if (parsed.hasErrors) {
        return parsed;
    }
    service = new http_service();

    parsed["is_satisfied"] = service.is_satisfied_certificate_check(JSON.stringify(parsed.parsed_formula));
    return parsed;
}

function is_satisfied_check_all(formula) {
    parsed = formula_to_json(formula);
    if (parsed.hasErrors) {
        return parsed;
    }
    service = new http_service();

    parsed["is_satisfied"] = service.is_satisfied_certificate_check(JSON.stringify(parsed.parsed_formula), "bruteforce=true");
    return parsed;
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

function decompose_contact_on_Tdis(node) {
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
        decompose_contact_on_Tdis(node.value[0]);
        decompose_contact_on_Tdis(node.value[1]);
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
        decompose_contact_on_Tdis(node.value[0]);
        decompose_contact_on_Tdis(node.value[1]);
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
    if (max_length != node.value.length) {
        children.push(node.value[max_length]);
    }
    node.value = children;
    decompose_max_two_childs(node);
}

function decompose_implication(node) {
    node.name = symbol_to_explanation["|"];
    left_child = {
        "name": symbol_to_explanation["~"],
        "value": node.value[0]
    }
    node.value = [left_child, node.value[1]]
}

function decompose_equivalency(node) {
    node.name = symbol_to_explanation["|"];
    left_child = {
        "name": symbol_to_explanation["&"],
        "value": [node.value[0], node.value[1]]
    }
    right_child = {
        "name": symbol_to_explanation["&"],
        "value": [
            {
                "name": symbol_to_explanation["~"],
                "value": node.value[0]
            }, 
            {
                "name": symbol_to_explanation["~"],
                "value": node.value[1]
            }
        ]
    }
    node.value = [left_child, right_child];
}

function constant_to_bool(constant) {
    if (constant === "0")
        return false;
    if (constant === "1") 
        return true;
    if (constant === "F")
        return false;
    if (constant === "T")
        return true;
    throw "Constant can not be " + constant;
}

function bool_to_term(constant) {
    return (constant) ? "1" : "0";
}

function bool_to_formula(constant) {
    return (constant) ? "T" : "F";
}

function calculate_constants(node) {
    function op(operation) {
        functions = {
            "Tstar": function(x) {
                return bool_to_term(!constant_to_bool(x));
            },
            "Tor": function(x, y) {
                return bool_to_term(constant_to_bool(x) | constant_to_bool(y));
            },
            "Tand": function(x, y) {
                return bool_to_term(constant_to_bool(x) & constant_to_bool(y));
            },
            "Tstar": function(x) {
                return bool_to_formula(!constant_to_bool(x));
            },
            "disjunction": function(x, y) {
                return bool_to_term(constant_to_bool(x) | constant_to_bool(y));
            },
            "conjunction": function(x, y) {
                return bool_to_term(constant_to_bool(x) & constant_to_bool(y));
            },
            "contact": function(x, y) {
                return bool_to_term(constant_to_bool(x) & constant_to_bool(y));
            },
            "equal0": function(x) {
                return bool_to_term(constant_to_bool(x) === false);
            }
        }
        return functions[operation]
    }
    if (node.name === "0" || node.name === "1" || node.name === "F" || node.name === "T") {
        return true;
    }
    if (node.name === "string") {
        return false;
    }
    if (node.name === "Tstar" || node.name === "negation" || node.name === "equal0")  {
        var result = calculate_constants(node.value);
        if (result) {
            node.name = op(node.name)(node.value);
            delete node.value;
            return true;
        } else {
            return false;
        }
    }

    var left = calculate_constants(node.value[0]);
    var right = calculate_constants(node.value[1]);
    if (left && right) {
        node.name = op(node.name)(node.value[0].name, node.value[1].name)
        delete node.value;
        return true;
    }
    return false;
}

function foo() {
    console.log("Adasd");
}

if (typeof module !== 'undefined' && typeof module.exports !== 'undefined') {
    module.exports = {formula_to_json, is_satisfied, foo};
}

// (<=((a*b)+c, (b*m+c)) | C(a,b)) & C(b,m)
// <=(a,b)<->C(a,b)-><=(m,b)

// bug ! is front does not do nything
