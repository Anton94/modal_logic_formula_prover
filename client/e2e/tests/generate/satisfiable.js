const fs = require('fs');
const path = require("path");

describe('#is_satisfied generated ', function() {

    beforeEach(function () {
    	browser.ignoreSynchronization = true;
			browser.get('http://localhost:34567/views/satisfiability_test.html');
    });

    console.log("qweweqwe " + __dirname);
    var file_content = fs.readFileSync(path.resolve(__dirname, '../build/random.txt'), 'utf-8');
    file_content += fs.readFileSync(path.resolve(__dirname, '../build/size10.txt'), 'utf-8');
    console.log(file_content);
    var lines = file_content.split(/\r?\n/);

    lines.forEach(function(line) {
        if (valid_line_formula(line.trim())) {
            it('formula ' + line, function(done) {
                is_satisfied(line);
                done();
            });
        }
    });

    function valid_line_formula(line) {
        if (line === "" || line[0] === "#" || (line[0] === "/" && line[1] === "/")) {
            return false;
        }
        return true;
    }

    function is_satisfied(formula, expect=true, timeout=10000) {
		//element(by.id("formula_field")).sendKeys(Key.chord(Key.CONTROL, 'a'));
		element(by.id("formula_field")).clear().sendKeys(formula);
		element(by.id("checkAllBtn")).click();

		var until = protractor.ExpectedConditions;
		// Waits for the element with id 'output' to contain the text 'neeeded'.
		browser.wait(until.textToBePresentInElement($('#formula_output'), 'Satisfiable? ' + expect), timeout, "Wait timed out for: " + formula + " to be " + expect);

		element(by.id("clearBtn")).click();
	}
});