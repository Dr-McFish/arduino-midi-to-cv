#include "bool_array.hpp"

BoolArray ::BoolArray (){
	for(int i = 0; i < BOOL_ARRAY_SIZE; i++)
		p_bools[i] = 0;
}

bool BoolArray::operator[](int idx) {
	return (p_bools[idx / sizeof(bool_unit)] >> (idx % sizeof(bool_unit))) & 1;
}

void BoolArray::set(int idx) {
	p_bools[idx / sizeof(bool_unit)] |= 1 << (idx % sizeof(bool_unit));
}

void BoolArray::clear(int idx) {
  p_bools[idx / sizeof(bool_unit)] &= ~(1 << (idx % sizeof(bool_unit)));
}

int BoolArray::lowest() {
  int rt = -1;
  for(int i = 0; i < BOOL_ARRAY_SIZE; i++){
    for(int j = 0; notes[i] >> j; j++){
      rt = i*j;
    }
  }

  return rt;
}

int BoolArray::highest(){
  int rt = -1;
  for(int i = BOOL_ARRAY_SIZE - 1; i >= 0; i--){
    for(int j = 0; notes[i] >> j; j++){
      rt = i*(sizeof(bool_unit) - j);
    }
  }

  return rt;
}
