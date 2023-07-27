#include "bool_array.hpp"

BoolArray ::BoolArray (){
	for(int i = 0; i < BOOL_ARRAY_SIZE; i++)
		p_bools[i] = 0;
}

bool BoolArray::operator[](int idx) {
	return (p_bools[idx / sizeof(bool_unit)] >> (idx % sizeof(bool_unit))) & 1;
}

void BoolArray::setb(int idx) {
	p_bools[idx / sizeof(bool_unit)] |= 1 << (idx % sizeof(bool_unit));
}

void BoolArray::clearb(int idx) {
	p_bools[idx / sizeof(bool_unit)] &= ~(1 << (idx % sizeof(bool_unit)));
}

void BoolArray::clear_all() {
	for(int i = 0; i < BOOL_ARRAY_SIZE; i++)
		p_bools[i] = 0;
}

int BoolArray::highest() {
	int rt = -1;
	for(int i = 0; i < BOOL_ARRAY_SIZE; i++){
		for(int j = 0; p_bools[i] >> j; j++){
			rt = i*sizeof(bool_unit) +j;
		}
	}
	// for(int i = 0; i < BOOL_ARRAY_SIZE_BITS; i++) {
	// 	if((*this)[i]) {
	// 		rt = i;
	// 	}
	// }

	return rt;
}

int BoolArray::lowest(){
	int rt = -1;

	for(int i = BOOL_ARRAY_SIZE_BITS - 1; i >= 0; i--) {
		if((*this)[i]) {
			rt = i;
		}
	}

	return rt;
}
