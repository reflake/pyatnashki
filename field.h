#pragma once

#include <vector>
class Field
{
public:

	Field();
	Field(int size);

	void Detach(int i, int j);

	void Turn(int &i, int &j, int h, int v);

	bool IsAssembled();

	const int* GetState() const { return state.data(); }
	int GetSize() const { return size; }

	const static int empty_cell = -1;

private:

	int size;
	std::vector<int> state;
};