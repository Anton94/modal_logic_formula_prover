import { operations } from './operations.js';

export class formula {
    constructor(cst) {
        this.ast_formula = this._from_cst(cst);
        this.cst_formula = cst;
    }

    get_ast() {
        return this.ast_formula;
    }

    ast_to_string() {
        return JSON.stringify(this.ast_formula, undefined, 3);
    }

    cst_to_string() {
        return JSON.stringify(this.cst_formula, undefined, 3);
    }

    beautify() {
        return this._print(this.ast_formula);
    }

    _from_cst(cst) {
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
                "value": this._from_cst(cst.children["lhs"][0])
            };
            for (var i = 1; i < cst.children[operation].length; ++i) {
                value = {
                    "name": operations.to_explanation(operation),
                    "value": value
                }
            }
            return value;
        } else if (cst.children.hasOwnProperty("rhs")) {
            var children = [this._from_cst(cst.children["lhs"][0])];
            cst.children["rhs"].forEach((x) => {
                children.push(this._from_cst(x));
            });
            return {
                "name": operations.to_explanation(cst.children["mid"][0].image),
                "value": children
            };
        } else if (cst.children.hasOwnProperty("mid")) {
            return {
                "name": operations.to_explanation(cst.children["mid"][0].image),
                "value": this._from_cst(cst.children["lhs"][0])
            }
        } else if (cst.children.hasOwnProperty("lhs")) {
            return this._from_cst(cst.children["lhs"][0]);
        }
        throw Error("This is not permitted");
    }

    _print(obj) {
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
}