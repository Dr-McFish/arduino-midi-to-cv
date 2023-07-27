#pragma once

#include <stdint.h>

#define BOOL_ARRAY_SIZE_BITS 128

typedef uint16_t bool_unit; //either uint16_t or uint8_t. unsure

#define BOOL_ARRAY_SIZE (BOOL_ARRAY_SIZE_BITS/sizeof(bool_unit))

class BoolArray 
{
public:
	BoolArray ();
	bool operator[](int idx);
	void setb(int idx);
	void clearb(int idx);
	void clear_all();
	int lowest();
	int highest();
private:
	bool_unit p_bools[BOOL_ARRAY_SIZE];
};
