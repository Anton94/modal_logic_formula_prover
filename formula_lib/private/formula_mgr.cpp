#include "formula_mgr.h"
#include "logger.h"
#include "utils.h"
#include "parser_API.h"
#include "visitor.h"

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

auto formula_mgr::build(const std::string& f, const formula_refiners& refiners_flags) -> bool
{
    clear();

    info() << "Start building a formula:\n" << f;

    parser_error_info error_info;
    auto formula_AST = parse_from_input_string(f.c_str(), error_info);
    if(!formula_AST)
    {
        std::stringstream error_msg;
        error_info.printer(f, error_msg);
        error() << "\n" << error_msg.str();
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
    if(has_flag(refiners_flags, formula_refiners::convert_contact_less_eq_with_same_terms))
    {
        VConvertLessEqContactWithEqualTerms convertor_lessEq_contact_with_equal_terms;
        formula_AST->accept(convertor_lessEq_contact_with_equal_terms);
        info_buff << "Converted C(a,a);<=(a,a)  : ";
        formula_AST->accept(printer);
        info_buff << "\n";
    }

    if(has_flag(refiners_flags, formula_refiners::convert_disjunction_in_contact_less_eq))
    {
        VSplitDisjInLessEqAndContacts disj_in_contact_splitter;
        formula_AST->accept(disj_in_contact_splitter);
        info_buff << "C(a+b,c)->C(a,c)|C(b,c) ;\n";
        info_buff << "<=(a+b,c)-><=(a,c)&<=(b,c): ";
        formula_AST->accept(printer);
        info_buff << "\n";
    }

    if(has_flag(refiners_flags, formula_refiners::convert_contact_less_eq_with_same_terms))
    {
        VConvertLessEqContactWithEqualTerms convertor_lessEq_contact_with_equal_terms;
        formula_AST->accept(convertor_lessEq_contact_with_equal_terms);
        info_buff << "Converted C(a,a);<=(a,a)  : ";
        formula_AST->accept(printer);
        info_buff << "\n";
    }

    VConvertLessEqToEqZero eq_zero_convertor;
    formula_AST->accept(eq_zero_convertor);
    info_buff << "Converted (<= =0) formula : ";
    formula_AST->accept(printer);
    info_buff << "\n";

    if(has_flag(refiners_flags, formula_refiners::reduce_constants))
    {
        VReduceConstants trivial_reducer;
        formula_AST->accept(trivial_reducer);
        info_buff << "Reduced constants         : ";
        formula_AST->accept(printer);
        info_buff << "\n";
    }

    if(has_flag(refiners_flags, formula_refiners::reduce_contacts_less_eq_with_constants))
    {
        VConvertContactsWithConstantTerms contacts_with_constant_as_term_convertor;
        formula_AST->accept(contacts_with_constant_as_term_convertor);
        info_buff << "Converted C(a,1)->~(a=0)  : ";
        formula_AST->accept(printer);
        info_buff << "\n";
    }

    if(has_flag(refiners_flags, formula_refiners::remove_double_negation))
    {
        VReduceDoubleNegation double_negation_reducer;
        formula_AST->accept(double_negation_reducer);
        info_buff << "Reduced double negation   : ";
        formula_AST->accept(printer);
        info_buff << "\n";
    }

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

auto formula_mgr::brute_force_evaluate_with_points_count(basic_bruteforce_model& out_model) const -> bool
{
    return out_model.create(f_, variables_.size(), this);
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

    // TODO: what about the system of inequalities?
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

auto formula_mgr::has_flag(const formula_refiners& flags, const formula_refiners& flag) const -> bool
{
    return static_cast<int32_t>(flags) & static_cast<int32_t>(flag);
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
