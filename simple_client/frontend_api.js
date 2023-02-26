window.api = {};

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
    return new Promise((resolve, reject) => {
        if (true) {
            resolve({ 'op_id': "" });
        } else {
            reject('int');
        }
    });
}

window.api.cancel_task = function cancel_task(serverOrigin, op_id) {
    return new Promise((resolve, reject) => {
        if (true) {
            resolve(true);
        } else {
            reject(false);
        }
    });
}

window.api.ping_task = function ping_task(serverOrigin, op_id) {
    return new Promise((resolve, reject) => {
        if (true) {
            resolve({ 
                'is_satisfied': "is_satisfied"],
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
