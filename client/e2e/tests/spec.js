describe('Protractor Demo App', function() {

    beforeEach(function () {
    	browser.ignoreSynchronization = true;
		browser.get('http://localhost:34567/views/satisfiability_test.html');
    });

    function is_satisfied(formula, expect=true) {
    	//element(by.id("formula_field")).sendKeys(Key.chord(Key.CONTROL, 'a'));
    	element(by.id("formula_field")).clear().sendKeys(formula);
		element(by.id("proveBtn")).click();

		var until = protractor.ExpectedConditions;
		// Waits for the element with id 'output' to contain the text 'neeeded'.
		browser.wait(until.textToBePresentInElement($('#formula_output'), 'Satisfiable? ' + expect), 5000);

		element(by.id("clearBtn")).click();
    }

	it('should have a title', function() {
		is_satisfied("C(a,b)");
		is_satisfied("C(a,b) & C(b,c)");
		is_satisfied("C(a, b) | ~C(a,b)");
		is_satisfied("(C(a, b) | <=(a, b)) | ~C(a,b)");
		is_satisfied("(C(a, b) | <=(a, b)) | (~C(a,b) & <=(a,b))");
		is_satisfied("C(a, b) & ~C(a, c)");
		is_satisfied("(C(a, b) | ~C(a, b)) & (C(a,b) | ~C(a,b))");
		is_satisfied("(C(a, b) | ~C(a, b)) & (C(a,b) | ~C(a,c))");
		
		//noup is_satisfied("C(a, *b) & ~C(a, b + h)");
		//noup is_satisfied("(C(a, b) & (C(f, h) & C(j, k))) & ~((C(a, b) & C(f, h)) & C(j, F))");
		
		is_satisfied("<=(a,b) & <=(b,a)");
		is_satisfied("<=(a,b) & ~(<=(b,a))");
		is_satisfied("(<=(a,b) & (~(<=(b,a)))) | (<=(a,b))");
		is_satisfied("(<=(a,b) & (~(<=(a,b)))) | (C(a,b) & (~C(-b,b))))");
		is_satisfied("(C(a,b) & ~C(b,b)) | (C(a,b) & ~C(-b,b))");
		is_satisfied("~C(x * -z,b) & (~C(-b,b) & ~<=(x, z))");
		is_satisfied("~C(x * -z, (b * -k) * c) & (~C(-b,b) & (~<=(x, z) & ~<=(b, k)))");
		is_satisfied("T | F");
		is_satisfied("<=(a, c) & C(a * (-b), c)");
		is_satisfied("<=(a, c) & (C(a * (-c), d) | C(a * (-b), c))");
		is_satisfied("(<=(a, c) | <=(a,b)) & (C(a * (-c), d) | C(a * (-b), c))");
		is_satisfied("(~<=(a,b) & (~<=(c,d) & C(a * -b, c * -d)))");
		is_satisfied("((~<=(x,y) & ~<=(z, t)) & ~<=(a,b)) & C(a * -b, z * -t)");
		is_satisfied("C(a * -d, b * -c) & ( (C(a * -d, -d) & <=(a,d)) | <=(b,x)))");

		is_satisfied("C(a, b) & ~C(a,b)", false);
		//noup is_satisfied("C(a - c, (*b) + h) & ~C(a - c, (*b) + h)", false);
		is_satisfied("(C(a, b) & (C(f, h) & C(j, k))) & ~((C(a, b) & C(f, h)) & C(j, k))", false);
		is_satisfied("<=(a,b) & ~(<=(a,b))", false);
		is_satisfied("(<=(a,b) & (~(<=(a,b)))) | (C(a,b) & (~C(a,b))))", false);
		is_satisfied("(<=(a,b) & (~(<=(a,b)))) | (C(a,b) & (~C(b,b))))", false);
		is_satisfied("~(C([x * -z], [b * -k]) | C([x * -z], c)) & (~C(-b,b) & (~<=(x, z) & ~<=(b, k)))", false);
		is_satisfied("~C(x * -z, (b * -k) + c) & (~C(-b,b) & (~<=(x, z) & ~<=(b, k)))", false);
		is_satisfied("F", false);
		is_satisfied("T & F", false);
		is_satisfied("F | F", false);
		//noup is_satisfied("C(a, *b) & F", false);
		is_satisfied("<=(a, b) & C(a * (-b), c)", false);
		is_satisfied("C(a * (-b), c) & <=(a, b)", false);
		is_satisfied("<=(a, c) & (C(a * (-c), d) & C(a * (-b), c))", false);
		is_satisfied("(<=(a, c) & <=(a,b)) & (C(a * (-c), d) | C(a * (-b), c))", false);
		is_satisfied("~(~<=(a, c) | ~<=(a,b)) & (C(a * (-c), d) | C(a * (-b), c))", false);
		is_satisfied("(~<=(a,b) & (~<=(c,d) & ~C(a * -b, c * -d)))", false);
		is_satisfied("(<=(x,y) & <=(a,b)) & C(a * -b, z)", false);
		is_satisfied("((~<=(x,y) & <=(z, t)) & ~<=(a,b)) & C(a * -b, z * -t)", false);
		is_satisfied("~( ~((<=(x,y) & ~<=(z, t)) & ~<=(a,b)) | C(a * -b, z * -t) )", false);
		is_satisfied("C(a * -d, b * -c) & ( (C(a * -d, d) & <=(a,d)) | <=(b,c)))", false);
		is_satisfied("C(a * -d, b * -c) & ( (C(a * -d, d) & <=(a,d)) | <=(b,x)))", false);
		is_satisfied("~C(a * -d, b * -c) & ( (~C(a * -d, d * -c) & (~<=(a,d) & ~<=(d,c))) | (~<=(b,c) & ~<=(a,d)) )", false);
		is_satisfied("~C(a * -d, a * -d) & ~<=(a,d)", false);
		is_satisfied("~<=(a,d) & ~C(a * -d, a * -d)", false);
	});
});