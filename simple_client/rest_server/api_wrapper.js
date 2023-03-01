// This file is copied as post build step of rest_server app and replaces the one in the parent directory.


// Include the js which gives the WASM functions
// Loads it asynchronous but it is OK. The functions are called on user action later on.
var imported = document.createElement('script');
imported.src = './server_api.js';
document.head.appendChild(imported);
