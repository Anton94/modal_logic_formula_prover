export function decompose_max_two_childs(node) {
    if (node.value.length < 3) {
        return;
    }
    var max_length = ( node.value.length % 2 ) ? node.value.length - 1 : node.value.length;
    var children = [];
    for (var i = 0; i < max_length; i += 2) {
        children.push({
            "name": node.name,
            "value": [node.value[i], node.value[i + 1]]
        });
    }
    if (max_length != node.value.length) {
        children.push(node.value[max_length]);
    }
    node.value = children;
    decompose_max_two_childs(node);
}