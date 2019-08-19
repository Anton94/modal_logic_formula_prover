// TODO

var $ = require('jquery/dist/jquery')

import { filter_manager } from './formula/filter/filter_manager';
import { parse } from './formula/parser/parser';

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

export function formula_to_json(formula) {
    var parsed = parse(formula);
    if ((parsed.lexErrors && parsed.lexErrors.length != 0) || 
        (parsed.parseErrors && parsed.parseErrors.length != 0)) {
        console.log("Lex errors: " + parsed.lexErrors);
        console.log("Parse errors: " + parsed.parseErrors);
        parsed["hasErrors"] = true;
        return parsed;
    }

    var filter_mgr = new filter_manager(parsed.parsed_formula, "dev");
    filter_mgr.filter();

    parsed["hasErrors"] = false;
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
