window.api = {
    is_instant: true
};

var js_wrapped_calculate = function() {
    return '{"contacts":[["1","1","0","0"],["1","1","0","0"],["0","0","1","0"],["0","0","0","1"]],"ids":[["1","0","1","0"],["0","1","0","1"]],"is_satisfied":true,"output":"Info: Start building a formula:\\n~a=0 & ~b=0 & C(a,b)\\nInfo: \\nParsed formula            : ((~(a)=0 & ~(b)=0) & C(a, b))\\nConverted (-> <->)        : ((~(a)=0 & ~(b)=0) & C(a, b))\\nConverted C(a,a);<=(a,a)  : ((~(a)=0 & ~(b)=0) & C(a, b))\\nC(a+b,c)->C(a,c)|C(b,c) ;\\n<=(a+b,c)-><=(a,c)&<=(b,c): ((~(a)=0 & ~(b)=0) & C(a, b))\\nConverted C(a,a);<=(a,a)  : ((~(a)=0 & ~(b)=0) & C(a, b))\\nConverted (<= =0) formula : ((~(a)=0 & ~(b)=0) & C(a, b))\\nReduced constants         : ((~(a)=0 & ~(b)=0) & C(a, b))\\nConverted C(a,1)->~(a=0)  : ((~(a)=0 & ~(b)=0) & C(a, b))\\n\\nInfo: Running satisfiability checking of ((~(a)=0 & ~(b)=0) & C(a, b))...\\nTrace: Adding ((~(a)=0 & ~(b)=0) & C(a, b)) to T formulas\\nTrace: Making an algorithm step:\\nTrace:            Formulas T: ((~(a)=0 & ~(b)=0) & C(a, b)) \\nTrace:            Formulas F: \\nTrace:            Contacts T: \\nTrace:            Contacts F: \\nTrace:          Zero terms T: \\nTrace:          Zero terms F: \\nTrace:         Measured <= T: \\nTrace:         Measured <= F: \\nTrace:      T contacts terms: \\nTrace: Processing ((~(a)=0 & ~(b)=0) & C(a, b)) from T formulas\\nTrace:            Formulas T: \\nTrace:            Formulas F: \\nTrace:            Contacts T: \\nTrace:            Contacts F: \\nTrace:          Zero terms T: \\nTrace:          Zero terms F: \\nTrace:         Measured <= T: \\nTrace:         Measured <= F: \\nTrace:      T contacts terms: \\nTrace: Adding (~(a)=0 & ~(b)=0) to T formulas\\nTrace: Adding C(a, b) to T contacts\\nTrace: Adding a to the terms of T contacts\\nTrace: Adding b to the terms of T contacts\\nTrace: Making an algorithm step:\\nTrace:            Formulas T: (~(a)=0 & ~(b)=0) \\nTrace:            Formulas F: \\nTrace:            Contacts T: C(a, b) \\nTrace:            Contacts F: \\nTrace:          Zero terms T: \\nTrace:          Zero terms F: \\nTrace:         Measured <= T: \\nTrace:         Measured <= F: \\nTrace:      T contacts terms: b a \\nTrace: Processing (~(a)=0 & ~(b)=0) from T formulas\\nTrace:            Formulas T: \\nTrace:            Formulas F: \\nTrace:            Contacts T: C(a, b) \\nTrace:            Contacts F: \\nTrace:          Zero terms T: \\nTrace:          Zero terms F: \\nTrace:         Measured <= T: \\nTrace:         Measured <= F: \\nTrace:      T contacts terms: b a \\nTrace: Adding ~(a)=0 to T formulas\\nTrace: Adding ~(b)=0 to T formulas\\nTrace: Making an algorithm step:\\nTrace:            Formulas T: ~(b)=0 ~(a)=0 \\nTrace:            Formulas F: \\nTrace:            Contacts T: C(a, b) \\nTrace:            Contacts F: \\nTrace:          Zero terms T: \\nTrace:          Zero terms F: \\nTrace:         Measured <= T: \\nTrace:         Measured <= F: \\nTrace:      T contacts terms: b a \\nTrace: Processing ~(b)=0 from T formulas\\nTrace:            Formulas T: ~(a)=0 \\nTrace:            Formulas F: \\nTrace:            Contacts T: C(a, b) \\nTrace:            Contacts F: \\nTrace:          Zero terms T: \\nTrace:          Zero terms F: \\nTrace:         Measured <= T: \\nTrace:         Measured <= F: \\nTrace:      T contacts terms: b a \\nTrace: Adding b to zero terms F\\nTrace: Making an algorithm step:\\nTrace:            Formulas T: ~(a)=0 \\nTrace:            Formulas F: \\nTrace:            Contacts T: C(a, b) \\nTrace:            Contacts F: \\nTrace:          Zero terms T: \\nTrace:          Zero terms F: b \\nTrace:         Measured <= T: \\nTrace:         Measured <= F: \\nTrace:      T contacts terms: b a \\nTrace: Processing ~(a)=0 from T formulas\\nTrace:            Formulas T: \\nTrace:            Formulas F: \\nTrace:            Contacts T: C(a, b) \\nTrace:            Contacts F: \\nTrace:          Zero terms T: \\nTrace:          Zero terms F: b \\nTrace:         Measured <= T: \\nTrace:         Measured <= F: \\nTrace:      T contacts terms: b a \\nTrace: Adding a to zero terms F\\nTrace: Making an algorithm step:\\nTrace:            Formulas T: \\nTrace:            Formulas F: \\nTrace:            Contacts T: C(a, b) \\nTrace:            Contacts F: \\nTrace:          Zero terms T: \\nTrace:          Zero terms F: a b \\nTrace:         Measured <= T: \\nTrace:         Measured <= F: \\nTrace:      T contacts terms: b a \\nTrace: There is no contradiciton in the path.\\nTrace: Start looking for an satisfiable model.\\nTrace: Start creating a model.\\nTrace: Used variables are 2. Total variables in the formula are 2.\\nTrace: Found a model:\\nModel points: \\n0 : a:1 b:0 \\n1 : a:0 b:1 \\n2 : a:1 b:0 \\n3 : a:0 b:1 \\nModel contact relations: \\n0 is in contact with: 0 1 \\n1 is in contact with: 0 1 \\n2 is in contact with: 2 \\n3 is in contact with: 3 \\nModel evaluation of the variables: \\nv(a) = { 0, 2 }\\nv(b) = { 1, 3 }\\n\\nTrace: Constructed a satisfiable model.\\nTrace: Removing a from zero terms\\nTrace: Returning processed ~(a)=0 back to T formulas\\nTrace: Removing b from zero terms\\nTrace: Returning processed ~(b)=0 back to T formulas\\nTrace: Removing ~(a)=0 from T formulas\\nTrace: Removing ~(b)=0 from T formulas\\nTrace: Returning processed (~(a)=0 & ~(b)=0) back to T formulas\\nTrace: Removing (~(a)=0 & ~(b)=0) from T formulas\\nTrace: Removing C(a, b) from T contacts\\nTrace: Removing a from the terms of T contacts\\nTrace: Removing b from the terms of T contacts\\nTrace: Returning processed ((~(a)=0 & ~(b)=0) & C(a, b)) back to T formulas\\nInfo: Satisfiable. Took 0ms.\\nInfo: Running satisfiability checking of ((~(a)=0 & ~(b)=0) & C(a, b)) with provided model: \\nModel points: \\n0 : a:1 b:0 \\n1 : a:0 b:1 \\n2 : a:1 b:0 \\n3 : a:0 b:1 \\nModel contact relations: \\n0 is in contact with: 0 1 \\n1 is in contact with: 0 1 \\n2 is in contact with: 2 \\n3 is in contact with: 3 \\nModel evaluation of the variables: \\nv(a) = { 0, 2 }\\nv(b) = { 1, 3 }\\n\\n","status":"FINISHED","variables":["a","b"]}';
};//Module.cwrap("post", "number", ["number", "number"]);

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


