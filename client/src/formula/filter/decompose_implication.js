import { operations } from '../operations';

export function decompose_implication(node) {
    node.name = operations.to_explanation("|");
    var left_child = {
        "name": operations.to_explanation("~"),
        "value": node.value[0]
    }
    node.value = [left_child, node.value[1]]
}