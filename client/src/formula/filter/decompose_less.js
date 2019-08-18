import { operations } from '../operations';


export function decompose_less(node) {
    if (node.value[0].name == operations.to_explanation("+")) {
        node.name = operations.to_explanation("&");
        node.value = [
            {
                "name": operations.to_explanation("<="),
                "value": [
                    node.value[0].value[0],
                    node.value[1]
                ]
            },
            {
                "name": operations.to_explanation("<="),
                "value": [
                    node.value[0].value[1],
                    node.value[1]
                ]
            }
        ];
        decompose_less(node.value[0]);
        decompose_less(node.value[1]);
    } else {
        node.name = operations.to_explanation("=0");
        node.value = {
            "name": operations.to_explanation("*"),
            "value": [
                node.value[0],
                {
                    "name": operations.to_explanation("-"),
                    "value": node.value[1]
                }
            ]
        }
    }
}
