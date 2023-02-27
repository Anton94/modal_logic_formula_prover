window.api = {};


var js_wrapped_post = null;//Module.cwrap("post", "number", ["number", "number"]);
var js_wrapped_ping = null;//Module.cwrap("ping", "number", ["number", "number"]);
var js_wrapped_cancel = null;//Module.cwrap("cancel", "number", ["number", "number"]);

// API 
// post_task
// args: formula, algorithm_type, formula_filters, serverOrigin
// return type { 'op_id': string }
//
// cancel_task
// args: serverOrigin, op_id
// return type None
//
// ping_task
// args:
// return type 
ping_return_type = { 
    'is_satisfied': 'bool',
    'status': 'string', // FINISHED, CANCELED, RUNNING
    'output': 'string',
    'variables': ['string'],
    'ids': [['string']],
    'contacts': [['string']],
}
//


window.api.post_task = function post_task(formula, algorithm_type, formula_filters, serverOrigin) {
    arguments_as_string = JSON.stringify({
        formula: formula,
        algorithm_type: algorithm_type,
        formula_filters: formula_filters
    });
    return new Promise((resolve, reject) => {
        if (js_wrapped_post(arguments_as_string)) {
            resolve({ 'op_id': "" });
        } else {
            reject('int');
        }
    });
}

window.api.cancel_task = function cancel_task(serverOrigin, op_id) {
    arguments_as_string = JSON.stringify({
        op_id: op_id
    });
    return new Promise((resolve, reject) => {
        if (js_wrapped_cancel(arguments_as_string)) {
            resolve(true);
        } else {
            reject(false);
        }
    });
}

window.api.ping_task = function ping_task(serverOrigin, op_id) {
    arguments_as_string = JSON.stringify({
        op_id: op_id
    });
    return new Promise((resolve, reject) => {
        if (js_wrapped_ping(arguments_as_string)) {
            resolve({ 
                'is_satisfied': "is_satisfied",
                'status': "status",
                'output': "output",
                'variables': "variables",
                'ids': "ids",
                'contacts': "contacts",
            });
        } else {
            reject('int');
        }
    });
}
