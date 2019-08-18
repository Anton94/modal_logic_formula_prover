import { operations } from '../operations';


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

export function calculate_constants(node) {
    function op(operation) {
        var functions = {
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