
// Include the js which gives the WASM functions
// Loads it asynchronous but it is OK. The functions are called on user action later on.
var imported = document.createElement('script');
imported.src = './formula_proover_wasm/formula_proover_wasm.js';
document.head.appendChild(imported);

window.api = {
    is_instant: true
};

// source: https://stackoverflow.com/questions/18729405/how-to-convert-utf8-string-to-byte-array
function toUTF8Array(str) {
    var utf8 = [];
    for (var i=0; i < str.length; i++) {
        var charcode = str.charCodeAt(i);
        if (charcode < 0x80) utf8.push(charcode);
        else if (charcode < 0x800) {
            utf8.push(0xc0 | (charcode >> 6), 0x80 | (charcode & 0x3f));
        }
        else if (charcode < 0xd800 || charcode >= 0xe000) {
            utf8.push(0xe0 | (charcode >> 12), 0x80 | ((charcode>>6) & 0x3f), 0x80 | (charcode & 0x3f));
        }
        else {
            i++;
            charcode = 0x10000 + (((charcode & 0x3ff)<<10) | (str.charCodeAt(i) & 0x3ff));
            utf8.push(0xf0 | (charcode >>18), 0x80 | ((charcode>>12) & 0x3f), 0x80 | ((charcode>>6) & 0x3f), 0x80 | (charcode & 0x3f));
        }
    }
    return utf8;
}

function allocate_WASM_string(s) {
    var len  = s.length;
    var converted_formula = new Uint8Array(toUTF8Array(s)) // Array of bytes (8-bit unsigned int) representing the string

    var ptr = Module._malloc(len * 1);     // Allocate memory - 1 byte per element

    Module.HEAPU8.set(converted_formula, ptr); // Write WASM memory calling the set method of the Uint8Array

    return ptr;
}

function free_WASM_mem(m) {
    Module._free(m);
}

