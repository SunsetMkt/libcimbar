/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "util/vec_xy.h"
#include <cstddef>
#include <utility>
#include <vector>

class CellPositions
{
public:
	using coordinate = std::pair<int, int>;
	using positions_list = std::vector<coordinate>;

	static positions_list compute_linear(cimbar::vec_xy spacing, cimbar::vec_xy dimensions, int offset, cimbar::vec_xy marker_size);
	static positions_list compute(cimbar::vec_xy spacing, cimbar::vec_xy dimensions, int offset, cimbar::vec_xy marker_size, int interleave_blocks=0, int interleave_partitions=1);

public:
	CellPositions(cimbar::vec_xy spacing, cimbar::vec_xy dimensions, int offset, cimbar::vec_xy marker_size,int interleave_blocks=0, int interleave_partitions=1);

	unsigned index() const;
	size_t count() const;
	void reset();
	bool done() const;
	const coordinate& next();

	const positions_list& positions() const;

protected:
	unsigned _index;
	positions_list _positions;
};