// wasm has only one function is_satisfiable which returns the output as ping does
window.api.is_satisfiable = function is_satisfiable(formula, algorithm_type, formula_filters) {
    arguments_as_string = JSON.stringify({
        formula: formula,
        algorithm_type: algorithm_type,
        formula_filters: formula_filters
    });
    var resultString = js_wrapped_calculate(arguments_as_string);

    return JSON.parse(resultString);
}

// window.api.post_task = function post_task(formula, algorithm_type, formula_filters, serverOrigin) {
//     arguments_as_string = JSON.stringify({
//         formula: formula,
//         algorithm_type: algorithm_type,
//         formula_filters: formula_filters
//     });
//     return new Promise((resolve, reject) => {
//         if (js_wrapped_post(arguments_as_string)) {
//             resolve({ 'op_id': "" });
//         } else {
//             reject('int');
//         }
//     });
// }

// window.api.cancel_task = function cancel_task(serverOrigin, op_id) {
//     arguments_as_string = JSON.stringify({
//         op_id: op_id
//     });
//     return new Promise((resolve, reject) => {
//         if (js_wrapped_cancel(arguments_as_string)) {
//             resolve(true);
//         } else {
//             reject(false);
//         }
//     });
// }

// window.api.ping_task = function ping_task(serverOrigin, op_id) {
//     arguments_as_string = JSON.stringify({
//         op_id: op_id
//     });
//     return new Promise((resolve, reject) => {
//         if (js_wrapped_ping(arguments_as_string)) {
//             resolve({ 
//                 'is_satisfied': "is_satisfied",
//                 'status': "status",
//                 'output': "output",
//                 'variables': "variables",
//                 'ids': "ids",
//                 'contacts': "contacts",
//             });
//         } else {
//             reject('int');
//         }
//     });
// }
