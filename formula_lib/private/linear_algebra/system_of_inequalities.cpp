#include "system_of_inequalities.h"
#include "impl/system_of_inequalities_impl.h"

#include <cassert>

system_of_inequalities::system_of_inequalities(size_t number_of_variables)
    : impl_(std::make_unique<system_of_inequalities_impl>(number_of_variables))
{
}

system_of_inequalities::~system_of_inequalities() = default;

system_of_inequalities::system_of_inequalities(system_of_inequalities&& rhs) noexcept
{
    impl_ = std::move(rhs.impl_);
}

system_of_inequalities& system_of_inequalities::operator=(system_of_inequalities&& rhs) noexcept
{
    if(this != &rhs)
    {
        impl_ = std::move(rhs.impl_);
    }
    return *this;
}

auto system_of_inequalities::add_constraint(const variables_set& lhs, const variables_set& rhs, inequality_operation op) -> bool
{
    assert(impl_);
    return impl_->add_constraint(lhs, rhs, op);
}

auto system_of_inequalities::is_solvable() const -> bool
{
    assert(impl_);
    return impl_->is_solvable();
}

auto system_of_inequalities::get_variables_values() const -> std::vector<double>
{
    assert(impl_);
    return impl_->get_variables_values();
}

void system_of_inequalities::print(std::ostream& out) const
{
    assert(impl_);
    return impl_->print(out);
}

void system_of_inequalities::clear()
{
    assert(impl_);
    impl_->clear();
}
