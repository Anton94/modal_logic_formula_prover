import sys
import random
import math
from optparse import OptionParser
import string

RETRY_COUNTS = 1000

min_term_length = 1
min_variables_count = 3
min_existence_rules = 4
min_non_existence_rules = 2

parser = OptionParser(usage="Generates -n number of formulas for each combination of -v -k and -m (values of each ranges in [min,max]. Max bound is configurable.")
parser.add_option("-o", "--output_filename", dest="output_filename", type = "string", default = "output.txt",
                  help="Filename for the generated formulas.")
parser.add_option("-t", "--max_term_length", dest="max_term_length", default = 3, type = "int",
                  help="The generated term's max height in it's binary tree representation. The min term length is " + str(min_term_length) + " (fixed).")
parser.add_option("-u", "--min_variables_count", dest="min_variables_count", default = 10, type = "int",
                  help="Min number of variables. Should be grater than " + str(min_variables_count))
parser.add_option("-v", "--max_variables_count", dest="max_variables_count", default = 10, type = "int",
                  help="Max number of variables. The min number of varialbes is " + str(min_variables_count) + " (fixed).")
parser.add_option("-k", "--max_existence_rules", dest="max_existence_rules", default = 5, type = "int",
                  help="Max number of existence rules. The contact brings two, !=0 brings one. The minimal number is " + str(min_existence_rules))
parser.add_option("-l", "--min_non_existence_rules", dest="min_non_existence_rules", default = 2, type = "int",
                  help="Min number of non-existence rules, i.e. the number of ~C and =0 atomics. Minimal value is " + str(min_non_existence_rules))
parser.add_option("-m", "--max_non_existence_rules", dest="max_non_existence_rules", default = 2, type = "int",
                  help="Max number of non-existence rules, i.e. the number of ~C and =0 atomics.")
parser.add_option("-n", "--formulas", dest="formulas", default = 20, type = "int",
                  help="The number of formulas to be generated for each parameter combination(-v -k -m).")

options, args = parser.parse_args()
print(options)

if options.max_variables_count < options.min_variables_count or options.min_variables_count < min_variables_count:
    print("Wrong [-u, -v] range.")
    exit()
if options.max_term_length < min_term_length:
    print("Wrong -t argument, less than the minimal value.")
    exit()
if options.max_existence_rules < min_existence_rules:
    print("Wrong -k argument, less than the minimal value.")
    exit()
if options.max_non_existence_rules < options.min_non_existence_rules or options.min_non_existence_rules < min_non_existence_rules:
    print("Wrong [-l, -m] range.")
    exit()
if options.formulas <= 0:
    print("Wrong -n argument, it should be a positive number.")
    exit()

variables = []
def generate_variables(variables_count):
    global variables
    variables = []
    for i in range(0, variables_count):
        var = chr(ord('a') + i % 26)
        if(i >= 26):
            var = var + str(int(i / 26))
        variables.append(var)

def get_random_variable():
    return random.choice(variables)

def generate_term(logic_symbols_count):
    if(logic_symbols_count <= 1):
        return get_random_variable()
    r = random.randint(1, 100)
    if r <= 20: # generate a variable
        return get_random_variable()
    if r <= 40: # generate a negation
        return "-" + generate_term(logic_symbols_count - 1)
    # generate binary operation - union or intersection
    op = "+" if r <= 70 else "*"
    left = generate_term(logic_symbols_count - 1)
    right = generate_term(logic_symbols_count - 1)
    return "(" + left + " " + op + " " + right + ")"

out = open(options.output_filename,"w+")

terms = set()
def generate_unique_term():
    for i in range(0, RETRY_COUNTS):
        # try 100 times to generate new combination if a == b or b == c or a == c
        # if not able then the restrictions are too strong
        # (or we are out of 'luck' to not generate a suitable one, but it's good enough)
        t = generate_term(options.max_term_length)
        if t not in terms:
            return t

    print("Not able to generate new unique term with the provided parameter ranges.")
    out.close()
    exit()

def C(t1, t2):
    return "C(" + t1 + ", " + t2 + ")"

def le(t1, t2):
    return "<=(" + t1 + ", " + t2 + ")"

def create_union_all_vars_not_zero():
    # creates ~((x + y + .. + z)=0)
    t = "~(";
    t = t + ' + '.join(variables)
    t = t + ")=0"
    return t

