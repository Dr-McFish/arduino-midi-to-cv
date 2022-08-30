#pragma once

#include <stdint.h>

#define BOOL_ARRAY_SIZE_BITS 64

typedef uint16_t bool_unit; //either uint16_t or uint8_t. unsure

#define BOOL_ARRAY_SIZE (BOOL_ARRAY_SIZE_BITS/sizeof(bool_unit))

class BoolArray 
{
public:
	BoolArray ();
	bool operator[](int idx);
	void set(int idx);
  void clear(int idx);
  int lowest();
  int highest();
private:
	bool_unit p_bools[BOOL_ARRAY_SIZE];
};
