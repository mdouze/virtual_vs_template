

#include "tv.h"


// #define M2 10
int TVI::f(int accu2) {
    for (int i = 0; i < M2; i++) {
        accu2 += accu2 | i;
    }  
    return accu2;
}




