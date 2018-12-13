#pragma once

class GridFillerNxN {
protected:
	const int n;
	virtual void fill_cell(int i, int j) = 0;

public:
	GridFillerNxN(int n);
	void fill();
};