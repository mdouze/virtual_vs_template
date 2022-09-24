
#include <stdio.h>

#include <sys/time.h>
#include <unistd.h>

#include "tv.h"

double getmillisecs() {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return tv.tv_sec * 1e3 + tv.tv_usec * 1e-3;
}


#define N 1000000

/* 
#define M 100
#define M2 10
*/


int process_virtual(TV* tv) {
   int accu = 0;

   for (int i = 0; i < N; i++) {
      
      for (int j = 0; j < M; j++) {
          accu += i ^ j; 
      }
      accu = tv->f(accu); 
   }
   return accu; 
}


struct TVt {
    
   int f(int accu2) {
      for (int i = 0; i < M2; i++) {
         accu2 += accu2 | i;
      }
      return accu2;
   }

};



template<class TV> 
int process_template(TV * tv) {
   int accu = 0;

   for (int i = 0; i < N; i++) {
      
      for (int j = 0; j < M; j++) {
          accu += i ^ j; 
      }
      accu = tv->f(accu); 
   }
   return accu; 


}


int main () {
   for(int run = 0; run < 5; run++) {
   printf("Run %d\n", run); 
   {

      TVI tvi; 
      double t0 = getmillisecs();
      int cs = process_virtual(&tvi); 
      double t1 = getmillisecs(); 

      printf("virtual cs=%d, time=%.3f ms\n", cs, t1 - t0);
   }

   {

      TVt tvt; 
      double t0 = getmillisecs();
      int cs = process_template(&tvt); 
      double t1 = getmillisecs(); 

      printf("template cs=%d, time=%.3f ms\n", cs, t1 - t0);
   } 
   }
   return 0; 
}



