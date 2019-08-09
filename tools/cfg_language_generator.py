import sys
from optparse import OptionParser

rules = {} # TODO: make them arguments
start_symbol = None

def load_grammar(filename):
    global start_symbol
    global rules

    for line in open(filename, 'r'):
        rule_components = line.strip('\n').split(' ')
        lhs = rule_components[0]
        # rule_components[1] should be '='
        rhs = rule_components[2:]

        # the start symbol is the first rule's lhs symbol
        if start_symbol is None:
            start_symbol = lhs

        if lhs in rules:
            rules[lhs].append(rhs)
        else:
            rules[lhs] = [rhs]

def length_without_sapces(s):
    return len(s) - s.count(' ')

def generate_language(length, output_filename):
    worklist = [start_symbol]
    non_terminals = rules.keys()

    print("Non-terminals: ", end='')
    print(*non_terminals, sep=', ')

    out = open(output_filename,"w+")

    print("Generating the words with length up to " + str(length) + "...")
    while worklist:
        s = worklist.pop(0)

        if length_without_sapces(s) <= length:
            found_non_terminal = False
            for i, symbol in enumerate(s):
                if symbol in non_terminals:
                    found_non_terminal = True
                    for prodcution in rules[symbol]:
                        worklist.append(s[:i] + ' '.join(prodcution) + s[i + 1:])
                    break

            if not found_non_terminal:
                out.write(s + '\n')
                #print(s)

if __name__ == "__main__":
    parser = OptionParser()
    parser.add_option("-i", "--cfg_filename", dest="cfg_filename", type = "string",
                      help="File containing the context free grammar rules. "
                      "Only single letter non-terminals are supported at the moment. "
                      "Example: (each rule on a separate line)" # TODO: print it on a separate lines
                      "    F = F & F"
                      "    F = x"
                      "    F = y"
                      "    F = var")
    parser.add_option("-l", "--length", dest="length", default = 5, type = "int",
                      help="The generated word's max character length (without spaces)")
    parser.add_option("-o", "--output_filename", dest="output_filename", type = "string", default = "out_cfg_language.txt",
                      help="Filename for the generated language words")

    options, args = parser.parse_args()
    print(options)

    if options.cfg_filename == None:
        print("Forgot to pass the cfg filename?")
        exit()

    load_grammar(options.cfg_filename)
    print('Loaded grammar start_symbol is ' + start_symbol)
    print('Rules: ')
    for lhs in rules:
        print(lhs + ' = ', end = '')
        print(*rules[lhs], sep=',')

    generate_language(options.length, options.output_filename)
