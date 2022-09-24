#include <vector>
#include <stdio.h>
#include <cmath>

#include <sys/time.h>
#include <unistd.h>

#include "tv.h"

double getmillisecs() {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return tv.tv_sec * 1e3 + tv.tv_usec * 1e-3;
}

/*
#define N 10000000

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

double mean(const double *x, int n) {
  double sum = 0;
  for(int i = 0; i < n; i++) {
    sum += x[i];
  }
  return sum / n;     
}

double stddev(const double *x, int n) {
  double sum = 0, sum2 = 0;
  for(int i = 0; i < n; i++) {
    sum += x[i];
    sum2 += x[i] * x[i]; 
  }
  return sqrt(sum2 / n - sum * sum / (n * n));     
}


int main () {
    int nrun = 6;
    std::vector<double> times(2 * nrun); 
    for(int run = 0; run < nrun; run++) {
     printf("Run %d\n", run); 
     {
       
       TVI tvi; 
       double t0 = getmillisecs();
       int cs = process_virtual(&tvi); 
       double t1 = getmillisecs(); 
       
       if (run != 0) {
	 printf("virtual cs=%d, time= %.3f ms\n", cs, t1 - t0);
       }
       times[run] = t1 - t0; 
     }
     
     {
       
       TVt tvt; 
       double t0 = getmillisecs();
       int cs = process_template(&tvt); 
       double t1 = getmillisecs(); 
       
       if (run != 0) {
	 printf("template cs=%d, time= %.3f ms\n", cs, t1 - t0);
       }
       times[run + nrun] = t1 - t0; 
       
     } 
   }
    double mean_virtual = mean(times.data() + 1, nrun - 1);
    double mean_template = mean(times.data() + 1 + nrun, nrun - 1);

    printf("virtual time: %.3f +/- %.3f\n",
	   mean_virtual,
	   stddev(times.data() + 1, nrun - 1)); 
    printf("template time: %.3f +/- %.3f (%+.2f %%)\n",
	   mean_template,
	   stddev(times.data() + 1 + nrun, nrun - 1),
	   (mean_template - mean_virtual) * 100.0 / mean_virtual
	   ); 
   return 0; 
}





