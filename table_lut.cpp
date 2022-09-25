
#include <random>


#include "table_lut.h"



TVI::TVI() {

  lut.resize(M2 * 256);
  std::mt19937 rng(123);
  for(int i = 0; i < lut.size(); i++) {
    lut[i] = rng() / float(rng.max()); 
  }   
  
};
  
float TVI::f(const uint8_t *codes) {
  const float *luti = lut.data();
  float accu = 0; 
  for(int i = 0; i < M2; i++) {
    accu += luti[codes[i]];
    luti += 256;
  }
  return accu; 
}