//var js_wrapped_calculate = function() {
//    return '{"contacts":[["1","1","0","0"],["1","1","0","0"],["0","0","1","0"],["0","0","0","1"]],"ids":[["1","0","1","0"],["0","1","0","1"]],"is_satisfied":true,"output":"Info: Start building a formula:\\n~a=0 & ~b=0 & C(a,b)\\nInfo: \\nParsed formula            : ((~(a)=0 & ~(b)=0) & C(a, b))\\nConverted (-> <->)        : ((~(a)=0 & ~(b)=0) & C(a, b))\\nConverted C(a,a);<=(a,a)  : ((~(a)=0 & ~(b)=0) & C(a, b))\\nC(a+b,c)->C(a,c)|C(b,c) ;\\n<=(a+b,c)-><=(a,c)&<=(b,c): ((~(a)=0 & ~(b)=0) & C(a, b))\\nConverted C(a,a);<=(a,a)  : ((~(a)=0 & ~(b)=0) & C(a, b))\\nConverted (<= =0) formula : ((~(a)=0 & ~(b)=0) & C(a, b))\\nReduced constants         : ((~(a)=0 & ~(b)=0) & C(a, b))\\nConverted C(a,1)->~(a=0)  : ((~(a)=0 & ~(b)=0) & C(a, b))\\n\\nInfo: Running satisfiability checking of ((~(a)=0 & ~(b)=0) & C(a, b))...\\nTrace: Adding ((~(a)=0 & ~(b)=0) & C(a, b)) to T formulas\\nTrace: Making an algorithm step:\\nTrace:            Formulas T: ((~(a)=0 & ~(b)=0) & C(a, b)) \\nTrace:            Formulas F: \\nTrace:            Contacts T: \\nTrace:            Contacts F: \\nTrace:          Zero terms T: \\nTrace:          Zero terms F: \\nTrace:         Measured <= T: \\nTrace:         Measured <= F: \\nTrace:      T contacts terms: \\nTrace: Processing ((~(a)=0 & ~(b)=0) & C(a, b)) from T formulas\\nTrace:            Formulas T: \\nTrace:            Formulas F: \\nTrace:            Contacts T: \\nTrace:            Contacts F: \\nTrace:          Zero terms T: \\nTrace:          Zero terms F: \\nTrace:         Measured <= T: \\nTrace:         Measured <= F: \\nTrace:      T contacts terms: \\nTrace: Adding (~(a)=0 & ~(b)=0) to T formulas\\nTrace: Adding C(a, b) to T contacts\\nTrace: Adding a to the terms of T contacts\\nTrace: Adding b to the terms of T contacts\\nTrace: Making an algorithm step:\\nTrace:            Formulas T: (~(a)=0 & ~(b)=0) \\nTrace:            Formulas F: \\nTrace:            Contacts T: C(a, b) \\nTrace:            Contacts F: \\nTrace:          Zero terms T: \\nTrace:          Zero terms F: \\nTrace:         Measured <= T: \\nTrace:         Measured <= F: \\nTrace:      T contacts terms: b a \\nTrace: Processing (~(a)=0 & ~(b)=0) from T formulas\\nTrace:            Formulas T: \\nTrace:            Formulas F: \\nTrace:            Contacts T: C(a, b) \\nTrace:            Contacts F: \\nTrace:          Zero terms T: \\nTrace:          Zero terms F: \\nTrace:         Measured <= T: \\nTrace:         Measured <= F: \\nTrace:      T contacts terms: b a \\nTrace: Adding ~(a)=0 to T formulas\\nTrace: Adding ~(b)=0 to T formulas\\nTrace: Making an algorithm step:\\nTrace:            Formulas T: ~(b)=0 ~(a)=0 \\nTrace:            Formulas F: \\nTrace:            Contacts T: C(a, b) \\nTrace:            Contacts F: \\nTrace:          Zero terms T: \\nTrace:          Zero terms F: \\nTrace:         Measured <= T: \\nTrace:         Measured <= F: \\nTrace:      T contacts terms: b a \\nTrace: Processing ~(b)=0 from T formulas\\nTrace:            Formulas T: ~(a)=0 \\nTrace:            Formulas F: \\nTrace:            Contacts T: C(a, b) \\nTrace:            Contacts F: \\nTrace:          Zero terms T: \\nTrace:          Zero terms F: \\nTrace:         Measured <= T: \\nTrace:         Measured <= F: \\nTrace:      T contacts terms: b a \\nTrace: Adding b to zero terms F\\nTrace: Making an algorithm step:\\nTrace:            Formulas T: ~(a)=0 \\nTrace:            Formulas F: \\nTrace:            Contacts T: C(a, b) \\nTrace:            Contacts F: \\nTrace:          Zero terms T: \\nTrace:          Zero terms F: b \\nTrace:         Measured <= T: \\nTrace:         Measured <= F: \\nTrace:      T contacts terms: b a \\nTrace: Processing ~(a)=0 from T formulas\\nTrace:            Formulas T: \\nTrace:            Formulas F: \\nTrace:            Contacts T: C(a, b) \\nTrace:            Contacts F: \\nTrace:          Zero terms T: \\nTrace:          Zero terms F: b \\nTrace:         Measured <= T: \\nTrace:         Measured <= F: \\nTrace:      T contacts terms: b a \\nTrace: Adding a to zero terms F\\nTrace: Making an algorithm step:\\nTrace:            Formulas T: \\nTrace:            Formulas F: \\nTrace:            Contacts T: C(a, b) \\nTrace:            Contacts F: \\nTrace:          Zero terms T: \\nTrace:          Zero terms F: a b \\nTrace:         Measured <= T: \\nTrace:         Measured <= F: \\nTrace:      T contacts terms: b a \\nTrace: There is no contradiciton in the path.\\nTrace: Start looking for an satisfiable model.\\nTrace: Start creating a model.\\nTrace: Used variables are 2. Total variables in the formula are 2.\\nTrace: Found a model:\\nModel points: \\n0 : a:1 b:0 \\n1 : a:0 b:1 \\n2 : a:1 b:0 \\n3 : a:0 b:1 \\nModel contact relations: \\n0 is in contact with: 0 1 \\n1 is in contact with: 0 1 \\n2 is in contact with: 2 \\n3 is in contact with: 3 \\nModel evaluation of the variables: \\nv(a) = { 0, 2 }\\nv(b) = { 1, 3 }\\n\\nTrace: Constructed a satisfiable model.\\nTrace: Removing a from zero terms\\nTrace: Returning processed ~(a)=0 back to T formulas\\nTrace: Removing b from zero terms\\nTrace: Returning processed ~(b)=0 back to T formulas\\nTrace: Removing ~(a)=0 from T formulas\\nTrace: Removing ~(b)=0 from T formulas\\nTrace: Returning processed (~(a)=0 & ~(b)=0) back to T formulas\\nTrace: Removing (~(a)=0 & ~(b)=0) from T formulas\\nTrace: Removing C(a, b) from T contacts\\nTrace: Removing a from the terms of T contacts\\nTrace: Removing b from the terms of T contacts\\nTrace: Returning processed ((~(a)=0 & ~(b)=0) & C(a, b)) back to T formulas\\nInfo: Satisfiable. Took 0ms.\\nInfo: Running satisfiability checking of ((~(a)=0 & ~(b)=0) & C(a, b)) with provided model: \\nModel points: \\n0 : a:1 b:0 \\n1 : a:0 b:1 \\n2 : a:1 b:0 \\n3 : a:0 b:1 \\nModel contact relations: \\n0 is in contact with: 0 1 \\n1 is in contact with: 0 1 \\n2 is in contact with: 2 \\n3 is in contact with: 3 \\nModel evaluation of the variables: \\nv(a) = { 0, 2 }\\nv(b) = { 1, 3 }\\n\\n","status":"FINISHED","variables":["a","b"]}';
//};

// return type = {
//    'is_satisfied': 'bool',
//    'is_parsed': 'bool',
//    'status': 'string', // FINISHED
//    'output': 'string',
//    'variables': ['string'],
//    'ids': [['string']],
//    'contacts': [['string']],
//}

// wasm has only one function is_satisfiable which returns the output as described above. It is a blocking operation.

window.api.is_satisfiable = function is_satisfiable(formula, algorithm_type, formula_filters) {
    input_data = JSON.stringify({
        formula: formula,
        algorithm_type: algorithm_type,
        formula_filters: formula_filters
    });

    var js_wrapped_calculate = Module.cwrap("calculate", "number", ["number", "number"]);

    var len = input_data.length;
    var wasm_input_data = allocate_WASM_string(input_data);

    var res = js_wrapped_calculate(wasm_input_data, len); // Call the WASM function
    var s = UTF8ToString(res);
    var resultAsJson = JSON.parse(s);

    console.log("The result of calling calculate is:\n", resultAsJson);

    free_WASM_mem(wasm_input_data); // Free the allocated memory for the input formula
    free_WASM_mem(res); // Free the memory allocated for the result.

    return resultAsJson;
}
