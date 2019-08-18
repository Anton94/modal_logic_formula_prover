import { operations } from '../operations';

export function decompose_contact_on_Tdis(node) {
    if (node.value[0].name == operations.to_explanation("+")) {
        node.name = operations.to_explanation("|");
        node.value = [
            {
                "name": operations.to_explanation("C"),
                "value": [
                    node.value[0].value[0],
                    node.value[1]
                ]
            },
            {
                "name": operations.to_explanation("C"),
                "value": [
                    node.value[0].value[1],
                    node.value[1]
                ]
            }
        ];
        decompose_contact_on_Tdis(node.value[0]);
        decompose_contact_on_Tdis(node.value[1]);
    } else if (node.value[1].name == operations.to_explanation("+")) {
        node.name = operations.to_explanation("|");
        node.value = [
            {
                "name": operations.to_explanation("C"),
                "value": [
                    node.value[0],
                    node.value[1].value[0]
                ]
            },
            {
                "name": operations.to_explanation("C"),
                "value": [
                    node.value[0],
                    node.value[1].value[1]
                ]
            }
        ];
        decompose_contact_on_Tdis(node.value[0]);
        decompose_contact_on_Tdis(node.value[1]);
    }
}