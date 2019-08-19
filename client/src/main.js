import { filter_manager } from './formula/filter/filter_manager';
import { parse } from './formula/parser/parser';
import { http_service } from './formula/http/http_service';
import { HomeIcon } from './img/home-icon.png';
import { AndIcon } from './img/and.png';
import { BotIcon } from './img/bot.png';
import { NotIcon } from './img/not.png';
import { OrIcon } from './img/or.png';
import { TopIcon } from './img/top.png';


var service = new http_service();

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

    parsed["build_formula"] = service.build_formula(JSON.stringify(parsed_formula));
    return parsed;
}

export function is_satisfied(formula) {
    var parsed = formula_to_json(formula);
    if (parsed.hasErrors) {
        return parsed;
    }
    
    parsed["is_satisfied"] = service.is_satisfied(parsed.parsed_formula.ast_to_string(3));
    return parsed;
}

function is_satisfied_certificate_check(formula) {
    var parsed = formula_to_json(formula);
    if (parsed.hasErrors) {
        return parsed;
    }

    parsed["is_satisfied"] = service.is_satisfied_certificate_check(parsed.parsed_formula.ast_to_string());
    return parsed;
}

function is_satisfied_check_all(formula) {
    var parsed = formula_to_json(formula);
    if (parsed.hasErrors) {
        return parsed;
    }

    parsed["is_satisfied"] = service.is_satisfied_certificate_check(parsed.parsed_formula.ast_to_string(), "bruteforce=true");
    return parsed;
}

//if (typeof module !== 'undefined' && typeof module.exports !== 'undefined') {
//    module.exports = {formula_to_json, is_satisfied, foo};
//}

// bug ! is front does not do nything
