require('./spec_helper.js');

describe("add suite", function () {
	var parser = require('../src/resources/main.js');
	var formulas_map = require('./formulas_map.js');

    beforeEach(function () {
        jasmine.addMatchers({
            toDeepEqual: function () {
                return {
                    compare: function (actual, expected) {
                        return {
                            pass: _.isEqual(actual, expected)
                        };
                    }
                };
            }
        });
    });

	it("parsed formulas", function () {

		var allFormulas = Object.keys(formulas_map.mock_json_by_formula);
		for(var i = 0; i < allFormulas.length; ++i) {
  			expect(parser.formula_to_json(allFormulas[i]))
  				.toDeepEqual(formulas_map.mock_json_by_formula[allFormulas[i]]);	
		}
	});
});
