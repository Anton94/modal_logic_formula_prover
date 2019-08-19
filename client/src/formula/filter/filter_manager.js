import { operations } from '../operations';

import { decompose_less } from './decompose_less';
import { decompose_max_two_childs } from './decompose_max_two_childs';
import { decompose_equivalency } from './decompose_equivalency';
import { decompose_implication } from './decompose_implication';
import { remove_double_negations } from './remove_double_negations';
import { decompose_contact_on_Tdis } from './decompose_contact_on_Tdis';
import { calculate_constants } from './calculate_constants';


export class filter_manager {
    constructor(formula, build_type) { // prod / dev
        this.formula = formula;
        this.build_type = build_type;
    }

    filter() {
        this._formula_traverse_top_to_bottom(this.formula.get_ast(), new Set([operations.to_explanation(operations.formula().LESS)]), decompose_less);
        this._formula_traverse_top_to_bottom(this.formula.get_ast(), operations.by_argument(), decompose_max_two_childs);
        this._formula_traverse_top_to_bottom(this.formula.get_ast(), new Set([operations.to_explanation(operations.formula().EQUIVALENCY)]), decompose_equivalency);
        this._formula_traverse_top_to_bottom(this.formula.get_ast(), new Set([operations.to_explanation(operations.formula().IMPLICATION)]), decompose_implication);
        this._formula_traverse_top_to_bottom(this.formula.get_ast(), operations.negation_operations(), remove_double_negations);
        this._formula_traverse_top_to_bottom(this.formula.get_ast(), new Set([operations.to_explanation(operations.formula().CONTACT)]), decompose_contact_on_Tdis);
        this._formula_traverse_top_to_bottom(this.formula.get_ast(), operations.all(), calculate_constants);
    }

    _formula_traverse_top_to_bottom(node, filter_names, func) {
        if (filter_names.has(node.name)) {
            func(node);
        }

        if (operations.by_argument(0).has(node.name)) {
            return;
        } else if (operations.by_argument(1).has(node.name)) {
            this._formula_traverse_top_to_bottom(node.value, filter_names, func);
        } else {
            node.value.forEach((x) => {
                this._formula_traverse_top_to_bottom(x, filter_names, func);
            });
        }
    }
} 
