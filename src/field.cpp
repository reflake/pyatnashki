#include "field.h"

#include <algorithm>
#include <iostream>
#include <stdexcept>

using std::min;
using std::max;
using std::swap;
using std::runtime_error;

Field::Field() : size(0) {}

Field::Field(int size) : size(size), state(size * size)
{
	for (int i = 0; i < size * size; i++)
	{
		state[i] = i;
	}
}

void Field::Detach(int i, int j)
{
	if (size <= 0)
		return;

	state[i * size + j] = Field::empty_cell;
}

bool Field::IsAssembled()
{
	if (size <= 0)
		return false;

	for (int i = 0; i < size * size; i++)
		if (i != state[i] && state[i] != Field::empty_cell)
			return false;

	return true;
}

void Field::Turn(int &i, int &j, int h, int v)
{
	if (size <= 0)
		return;

	if (h == 0 && v == 0)
		return;

	if (h != 0 && v != 0)
		throw new runtime_error("Cannot move tile diagonally!");

	int newI = min(max(i + v, 0), size - 1);
	int newJ = min(max(j + h, 0), size - 1);

	if (i == newI && j == newJ)
		return;
		//throw new std::exception("Cannot move to the same cell!");

	// press right
	swap(state[i * size + j], state[newI * size + newJ]);

	i = newI;
	j = newJ;
}