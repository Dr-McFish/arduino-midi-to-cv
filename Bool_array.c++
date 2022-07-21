#include "Bool_array.hpp"

Bool_array ::Bool_array (){
	for(int i = 0; i < BOOL_ARRAY_SIZE; i++)
		p_bools[i] = 0;
}

bool Bool_array::operator[](int idx) {
	if(idx < 16*BOOL_ARRAY_SIZE)
		return (p_bools[idx / 16] >> (idx % 16)) & 1;
	else
		return 3;
}

void Bool_array::set(int idx, bool value) {
	if(idx < 16*BOOL_ARRAY_SIZE){
		uint16_t byte2 = 1 << (idx % 16);
		if(value)
			p_bools[idx / 16] |= byte2;
		else
			p_bools[idx / 16] &= ~byte2;
	}
}