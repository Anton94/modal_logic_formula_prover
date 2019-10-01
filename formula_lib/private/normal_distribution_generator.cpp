#include "normal_distribution_generator.h"

normal_distribution_generator::normal_distribution_generator(std::vector<variables_evaluations_t>& current)
	:W(0)//: current(current), W(0)
{
	ones = 0;
	index = 0;
	size = current.size();
	if (size != 0) 
	{
		W = current[0].size();
	}
}

bool normal_distribution_generator::has_next(std::vector<variables_evaluations_t>& current) const
{
	for (int i = 0; i < size; ++i)
	{
		if (!current[i].all())
		{
			return true;
		}
	}

	return false;
}

bool normal_distribution_generator::next(std::vector<variables_evaluations_t>& current)
{
	if (all(current)) 
	{
		return false;
	}

	if (should_increase_ones(current))
	{
		ones++;
		set_bits(current, 0, size * W, false);
		set_bits(current, 0, ones, true);
	}
	else
	{
		int ones_passed = 0;
		int positions_passed = 0;
		for (int i = 0; i < size; ++i)
		{
			for (int j = 0; j < W; ++j)
			{
				positions_passed++;
				if (i == size - 1 && j == W - 1)
				{
					return true;
				}
				if (current[i][j])
				{
					bool next_bit = (j + 1 >= W) ? current[i + 1][0] : current[i][j + 1];
					if (!next_bit)
					{
						set_bits(current, 0, positions_passed, false);
						set_bits(current, 0, ones_passed, true);
						set_bits(current, positions_passed, positions_passed + 1, true);
						return true;
					}
					ones_passed++;
				}
			}
		}
	}

	return true;
}

bool normal_distribution_generator::should_increase_ones(std::vector<variables_evaluations_t>& current) const
{
	int ones_on_last_positions = 0;
	for (int i = size - 1; i >= 0; --i)
	{
		for (int j = W - 1; j >= 0; --j)
		{
			if (ones_on_last_positions++ >= ones)
			{
				return true;
			}
			if (!current[i][j])
			{
				return false;
			}
		}
	}

	return true;
}

bool normal_distribution_generator::all(std::vector<variables_evaluations_t>& current) const
{
	for (int i = 0; i < size; ++i)
	{
		if (!current[i].all())
		{
			return false;
		}
	}
	return true;
}

///
///  [0, 0, 0, 0]
///  [0, 0, 0, 0]
///  [0, 0, 0, 0]
///
/// W = 4, size = 3
///
/// set_bits(3, 10, 1) ->
///
///  [0, 0, 0, 1]
///  [1, 1, 1, 1]
///  [1, 1, 0, 0]
///
/// set_bits(9, 11, 0) ->
///  [0, 0, 0, 1]
///  [1, 1, 1, 1]
///  [1, 0, 0, 0]
///
void normal_distribution_generator::set_bits(std::vector<variables_evaluations_t>& current, int start, int end, bool value)
{
	int set_index = (start / W);
	int bit_index = (start % W);

	for (int i = set_index; i < size; ++i)
	{
		for (int j = bit_index; j < W; ++j)
		{
			if (start++ >= end) 
			{
				return;
			}
			bool cur = current[i][j];
			current[i][j] = value;
		}
		bit_index = 0;
	}
}
