#include <vector>
#include <random>
#include <stdio.h>
#include <cmath>

#include <sys/time.h>
#include <unistd.h>

#include "table_lut.h"

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


float process_virtual(TV* tv, const uint8_t *codes) {
  float themax = 0;
    
  for (int i = 0; i < N; i++) {
    float v = tv->f(codes);
    if (v > themax) {
      themax = v;
    }     
  }
  return themax;
}


struct TVt {
  std::vector<float> lut; 
  
  TVt() {

    lut.resize(M2 * 256);
    std::random_device rd;
    std::mt19937 rng;
    std::uniform_real_distribution<> dis(0, 1);
    for(int i = 0; i < lut.size(); i++) {
      lut[i] = dis(rd); 
    }   
    
  };

  
   int f(const uint8_t *codes) {
     const float *luti = lut.data();
     float accu = 0; 
     for(int i = 0; i < M2; i++) {
       accu += luti[codes[i]];
       luti += 256;
     }
     return accu; 
   }

};



template<class TV> 
float process_template(TV* tv, const uint8_t *codes) {
  float themax = 0;
  
  for (int i = 0; i < N; i++) {
    float v = tv->f(codes);
    if (v > themax) {
      themax = v;
    }     
  }
  return themax;
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
  std::vector<uint8_t> codes(M2 * N);

  {
    std::random_device rd;
    std::mt19937 rng;
    std::uniform_int_distribution<int> dis(0, 255);
    for(int i = 0; i < codes.size(); i++) {
      codes[i] = dis(rd); 
    }
  }

  for(int run = 0; run < nrun; run++) {
    printf("Run %d\n", run); 
    {
      
      TVI tvi; 
      double t0 = getmillisecs();
      float cs = process_virtual(&tvi, codes.data()); 
      double t1 = getmillisecs(); 
      
      if (run != 0) {
	printf("virtual cs=%g, time= %.3f ms\n", cs, t1 - t0);
      }
      times[run] = t1 - t0; 
    }

    {
      
      TVt tvt; 
      double t0 = getmillisecs();
      float cs = process_template(&tvt, codes.data()); 
      double t1 = getmillisecs(); 
      
      if (run != 0) {
	printf("template cs=%g, time= %.3f ms\n", cs, t1 - t0);
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





