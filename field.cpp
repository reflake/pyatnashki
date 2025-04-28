#include "field.h"

#include <algorithm>
#include <stdexcept>

using std::min;
using std::max;
using std::swap;
using std::runtime_error;

Field::Field(int size) : size(size)
{
	state = new int* [size];

	for (int i = 0; i < size; i++)
	{
		state[i] = new int[size];

		for (int j = 0; j < size; j++)
		{
			state[i][j] = i * size + j;
		}
	}
}

void Field::Detach(int i, int j)
{
	state[i][j] = Field::empty_cell;
}

bool Field::IsAssembled()
{
	for (int i = 0; i < size; i++)
		for (int j = 0; j < size; j++)
			if (i * size + j != state[i][j] && state[i][j] != Field::empty_cell)
				return false;

	return true;
}

void Field::Turn(int &i, int &j, int h, int v)
{
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
	swap(state[i][j], state[newI][newJ]);

	i = newI;
	j = newJ;
}