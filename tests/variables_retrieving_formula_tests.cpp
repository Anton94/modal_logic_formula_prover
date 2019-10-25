#include "catch/catch.hpp"

#include "library.h"
#include <string>
#include <unordered_set>

using json = nlohmann::json;

void variables_check(const std::string& f_str, const variables_set_t& expected_variables, const formula_mgr::formula_refiners& flags = formula_mgr::formula_refiners::all)
{
    formula_mgr f;
    CHECK(f.build(f_str, flags));

    info() << "Checking that " << f << " has the following variables: " << expected_variables;
    const auto variables = f.get_variables();
    CHECK(variables.size() == expected_variables.size());
    for(const auto& variable : variables)
    {
        CHECK(expected_variables.find(variable) != expected_variables.end());
    }
}

TEST_CASE("variables_check 1", "[variables_check_formula_mgr]")
{
    variables_check("C(a, b) | ~C(a,b)", {"a", "b"});
}

TEST_CASE("variables_check 2", "[variables_check_formula_mgr]")
{
    variables_check("C(a, a)", {"a"});
}

TEST_CASE("variables_check 2.1", "[variables_check_formula_mgr]")
{
    variables_check("C(axxa, axxa)", {"axxa"});
}

TEST_CASE("variables_check 2.2", "[variables_check_formula_mgr]")
{
    variables_check("<=(axxa, axxa)", {"axxa"}, formula_mgr::formula_refiners::none);
    variables_check("<=(axxa, axxa)", {}); // <=(X,X) -> T
}

TEST_CASE("variables_check 2.3", "[variables_check_formula_mgr]")
{
    variables_check("C(a, b)", {"a", "b"});
}

TEST_CASE("variables_check 3", "[variables_check_formula_mgr]")
{
    variables_check("C(a, b) & ~C(a,b)", {"a", "b"});
}

TEST_CASE("variables_check 4", "[variables_check_formula_mgr]")
{
    variables_check("(C(a, b) | <=(a, b)) | ~C(a,b)", {"a", "b"});
}

TEST_CASE("variables_check 5", "[variables_check_formula_mgr]")
{
    variables_check("(C(a, b) | <=(a, b)) | (~C(a,b) & <=(a,b))", {"a", "b"});
}

TEST_CASE("variables_check 6", "[variables_check_formula_mgr]")
{
    variables_check("C(a, b) & ~C(a, c)", {"a", "b", "c"});
}

TEST_CASE("variables_check 7", "[variables_check_formula_mgr]")
{
    variables_check("(C(a, b) | ~C(a, b)) & (C(a,b) | ~C(a,b))", {"a", "b"});
}

TEST_CASE("variables_check 8", "[variables_check_formula_mgr]")
{
    variables_check("(C(a, b) | ~C(a, b)) & (C(a,b) | ~C(a,c))", {"a", "b", "c"});
}

TEST_CASE("variables_check 9", "[variables_check_formula_mgr]")
{
    variables_check("C(a, -b) & ~C(a, b + h)", {"a", "b", "h"});
}

TEST_CASE("variables_check 10", "[variables_check_formula_mgr]")
{
    variables_check("C(a * c, (-b) + h) & ~C(a + c, (-b) + h)", {"a", "b", "c", "h"});
}

TEST_CASE("variables_check 11", "[variables_check_formula_mgr]")
{
    variables_check("(C(a, b) & (C(f, h) & C(j, k))) & ~((C(a, b) & C(f, h)) & C(j, k))",
                    {"a", "b", "f", "h", "j", "k"});
}

TEST_CASE("variables_check 12", "[variables_check_formula_mgr]")
{
    variables_check("(C(a, b) & (C(f, h) & C(j, k))) & ~((C(a, b) & C(f, h)) & C(j, Fxa))",
                    {"a", "b", "f", "h", "j", "k", "Fxa"});
}
