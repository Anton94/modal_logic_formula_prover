// TODO

const createToken = chevrotain.createToken;
const Lexer = chevrotain.Lexer;
const CstParser = chevrotain.CstParser;

// ----------------- lexer -----------------

const Sub = createToken({ name: "Sub", pattern: /<=/ })
const Contact = createToken({ name: "Contact", pattern: /C/ })

const Dis = createToken({ name: "Dis", pattern: /\|/ })
const Con = createToken({ name: "Con", pattern: /&/ })
const Imp = createToken({ name: "Imp", pattern: /=>/ })
const Equ = createToken({ name: "Equ", pattern: /<->/ })
const Neg = createToken({ name: "Neg", pattern: /~/ })

// Term operations
const TCon = createToken({ name: "TCon", pattern: /@/ })
const TDis = createToken({ name: "TDis", pattern: /#/ })
const TStar = createToken({ name: "TStar", pattern: /\*/ })

const True = createToken({ name: "True", pattern: "T" })
const False = createToken({ name: "False", pattern: "F" })
const StringLiteral = createToken({
    name: "StringLiteral",
    pattern: /[a-z0-9]+/
})
const LParen = createToken({ name: "LParen", pattern: /\(/ })
const RParen = createToken({ name: "RParen", pattern: /\)/ })
const Comma = createToken({ name: "Comma", pattern: /,/ })
const WhiteSpace = createToken({
    name: "WhiteSpace",
    pattern: /\s+/,
    group: Lexer.SKIPPED
});

const allTokens = [
    WhiteSpace,
    Comma,

// atomic formula
    Sub,
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
                            { ALT: () => $.CONSUME(Sub, { LABEL: "mid" }) },
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

// ----------------- wrapping it all together -----------------

// reuse the same parser instance.
const parser = new JsonParser()

var jsonToken = allTokens;
var res_par = JsonParser;

function appenda(text, width, skip_new_line) {
    var output = document.getElementById("output");
    output.innerHTML = output.innerHTML + "&emsp;".repeat(width) + text + "<br/>";
}


const symbol_to_explanation =  {
    "<=": "less",
    "C": "contact",
    "|": "disjunction",
    "&": "conjunction",
    "=>": "implication",
    "<->": "equivalence",
    "~": "negation",
    "@": "Tconjunction",
    "#": "Tdisjunction",
    "*": "star",
    "T": "true",
    "F": "false"
}

function simplify(cst) {
    if (!cst.hasOwnProperty("name")) {
        return { 
            "name": "string", 
            "value":symbol_to_explanation[cst.image]
        };
    } else if (cst.children.hasOwnProperty("star")) {
        var starValue = {
            "name": symbol_to_explanation["*"],
            "value": simplify(cst.children["lhs"][0])
        };
        for (var i = 1; i <cst.children["star"].length; ++i) {
            starValue = {
                "name": symbol_to_explanation["*"],
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

function parse(text) {
    const lexResult = JsonLexer.tokenize(text)
    // setting a new input will RESET the parser instance's state.
    parser.input = lexResult.tokens
    // any top level rule may be used as an entry point
    const cst = parser.disjunction()
    document.getElementById("output").innerHTML = "";

    return {
        cst: cst,
        lexErrors: lexResult.errors,
        parseErrors: parser.errors
    }
}



// (<=((a#b)@c, (b#m@c))|C(a,b))&C(b,m)