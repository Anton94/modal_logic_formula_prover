import { operations } from '../operations';

export function remove_double_negations(node) {
    if (node.value.name != null && operations.negation_operations().has(node.value.name)) {
        node.name = node.value.value.name;
        node.value = node.value.value.value;
    }
}
