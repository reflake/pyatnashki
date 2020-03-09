#pragma once

class Field
{
public:

	Field(int size);

	void Detach(int i, int j);

	void Turn(int &i, int &j, int h, int v);

	bool IsAssembled();

	int** GetState() const { return state; }
	int GetSize() const { return size; }

	const static int empty_cell = -1;

private:

	int size;
	int** state;
};