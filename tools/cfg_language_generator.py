import sys
import random
import math
from optparse import OptionParser

rules = {} # TODO: make them arguments
start_symbol = None

def print_grammar():
    print('Grammar start_symbol is ' + start_symbol)
    print("Non-terminals: ", end='')
    print(*rules.keys(), sep=', ')
    print('Rules: ')
    for lhs in rules:
        print(lhs + ' = ', end = '')
        print(*rules[lhs], sep=',')

def load_grammar(filename):
    global start_symbol
    global rules

    for line in open(filename, 'r'):
        # the format is of the following type: S 0.5 = K M L
        # where S is the non terminal, 0.5 is optional probability factor
        # for choosing that rule when generating a random word from the language
        # and K M L is right hand side of the rule
        rule_components = line.strip('\n').split(' ')
        lhs = rule_components[0]

        if '#' in lhs: # Skip the comment lines
            continue

        probability = None
        if rule_components[1] == '=':
            rhs = rule_components[2:]
        else:
            probability = rule_components[1]
            rhs = rule_components[3:]

        # the start symbol is the first rule's lhs symbol
        if start_symbol is None:
            start_symbol = lhs

        if lhs in rules:
            rules[lhs].append((probability, rhs))
        else:
            rules[lhs] = [(probability, rhs)]

    # autofill the missing probabilities
    for non_terminal, productions in rules.items():
        # productions is of type [(probability, rhs), ...]
        accumulated_probability = 0
        productions_without_probability = 0
        for i, (probability, production) in enumerate(productions):
            if probability == None:
                productions_without_probability = productions_without_probability + 1
            else:
                float_probability = round(float(probability), 3)
                productions[i] = (float_probability, production)
                accumulated_probability = accumulated_probability + float_probability

        if accumulated_probability > 1:
            print("The non_terminal " + non_terminal + " rules has combined probability of more than 1")
            exit()

        if  productions_without_probability > 0:
            missing_probability = (1 - accumulated_probability) / productions_without_probability
            for i, (probability, production) in enumerate(productions):
                if probability == None:
                    productions[i] = (missing_probability, production)

    print_grammar()

    # swap the probabilities with numbers from 1 to 1000
    # that way when we generate a radnom number in that range
    # and easily find out which rule to choose
    for non_terminal, productions in rules.items():
        accumulated = 0
        for i, (probability, production) in enumerate(productions):
            accumulated = accumulated + probability * 1000
            productions[i] = (accumulated, production)

    print("DEBUG: Swapping the probabilities with numbers in range[1, 1000]")
    print_grammar()

def length_without_sapces(s):
    return len(s) - s.count(' ')

def generate_language(length, output_filename):
    worklist = [start_symbol]
    non_terminals = rules.keys()

    out = open(output_filename,"w+")

    print("Generating the words with length up to " + str(length) + "...")
    while worklist:
        s = worklist.pop(0)

        if length_without_sapces(s) <= length:
            found_non_terminal = False
            for i, symbol in enumerate(s):
                if symbol in non_terminals:
                    found_non_terminal = True
                    for probability_bound, prodcution in rules[symbol]:
                        worklist.append(s[:i] + ' '.join(prodcution) + s[i + 1:])
                    break

            if not found_non_terminal:
                out.write(s + '\n')
                #print(s)

def get_random_rule(rules):
    r = random.randint(1, 1000)
    for probability_bound, rule in rules:
        if r < probability_bound or math.isclose(r, probability_bound, rel_tol=1e-5):
            return rule
    print("Sorry, could not fined a suitable rule with that probabilities, code error...")
    exit()

def generate_random_words(length, count, output_filename):
    non_terminals = rules.keys()

    out = open(output_filename,"w+")

    print("Generating " + str(count) + " random words with length up to " + str(length) + "...")
    while count > 0:
        worklist = [start_symbol]
        while worklist:
            s = worklist.pop(0)

            if length_without_sapces(s) <= length:
                found_non_terminal = False
                for i, symbol in enumerate(s):
                    if symbol in non_terminals:
                        found_non_terminal = True
                        rule = get_random_rule(rules[symbol])
                        worklist.append(s[:i] + ' '.join(rule) + s[i + 1:])
                        break

                if not found_non_terminal:
                    out.write(s + '\n')
                    count = count - 1
                    #print(s)

if __name__ == "__main__":
    parser = OptionParser()
    parser.add_option("-i", "--cfg_filename", dest="cfg_filename", type = "string",
                      help="File containing the context free grammar rules. "
                      "Only single letter non-terminals are supported at the moment. "
                      "Use '#' at the beginning of a line to indicate that it's a comment and it will be ignored. "
                      "Example: (each rule on a separate line)" # TODO: print it on a separate lines
                      "    F = F & F"
                      "    F = x"
                      "    F = y"
                      "    F = var    "
                      "The grammar can have a probability factor (between 0 and 1, at most 3 digits after the decimal point) "
                      "for some of it's rules which are going to be used for random word generation. "
                      "For example: "
                      "    F 0.5 = F & F"
                      "    F = x"
                      "    F = y"
                      "    F 0.1 = var"
                      " When resolving the non-terminal F there is 50% chance to choose 'F & F', "
                      " 10% chance to choose 'var', (auto-calculated) 30 % chance for 'x' and 30% chance for 'y'")
    parser.add_option("-o", "--output_filename", dest="output_filename", type = "string", default = "out_cfg_language.txt",
                      help="Filename for the generated language words")
    parser.add_option("-l", "--length", dest="length", default = 10, type = "int",
                      help="The generated word's max character length (without spaces)")
    parser.add_option("-r", "--random_words_count", dest="random_words_count", default = 0, type = "int",
                      help="If bigger than 0 it will generate random words of that ammount instead of all words. "
                      "Make sure that the length is big enough or it might not be able to generate the words and hold")

    options, args = parser.parse_args()
    print(options)

    if options.cfg_filename == None:
        print("Forgot to pass the cfg filename?")
        exit()

    load_grammar(options.cfg_filename)

    if options.random_words_count > 0:
        generate_random_words(options.length, options.random_words_count, options.output_filename)
    else:
        generate_language(options.length, options.output_filename)
