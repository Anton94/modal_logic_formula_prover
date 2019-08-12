# copy the output files to a log directory/ remove for now
rm -rf ./e2e/tests/build/* .
# generate new
python ../tools/cfg_language_generator.py -i ../tools/formula_cfg.txt -l 3600 -r 3 -o ./e2e/tests/build/random.txt .
python ../tools/cfg_language_generator.py -i ../tools/formula_cfg.txt -l 10 -o ./e2e/tests/build/size10.txt .