// TODO

var chevrotain = require('chevrotain/lib/chevrotain');
var $ = require('jquery/dist/jquery')

import { filter_manager } from './formula/filter/filter_manager';
import { operations } from './formula/operations.js';


const createToken = chevrotain.createToken;
const Lexer = chevrotain.Lexer;
const CstParser = chevrotain.CstParser;

// ----------------- lexer -----------------

const Less = createToken({ name: "Less", pattern: operations.formula().LESS })
const EqualZero = createToken({ name: "EqualZero", pattern: operations.formula().EQUAL_ZERO })
const Contact = createToken({ name: "Contact", pattern: operations.formula().CONTACT })

const Dis = createToken({ name: "Dis", pattern: operations.formula().DISJUNCTION })
const Con = createToken({ name: "Con", pattern: operations.formula().CONJUNCTION })
const Imp = createToken({ name: "Imp", pattern: operations.formula().IMPLICATION })
const Equ = createToken({ name: "Equ", pattern: operations.formula().EQUIVALENCY })
const Neg = createToken({ name: "Neg", pattern: operations.formula().NEGATION })

const FormulaTrue = createToken({ name: "FormulaTrue", pattern: operations.formula().constant.TRUE })
const FormulaFalse = createToken({ name: "FormulaFalse", pattern: operations.formula().constant.FALSE })

// Term operations
const TCon = createToken({ name: "TCon", pattern: operations.term().INTERSECTION })
const TDis = createToken({ name: "TDis", pattern: operations.term().UNION })
const TStar = createToken({ name: "TStar", pattern: operations.term().STAR })

const TermTrue = createToken({ name: "TermTrue", pattern: operations.term().constant.TRUE })
const TermFalse = createToken({ name: "TermFalse", pattern: operations.term().constant.FALSE })
const StringLiteral = createToken({
    name: "StringLiteral",
    pattern: /[a-z0-9]+/
})
const LParen = createToken({ name: "LParen", pattern: operations.help().LPAREN })
const RParen = createToken({ name: "RParen", pattern: operations.help().RPAREN })
const Comma = createToken({ name: "Comma", pattern: operations.help().COMMA })
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
                $.CONSUME(Neg, { LABEL: operations.formula().NEGATION })
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
                $.CONSUME(TStar, { LABEL: operations.term().STAR })
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
            "name": operations.help().STRING, 
            "value": cst.image
        };
    }

    if (cst.children.hasOwnProperty("constant")) {
        return {
            "name": operations.to_explanation(cst.children["constant"][0].image)
        };
    }

    if (cst.children.hasOwnProperty(operations.term().STAR) ||
            cst.children.hasOwnProperty(operations.formula().NEGATION)) {
        var operation = cst.children.hasOwnProperty(operations.term().STAR) 
                            ? operations.term().STAR 
                            : operations.formula().NEGATION;
        var value = {
            "name": operations.to_explanation(operation),
            "value": simplify(cst.children["lhs"][0])
        };
        for (var i = 1; i < cst.children[operation].length; ++i) {
            value = {
                "name": operations.to_explanation(operation),
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
            "name": operations.to_explanation(cst.children["mid"][0].image),
            "value": children
        };
    } else if (cst.children.hasOwnProperty("mid")) {
        return {
            "name": operations.to_explanation(cst.children["mid"][0].image),
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
    var cst = null;
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

export function formula_to_json(formula) {
    var parsed = parse(formula);
    if ((parsed.lexErrors && parsed.lexErrors.length != 0) || 
        (parsed.parseErrors && parsed.parseErrors.length != 0)) {
        console.log("Lex errors: " + parsed.lexErrors);
        console.log("Parse errors: " + parsed.parseErrors);
        parsed["hasErrors"] = true;
        return parsed;
    }

    var simplified = simplify(parsed.cst);

    var filter_mgr = new filter_manager(simplified, "dev");
    filter_mgr.filter();

    parsed["hasErrors"] = false;
    parsed["parsed_formula"] = simplified;
    return parsed;
}

function build_formula(formula) {
    parsed = formula_to_json(formula);
    if (parsed.hasErrors) {
        return parsed;
    }
    var service = new http_service();

    parsed["build_formula"] = service.build_formula(JSON.stringify(parsed_formula));
    return parsed;
}

export function is_satisfied(formula) {
    var parsed = formula_to_json(formula);
    if (parsed.hasErrors) {
        return parsed;
    }
    var service = new http_service();

    parsed["is_satisfied"] = service.is_satisfied(JSON.stringify(parsed.parsed_formula));
    return parsed;
}

function is_satisfied_certificate_check(formula) {
    var parsed = formula_to_json(formula);
    if (parsed.hasErrors) {
        return parsed;
    }
    var service = new http_service();

    parsed["is_satisfied"] = service.is_satisfied_certificate_check(JSON.stringify(parsed.parsed_formula));
    return parsed;
}

function is_satisfied_check_all(formula) {
    var parsed = formula_to_json(formula);
    if (parsed.hasErrors) {
        return parsed;
    }
    var service = new http_service();

    parsed["is_satisfied"] = service.is_satisfied_certificate_check(JSON.stringify(parsed.parsed_formula), "bruteforce=true");
    return parsed;
}

function remove_equal_TDis_in_less(node) {
    // remove problems in <= if 
    // for example <=(a*b*c, a*-b)
    // the resulting node should contain only
    // <=(b*c, -b)
}


//if (typeof module !== 'undefined' && typeof module.exports !== 'undefined') {
//    module.exports = {formula_to_json, is_satisfied, foo};
//}

// (<=((a*b)+c, (b*m+c)) | C(a,b)) & C(b,m)
// <=(a,b)<->C(a,b)-><=(m,b)

// bug ! is front does not do nything
