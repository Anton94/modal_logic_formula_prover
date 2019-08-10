exports.config = {
  framework: 'jasmine',
  seleniumAddress: 'http://localhost:4444/wd/hub',
  specs: [//'tests/satisfiability.js',
    //'tests/formula_building.js',
    'tests/generate/satisfiable.js'
  ]
}