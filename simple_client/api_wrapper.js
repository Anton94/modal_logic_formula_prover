
// Include the js which gives the WASM functions
// Loads it asynchronous but it is OK. The functions are called on user action later on.
var imported = document.createElement('script');
imported.src = './frontend_api.js';
document.head.appendChild(imported);
