const createToken = chevrotain.createToken;
const Lexer = chevrotain.Lexer;
const CstParser = chevrotain.CstParser;

// ----------------- lexer -----------------
const BinOperator = createToken({name : "BinOperator", pattern: Lexer.NA})
const Dis = createToken({ name: "Dis", pattern: /\|/, categories: BinOperator })
const Con = createToken({ name: "Con", pattern: /&/, categories: BinOperator })
const Neg = createToken({ name: "Neg", pattern: /!/ })
const StringLiteral = createToken({
    name: "StringLiteral",
    pattern: /\w+/
})
const LParen = createToken({ name: "LParen", pattern: /\(/ })
const RParen = createToken({ name: "RParen", pattern: /\)/ })
const WhiteSpace = createToken({
    name: "WhiteSpace",
    pattern: /\s+/,
    group: Lexer.SKIPPED
});

const allTokens = [
    WhiteSpace,
    Dis,
    Con,
    Neg,
    LParen,
    RParen,
    StringLiteral,
    BinOperator
]

const JsonLexer = new Lexer(allTokens)

// ----------------- parser -----------------

class JsonParser extends CstParser {
    constructor() {
        super(allTokens)

        const $ = this

        $.RULE("disjunction", () => {
            $.SUBRULE($.conjunction, { LABEL : "lhs" })
            $.MANY(() => {
                $.CONSUME(Dis)
                $.SUBRULE2($.conjunction, { LABEL : "rhs" })  
            })
        })

        $.RULE("conjunction", () => {
            $.SUBRULE($.atomic, { LABEL : "lhs" })
            $.MANY(() => {
                $.CONSUME(Con)
                $.SUBRULE2($.atomic, { LABEL : "rhs" })
            })
        })

        $.RULE("atomic", () => {
            $.OPTION(() => {
                $.CONSUME(Neg)
            })
            $.OR([
                { ALT: () => $.SUBRULE($.parenthesis) },
                { ALT: () => $.CONSUME(StringLiteral) }
            ])
        })

        $.RULE("parenthesis", () => {
            $.CONSUME(LParen)
            $.SUBRULE($.disjunction, { LABEL : "lhs" })
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


function printa(cst, width) {
    if (cst.children.hasOwnProperty("Neg")) {
        appenda("!", width, true);

        if (cst.children.hasOwnProperty("parenthesis")) {
            printa(cst.children["parenthesis"][0], width);
        } else if (cst.children.hasOwnProperty("StringLiteral")) {
            appenda(cst.children["StringLiteral"].image, width);
        } else {
            throw "Negated something whicis not ( ) or variable";
        }
    } else if (cst.children.hasOwnProperty("Dis") || cst.children.hasOwnProperty("Con")) {
        printa(cst.children["lhs"][0], width + 1);
        appenda(cst.children.hasOwnProperty("Dis") ? "|" : "&", width);
        printa(cst.children["rhs"][0], width + 1);
    } else if (cst.children.hasOwnProperty("parenthesis")) {
        printa(cst.children["parenthesis"][0], width);
    } else if (cst.children.hasOwnProperty("LParen")) {
        appenda("(", width);
        printa(cst.children["lhs"][0], width + 1);
        appenda(")", width);
    } else if (cst.children.hasOwnProperty("StringLiteral")) {
        appenda(cst.children["StringLiteral"][0].image, width);
    } else if (cst.children.hasOwnProperty("lhs")){
        printa(cst.children["lhs"][0], width)
    } else if (Object.keys(cst.children).length > 0) {
        throw "Not good  " + JSON.stringify(cst.children);
    }
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