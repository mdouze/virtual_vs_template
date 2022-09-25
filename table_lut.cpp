
#include <random>


#include "table_lut.h"



TVI::TVI() {

  lut.resize(M2 * 256);
  std::random_device rd;
  std::mt19937 rng;
  std::uniform_real_distribution<> dis(0, 1);
  for(int i = 0; i < lut.size(); i++) {
    lut[i] = dis(rd); 
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



