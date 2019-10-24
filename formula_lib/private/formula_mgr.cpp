#include "formula_mgr.h"
#include "logger.h"
#include "utils.h"
#include "parser_API.h"
#include "visitor.h"

namespace
{

using json = nlohmann::json;

auto get_all_variables(const json& f, variables_set_t& variables) -> bool
{
    if(!f.contains("name"))
    {
        error() << "Json (sub)formula is missing a 'name' field:\n" << f.dump(4);
        return false;
    }

    auto& name_field = f["name"];
    if(!name_field.is_string())
    {
        error() << "Json (sub)formula has 'name' field which is not a string:\n" << f.dump(4);
        return false;
    }

    auto op = name_field.get<std::string>();
    if(op == "T" || op == "F" || op == "1" || op == "0")
    {
        return true;
    }

    if(!f.contains("value"))
    {
        error() << "Json (sub)formula is missing a 'value' field:\n" << f.dump(4);
        return false;
    }
    auto& value_field = f["value"];

    if(op == "string") // variable
    {
        if(!value_field.is_string())
        {
            error() << "Json (sub)formula has 'value' field which is not a string:\n" << f.dump(4);
            return false;
        }
        variables.insert(value_field.get<std::string>());
        return true;
    }

    if(value_field.is_object())
    {
        return get_all_variables(value_field, variables);
    }
    if(value_field.is_array() && value_field.size() == 2)
    {
        return get_all_variables(value_field[0], variables) && get_all_variables(value_field[1], variables);
    }
    error()
        << "Json (sub)formula has 'value' field which is neither an object, nor an array of two objects:\n"
        << f.dump(4);
    return false;
}
}

formula_mgr::formula_mgr(const termination_callback& c)
    : f_(this)
	, is_terminated_(c)
{
}

formula_mgr::formula_mgr(formula_mgr&& rhs) noexcept
    : f_(this)
{
    move(std::move(rhs));
}

formula_mgr& formula_mgr::operator=(formula_mgr&& rhs) noexcept
{
    if(this != &rhs)
    {
        move(std::move(rhs));
    }
    return *this;
}

auto formula_mgr::build(const std::string& f) -> bool
{
    clear();

    info() << "Start building a formula: " << f;

    auto formula_AST = parse_from_input_string(f.c_str());
    if(!formula_AST)
    {
        error() << "Unable to parse the input formula";
        return false;
    }

    // TODO: more detailed logs
    std::stringstream info_buff;
    info_buff << "Parsed formula: ";
    VPrinter printer(info_buff);
    formula_AST->accept(printer);
    info_buff << "\n";

    VConvertImplicationEqualityToConjDisj convertor;
    formula_AST->accept(convertor);
    info_buff << "Converted (-> <->)        : ";
    formula_AST->accept(printer);
    info_buff << "\n";

    // TODO: consider making VSplitDisjInLessEqAndContacts and VSplitDisjInLessEqAndContacts combined
    // because in some intermediate splitting the two terms might match
    // Nevertheless, this will still be not 100% sufficient because the order of spliting might take a big role and
    // skip some pontential matches. For not just convert them after the splitting.
    VConvertLessEqContactWithEqualTerms convertor_lessEq_contact_with_equal_terms;
    formula_AST->accept(convertor_lessEq_contact_with_equal_terms);
    info_buff << "Converted C(a,a);<=(a,a)  : ";
    formula_AST->accept(printer);
    info_buff << "\n";

    VSplitDisjInLessEqAndContacts disj_in_contact_splitter;
    formula_AST->accept(disj_in_contact_splitter);
    info_buff << "C(a+b,c)->C(a,c)|C(b,c) ;\n";
    info_buff << "<=(a+b,c)-><=(a,c)&<=(b,c): ";
    formula_AST->accept(printer);
    info_buff << "\n";

    formula_AST->accept(convertor_lessEq_contact_with_equal_terms);
    info_buff << "Converted C(a,a);<=(a,a)  : ";
    formula_AST->accept(printer);
    info_buff << "\n";

    VConvertLessEqToEqZero eq_zero_convertor;
    formula_AST->accept(eq_zero_convertor);
    info_buff << "Converted (<= =0) formula : ";
    formula_AST->accept(printer);
    info_buff << "\n";

    VReduceConstants trivial_reducer;
    formula_AST->accept(trivial_reducer);
    info_buff << "Reduced constants         : ";
    formula_AST->accept(printer);
    info_buff << "\n";

    VConvertContactsWithConstantTerms contacts_with_constant_as_term_convertor;
    formula_AST->accept(contacts_with_constant_as_term_convertor);
    info_buff << "Converted C(a,1)->~(a=0)  : ";
    formula_AST->accept(printer);
    info_buff << "\n";

    VReduceDoubleNegation double_negation_reducer;
    formula_AST->accept(double_negation_reducer);
    info_buff << "Reduced double negation   : ";
    formula_AST->accept(printer);
    info_buff << "\n";

    info() << info_buff.str();

    // Will cash all variables and when building the formula tree we will use their ids instead of the heavy strings
    VVariablesGetter::variables_set_t variables;
    VVariablesGetter variables_getter(variables);
    formula_AST->accept(variables_getter);

    variables_.reserve(variables.size());
    variable_to_id_.reserve(variables.size());
    for(const auto& variable : variables)
    {
        variable_to_id_[variable] = variables_.size();
        variables_.emplace_back(variable);
    }

    return f_.build(*formula_AST, variable_to_id_);
}

