#include "Bool_array.hpp"
#include <stdio.h>

int main(){
	BoolArray testbool;

	for (int i = 0; i < BOOL_ARRAY_SIZE*16; i++)
	{
		printf("%1d", testbool[i]);
	}
	
	for (int i = 0; i < BOOL_ARRAY_SIZE*16; i++)
	{
		testbool.setb(i);
	}

	putchar('\n');

	for (int i = 0; i < BOOL_ARRAY_SIZE*16; i++)
	{
		printf("%1d", testbool[i]);
	}
	putchar('\n');
}