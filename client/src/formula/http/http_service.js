var $ = require('jquery/dist/jquery')

export class http_service {
    constructor() {
        this.x = 10;
    }

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