auto formula_mgr::build(json& f) -> bool
{
    clear();

    info() << "Start building a formula";
    verbose() << f.dump(4);

    // Will cash all variables and swap the string representations with their id in the cache.
    variables_set_t variables_set;
    if(!get_all_variables(f, variables_set))
    {
        return false;
    }

    variables_.reserve(variables_set.size());
    variable_to_id_.reserve(variables_set.size());
    for(const auto& variable : variables_set)
    {
        variable_to_id_[variable] = variables_.size();
        variables_.emplace_back(variable);
    }

    return change_variables_to_variable_ids(f) && f_.build(f);
}

auto formula_mgr::brute_force_evaluate_with_points_count(basic_bruteforce_model& out_model) const -> bool
{
    return out_model.create(f_, variables_.size());
}

auto formula_mgr::is_satisfiable(imodel& out_model) -> bool
{
    info() << "Running satisfiability checking of " << f_ << "...";

    bool is_f_satisfiable{};
    const auto elapsed_time =
        time_measured_call([&]() { is_f_satisfiable = t_.is_satisfiable(f_, out_model); });

    info() << (is_f_satisfiable ? "Satisfiable" : "NOT Satisfiable") << ". "
           << "Took " << elapsed_time.count() << "ms.";

    if(is_f_satisfiable)
    {
        assert(is_model_satisfiable(out_model));
    }

    return is_f_satisfiable;
}

auto formula_mgr::is_model_satisfiable(const imodel& model) const -> bool
{
    info() << "Running satisfiability checking of " << f_ << " with provided model: \n" << model;

    const auto& model_variable_evaluations = model.get_variables_evaluations();
    const auto& model_contact_relations = model.get_contact_relations();

    return f_.evaluate(model_variable_evaluations, model_contact_relations);
}

void formula_mgr::clear()
{
    f_.clear();
    variable_to_id_.clear();
    variables_.clear();
}

auto formula_mgr::get_variables() const -> const variables_t&
{
    return variables_;
}

auto formula_mgr::get_variable(variable_id_t id) const -> std::string
{
    assert(id < variables_.size());
    return variables_[id];
}

auto formula_mgr::get_variable(const std::string& name) const -> variable_id_t
{
    auto it = variable_to_id_.find(name);
    if(it != variable_to_id_.end())
    {
        return it->second;
    }
    return variable_id_t(-1);
}

auto formula_mgr::get_internal_formula() const -> const formula*
{
    return &f_;
}

auto formula_mgr::print(std::ostream& out, const variables_evaluations_block& block) const -> std::ostream&
{
    const auto& variables = block.get_variables();
    const auto& evaluations = block.get_evaluations();
    for(size_t i = 0; i < variables.size(); ++i)
    {
        if(variables[i])
        {
            out << "<" << get_variable(i) << " : " << evaluations.test(i) << "> ";
        }
    }
    return out;
}

auto formula_mgr::print(std::ostream& out, const variables_mask_t& variables_mask) const -> std::ostream&
{
    for(size_t i = 0; i < variables_mask.size(); ++i)
    {
        if(variables_mask[i])
        {
            out << get_variable(i) << " ";
        }
    }
    return out;
}

auto formula_mgr::change_variables_to_variable_ids(json& f) const -> bool
{
    if(!f.contains("name"))
    {
        error() << "Json (sub)formula is missing a 'name' field:\n" << f.dump(4);
        return false;
    }

    auto& name_field = f["name"];
    if(!name_field.is_string())
    {
        error() << "Json (sub)formula has 'name' field which is not a string:\n" << f.dump(4);
        return false;
    }

    auto op = name_field.get<std::string>();
    if(op == "T" || op == "F" || op == "1" || op == "0")
    {
        return true;
    }

    if(!f.contains("value"))
    {
        error() << "Json (sub)formula is missing a 'value' field:\n" << f.dump(4);
        return false;
    }

    auto& value_field = f["value"];
    if(op == "string") // variable
    {
        if(!value_field.is_string())
        {
            error() << "Json (sub)formula has 'value' field which is not a string:\n" << f.dump(4);
            return false;
        }
        name_field = "variable_id";

        const auto variable_str = value_field.get<std::string>();
        assert(variable_to_id_.find(variable_str) != variable_to_id_.end());
        value_field = variable_to_id_.find(variable_str)->second;
        return true;
    }

    if(value_field.is_object())
    {
        return change_variables_to_variable_ids(value_field);
    }
    if(value_field.is_array() && value_field.size() == 2)
    {
        return change_variables_to_variable_ids(value_field[0]) &&
               change_variables_to_variable_ids(value_field[1]);
    }

    error() << "Json (sub)formula has 'value' field which is neither an object, nor an array of two "
               "objects:\n"
            << f.dump(4);
    return false;
}

void formula_mgr::terminate_if_need() const
{
	if (is_terminated_)
	{
		if (is_terminated_())
		{
			throw "The process was terminated";
		}
	}
}

void formula_mgr::move(formula_mgr&& rhs) noexcept
{
    variable_to_id_ = std::move(rhs.variable_to_id_);
    variables_ = std::move(rhs.variables_);
    f_ = std::move(rhs.f_);

    f_.change_formula_mgr(this);
}

std::ostream& operator<<(std::ostream& out, const formula_mgr& f)
{
    out << *f.get_internal_formula();
    return out;
}
