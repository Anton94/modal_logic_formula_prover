const symbol_to_explanation = {
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

export class operations {
    static formula() {
        return {
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
        }
    }

    static term() {
        return {
            UNION: "+",
            INTERSECTION: "*",
            STAR: "-",
            constant: {
                TRUE: "1",
                FALSE: "0"
            }
        }
    }

    static help() {
        return {
            STRING: "string",
            LPAREN: "(",
            RPAREN: ")",
            COMMA: ","
        };
    }

    static by_argument(arguments_count) {
        if (arguments_count === 0) {
            return new Set([
                symbol_to_explanation[operations.formula().constant.TRUE],
                symbol_to_explanation[operations.formula().constant.FALSE],
                symbol_to_explanation[operations.term().constant.TRUE],
                symbol_to_explanation[operations.term().constant.FALSE],
                symbol_to_explanation[operations.help().STRING]
            ]);
        }
        if (arguments_count === 1) {
            return new Set([
                symbol_to_explanation[operations.formula().NEGATION],
                symbol_to_explanation[operations.term().STAR],
                symbol_to_explanation[operations.formula().EQUAL_ZERO]
            ]);
        } else if (arguments_count === 2) {
            return new Set([
                symbol_to_explanation[operations.formula().LESS],
                symbol_to_explanation[operations.formula().CONTACT]
            ]);
        } else {
            return new Set([
                symbol_to_explanation[operations.formula().CONJUNCTION],
                symbol_to_explanation[operations.formula().DISJUNCTION],
                symbol_to_explanation[operations.formula().IMPLICATION],
                symbol_to_explanation[operations.formula().EQUIVALENCY],
                symbol_to_explanation[operations.term().UNION],
                symbol_to_explanation[operations.term().INTERSECTION]
            ])
        }
    }

    static negation_operations() {
        return new Set([
            symbol_to_explanation[operations.formula().NEGATION],
            symbol_to_explanation[operations.term().STAR]
        ]);
    }

    static constant_operations() {
        return new Set([
            symbol_to_explanation[operations.formula().constant.TRUE],
            symbol_to_explanation[operations.formula().constant.FALSE],
            symbol_to_explanation[operations.term().constant.TRUE],
            symbol_to_explanation[operations.term().constant.FALSE]
        ])
    }

    static all() {
        return new Set([
            symbol_to_explanation[operations.formula().NEGATION],
            symbol_to_explanation[operations.formula().EQUAL_ZERO],
            symbol_to_explanation[operations.formula().LESS],
            symbol_to_explanation[operations.formula().CONTACT],
            symbol_to_explanation[operations.formula().CONJUNCTION],
            symbol_to_explanation[operations.formula().DISJUNCTION],
            symbol_to_explanation[operations.formula().IMPLICATION],
            symbol_to_explanation[operations.formula().EQUIVALENCY],
            symbol_to_explanation[operations.formula().constant.TRUE],
            symbol_to_explanation[operations.formula().constant.FALSE],
            symbol_to_explanation[operations.term().STAR],
            symbol_to_explanation[operations.term().UNION],
            symbol_to_explanation[operations.term().INTERSECTION],
            symbol_to_explanation[operations.term().constant.TRUE],
            symbol_to_explanation[operations.term().constant.FALSE],
            symbol_to_explanation[operations.help().STRING]
        ]);
    }

    static to_explanation(symbol) {
        return symbol_to_explanation[symbol];
    }
}



