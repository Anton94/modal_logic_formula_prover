var chevrotain = require('chevrotain/lib/chevrotain');

import { operations } from '../operations.js';
import { formula } from '../formula';

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

// ----------------- wrapping it all together -----------------

// reuse the same parser instance.
const parser = new JsonParser()

var jsonToken = allTokens;
var res_par = JsonParser;

export function parse(formula_string) {
    const lexResult = JsonLexer.tokenize(formula_string)
    var cst = null;
    var result = {};
    if (lexResult.errors.length == 0) {
        // setting a new input will RESET the parser instance's state.
        parser.input = lexResult.tokens
        // any top level rule may be used as an entry point
        cst = parser.disjunction();   
        
        if (parser.errors == 0) {
            result["parsed_formula"] = new formula(cst);
        }
    }
    result["cst"] = cst;
    result["lexErrors"] = lexResult.errors;
    result["parseErrors"] = parser.errors;
    return result;
}
