import { operations } from '../operations';

export function decompose_equivalency(node) {
    node.name = operations.to_explanation("|");
    var left_child = {
        "name": operations.to_explanation("&"),
        "value": [node.value[0], node.value[1]]
    }
    var right_child = {
        "name": operations.to_explanation("&"),
        "value": [
            {
                "name": operations.to_explanation("~"),
                "value": node.value[0]
            }, 
            {
                "name": operations.to_explanation("~"),
                "value": node.value[1]
            }
        ]
    }
    node.value = [left_child, right_child];
}