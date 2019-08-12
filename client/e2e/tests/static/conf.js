exports.config = {
  framework: 'jasmine',
  seleniumAddress: 'http://localhost:4444/wd/hub',
  specs: ['./satisfiability.js',
    './formula_building.js'
  ]
}