describe('#is_satisfied', function() {

    beforeEach(function () {
    	browser.ignoreSynchronization = true;
		browser.get('http://localhost:34567/views/build_formula_test.html');
    });

    function is_satisfied(formula, expect=true, timeout=5000) {
    	//element(by.id("formula_field")).sendKeys(Key.chord(Key.CONTROL, 'a'));
    	element(by.id("formula_field")).clear().sendKeys(formula);
		element(by.id("proveBtn")).click();

		var until = protractor.ExpectedConditions;
		// Waits for the element with id 'output' to contain the text 'neeeded'.
		browser.wait(until.textToBePresentInElement($('#formula_output'), expect), timeout, "Wait timed out for: " + formula + " to be " + expect);

		element(by.id("clearBtn")).click();
    }

	it(' THEN there are satisfiable formulas', function() {	
		is_satisfied("C(a,b)");
	});

	it(' THEN there are satisfiable formulas', function() {	
		is_satisfied("C(-a*-b,b+c+-e*m)&<=(a*-b+c, ---a*--b*-c+m+l) | ~<=(a*-b, c+---m)");
	});

	it(' THEN there are satisfiable formulas', function() {	
		is_satisfied("C(a,b)|C(b,c)&C(c,d)|~C(d,e)->C(e,f)<->C(f,g)");
	});

	it(' THEN there are satisfiable formulas', function() {	
		is_satisfied("~~~C(a,b)&~~<=(a,b*-c*-d*-e*---e+ww+aa+m)");
	});

	it(' THEN there are satisfiable formulas', function() {	
		is_satisfied("C(0,1)");
	});

	it(' THEN there are satisfiable formulas', function() {	
		is_satisfied("C(1,1)");
	});

	it(' THEN there are satisfiable formulas', function() {	
		is_satisfied("F");
	});

	it(' THEN there are satisfiable formulas', function() {	
		is_satisfied("T");
	});

	it(' THEN there are satisfiable formulas', function() {	
		is_satisfied("T|F|T|C(a,b)&F&T&<=(0,1)");
	});

});