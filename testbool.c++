#include "Bool_array.hpp"
#include <stdio.h>

int main(){
	Bool_array testbool;

	for (int i = 0; i < BOOL_ARRAY_SIZE*16; i++)
	{
		printf("%1d", testbool[i]);
	}
	
	for (int i = 0; i < BOOL_ARRAY_SIZE*16; i++)
	{
		testbool.set(i, (i % 4) == 0);
	}

	putchar('\n');

	for (int i = 0; i < BOOL_ARRAY_SIZE*16; i++)
	{
		printf("%1d", testbool[i]);
	}
	putchar('\n');
}