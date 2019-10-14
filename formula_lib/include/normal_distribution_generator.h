#pragma once

#include "../private/types.h"

#include <vector>

class normal_distribution_generator
{
public:
    normal_distribution_generator(std::vector<variables_evaluations_t>& current);

    bool has_next(std::vector<variables_evaluations_t>& current) const;
    bool next(std::vector<variables_evaluations_t>& current);

    // private:
    void set_bits(std::vector<variables_evaluations_t>& current, int start, int end, bool value);
    bool should_increase_ones(std::vector<variables_evaluations_t>& current) const;
    bool all(std::vector<variables_evaluations_t>& current) const;

private:
    int ones;
    int index;
    int size;
    int W;
    // std::vector<variables_evaluations_t>& current;
};
