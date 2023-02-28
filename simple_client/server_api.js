window.api = {
    is_instant: false
};

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
    var params =
        "?algorithm_type=" + algorithm_type +
        "&formula_filters=" + formula_filters;
    return new Promise((resolve, reject) => {
        $.post(serverOrigin + "/task" + params, formula)
            .done(function (data) {
                console.log("Posted task");
                resolve({ 'op_id': data });
            })
            .fail(function (data) {
                console.log("Task failed to post " + data.status);
                reject(data.status);
            })
        });
}

window.api.cancel_task = function cancel_task(serverOrigin, op_id) {
    var params = "?op_id=" + op_id;
    return new Promise((resolve, reject) => {
        $.get(serverOrigin + "/rest/cancel" + params, null)
            .done(function (data) {
                console.log("Canceled task " + op_id);
                resolve(true);
            })
            .fail(function (data) {
                console.log("Task cancelation failed " + data.status);
                reject(false);
            })
        });
}

window.api.ping_task = function ping_task(serverOrigin, op_id) {
    var params = "?op_id=" + op_id;
    return new Promise((resolve, reject) => {
        $.get(serverOrigin + "/rest/ping" + params, null)
            .done(function (data) {
                console.log("Posted task");
                // depending on the data return RUNNING or CANCELED or FINISHED...
                console.log(data);
                var jsonified = JSON.parse(data.replace(/'/g, "\""));

                resolve({ 
                    'is_satisfied': jsonified["is_satisfied"],
                    'status': jsonified["status"],
                    'output': jsonified["output"],
                    'variables': jsonified["variables"],
                    'ids': jsonified["ids"],
                    'contacts': jsonified["contacts"],
                });
            })
            .fail(function (data) {
                console.log("Task failed to post " + data.status);
                reject(data.status);
            })
        });
}
