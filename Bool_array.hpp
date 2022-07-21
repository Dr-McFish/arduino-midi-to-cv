#pragma once

#include <stdint.h>

#define BOOL_ARRAY_SIZE 4 /* 4*16 = 64 bits */

class Bool_array 
{
public:
	Bool_array ();
	bool operator[](int idx);
	void set(int idx, bool value);
private:
	uint16_t p_bools[BOOL_ARRAY_SIZE];
};
