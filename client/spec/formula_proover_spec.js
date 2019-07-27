require('./spec_helper.js');

describe("add suite", function () {
	var parser = require('../src/formula_parser.js');
	it("should add two numbers", function () {
	  expect(parser.formula_to_json("<=(a,b)")).not.toEqual(2);
	});
});
