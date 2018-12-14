#include "GridFiller.h"

GridFillerNxN::GridFillerNxN(int n) : n(n) {}

void GridFillerNxN::fill() {
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			fill_cell(i, j);
		}
	}
}