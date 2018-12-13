#include "GridFiller.h"

GridFillerNxN::GridFillerNxN(int n) : n(n) {}

void GridFillerNxN::fill() {
	for (int j = 0; j < n; j++) {
		for (int i = 0; i < n; i++) {
			fill_cell(i, j);
		}
	}
}