def generate_contact(contacts_A, contacts_B, terms_to_ignore):
    for i in range(0,RETRY_COUNTS):
        a = generate_term(options.max_term_length)
        b = generate_term(options.max_term_length)
        # TODO unique terms in the contact!
        if a not in terms_to_ignore and b not in terms_to_ignore and C(a,b) not in contacts_A and C(a,b) not in contacts_B:
            return (a,b)

    out.close()
    print("Not able to generate new contact with the provided parameter ranges.")
    exit()

def generate_term_with_ignore_collections(ignore_terms_A, ignore_terms_B, ignore_terms_C):
    for i in range(0,RETRY_COUNTS):
        a = generate_term(options.max_term_length)
        if a not in ignore_terms_A and a not in ignore_terms_B and a not in ignore_terms_C:
            return a

    out.close() # TODO: make a termination function
    print("Not able to generate new non-zero term with the provided parameter ranges.")
    exit()

def generate_formula(v, k, m):
    # generate the contradiction part: <=(b,a) & C(b,c) & ~C(a, c)
    # generate an atomic with all variables in order all of them to be used: x + y + ... + z != 0
    # Note that we do not want to create a formula with a lexical contradiction or at least to bring to minimum the chance to create such
    # I.e. C(a,b) & ... & ~C(b,a)
    # So we need to keep track of all terms in contact
    # But also, we do not want to create the following: C(a,b) & .. & a = 0
    # because it's also a lexilac contradiction.
    # If the terms are only variables it will bi guaranteed that there won't be any lexical contradictions,
    # but if they are complex - there might be some reduces or conversions which modify the atomics and some contradictions to pop up.
    # Also, we need to generate unique atomic formulas, so no duplications (i.e. no ... & C(a,b) & ... & C(a,b) &...)
    global terms
    terms = set()

    a = generate_unique_term()
    terms.add(a)
    b = generate_unique_term()
    terms.add(b)
    c = generate_unique_term()
    terms.add(c)
    g = create_union_all_vars_not_zero()

    f = le(b,a) + " & " + C(b,c) + " & ~" + C(a, c) + " & " + g

    # for now we have 4 existence rules and one non-existence
    zero_terms = { b + " * -" + a } # <=(b,a) -> b * -a = 0
    non_zero_terms = set()
    contacts = { C(b,c), C(c,b) }
    neg_contacts = { C(a,c), C(c,a) }
    contact_terms = { b, c }

    k -= 3 # C(b,c) & g, where g is ~(x + y + .. + z)=0
    m -= 2 # ~C(a,c) & <=(b, a)

    # generate the rest of the existence rules
    while k > 0:
        r = random.randint(1, 2)
        if r == 1 and k >= 2: # generate a contact
            (a,b) = generate_contact(contacts, neg_contacts, zero_terms)
            terms.add(a)
            terms.add(b)

            f += " & " + C(a,b)
            contacts.add(C(a,b))
            contacts.add(C(b,a)) # bacuse of the comutativity
            contact_terms.add(a)
            contact_terms.add(b)
            k -= 2
        else: # generate !=0
            a = generate_term_with_ignore_collections(non_zero_terms, zero_terms, set())
            terms.add(a)

            f += " & ~(" + a + ")=0"
            non_zero_terms.add(a)
            k -= 1

    # generate the rest of non-existence rules
    for i in range(0, m):
        r = random.randint(1, 2)
        if r == 1: # generate a ~contact
            (a,b) = generate_contact(contacts, neg_contacts, set())
            terms.add(a)
            terms.add(b)

            f += " & ~" + C(a,b)
            neg_contacts.add(C(a,b))
            neg_contacts.add(C(b,a)) # bacuse of the comutativity
        else: # generate =0
            a = generate_term_with_ignore_collections(non_zero_terms, zero_terms, contact_terms)
            terms.add(a)

            f += " & (" + a + ")=0"
            zero_terms.add(a)

    return f

out.write(str(options.formulas) + "\n")

for k in range(min_existence_rules, options.max_existence_rules + 1):
    for m in range(options.min_non_existence_rules, options.max_non_existence_rules + 1):
        for v in range(options.min_variables_count, options.max_variables_count + 1):
            generate_variables(v)
            out.write("INFO: " + str(v) + " variables, " + str(k) + " existence rules, " + str(m) + " non existence rules.\n")
            for l in range(0, options.formulas):
                f = generate_formula(v, k, m)
                out.write(f + "\n")

out.close()

print("Done")

