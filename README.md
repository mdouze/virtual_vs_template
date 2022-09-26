


There are two ways of providing callbacks: 

- *virtual functions*:  where a parent class defines the function signature and sub-classes implement the virtual function. The advantage is that the code separation is clean, as the two parts of code can be compiled separately.

- *template classes*: the callback is left as a template argument and the compiler knows at compile time what is used. The advantage is that the compiler can optimize the callback and inline it. The disadvantage is that it is clumsy to write, slow to compile and that there is no code separation. 

Since the virtual function based approach is so much more easier to manage code-wise, it is worthwhile to figure out when the runtime impact of vitrual functions in manageable. 

## Test setup 

We assume the callback is called in a tight performance-critical loop (otherwise it's not worth considering the template solution anyway).

### Main loop 

The example we use here is:

```c++

int process(TV* tv) {
   int accu = 0;
   for (int i = 0; i < N; i++) {    // N large enough to get meaningful timings
      // loop workload 
      for (int j = 0; j < M; j++) { // M known at compile time
          accu += i ^ j;  // some arbitrary operation, not trivial to optimize out
      }
      // callback 
      accu = tv->f(accu);
   }
   return accu;
}
```

The rationale for this is that loop workload is really very cheap: it does not contain memory accesses and the arithmetic operations are 1-cycle (and will be vectorized if the hardware and compiler support it). `M` is used to tune the cost of the workload. 

The callback is written as a class method, which is generic enough for our purpose. 

### Callback  

The callback is quite similar to the main loop. 
```c++
  int f(int accu2) {
      for (int i = 0; i < M2; i++) {
         accu2 += accu2 | i;
      }
      return accu2;
   }
```
`M2` is a compile-time parameter that is used to tune the cost of the callback.

### Virtual function implementation

The virtual function version is straightforward: 
```c++

struct TV {
   virtual int f(int i) = 0;
};

struct TVI: TV {
   int f(int i) override;
};


   // how to call 
   TVI tvi; 
   int cs = process(&tvi); 

```

### Template implementation

For templates there is no notion of abstract function, everything is resolved at compile time: 

```c++
struct TVt {   
   int f(int accu2) {
...
   } 
};


template<class TV> 
int process(TV * tv) {
...
}

   // how to call (syntactically same as virtual function...)
   TVt tvt; 
   int cs = process(&tvt); 

```

## Results

Try several values of `N=`1M and measure the wall-clock time of the two variants for several values of `M`. 
We also fix `M2`=8, which is relatively small, the assumption being that it is sufficient to tune `M` only. 
Experiments are repeated 6 times, ignoring the first run (considered as warmup). 

We report the time with templates compared to with virtual functions as a percentage, ie. a negative percentage means that the template version is faster by this percentage.

On a Mac M2 (clang version 13.1.6): 

| N    | M=1  | M=4  | M=16  | M=64  | M=256 |
|------|------|------|-------|-------|-------| 
| 1M   | -9.82 % | -8.01 % | -5.52 % | -7.31 % | -1.82 % | 
| 5M   | -1.69 % | -0.13 % | +0.10 % | +0.03 % | -0.08 % | 
| 10M  | +0.18 % | +0.01 % | -0.01 % | +0.06 % | -0.07 % | 

The full results, including standard deviations are in [this gist](https://gist.github.com/mdouze/c2074f69228173175901a91300586cf1) 

Observations: 

- the measurements are a bit noisy for the N=1M range. 

- templates have an edge only on small workloads (M small) and this edge vanishes when the number of loop iterations increases (N high), 

- in fact, the number of loop iterations has more impact than the "weight" of the loop body, presumably because the CPU has some sophisticated optimization that kicks in only beyond 1M iterations.

Same for a Xeon E5-2698 v4, gcc 9.3.0: [this gist](https://gist.github.com/ba732bec0b7806ba064af37b46eec90a)

| N    | M=1  | M=4  | M=16  | M=64  | M=256 |
|------|------|------|-------|-------|-------| 
| 1M   | -16.35 % | -6.36 % | -3.30 % | -6.44 % | +0.86 % | 
| 5M   | -18.35 % | -3.42 % | -2.67 % | -3.82 % | +0.81 % | 
| 10M  | -17.79 % | -6.66 % | -0.41 % | -9.82 % | -0.39 % | 

This is less monotonous. The small workload case (M=1) is significantly more expensive than other options, however increasing the number of iterations does not have a strong impact. 

## Conclusions 

For small workloads, the performance impact of virtual function calls is indeed significant. However, these experiments are with a very light workload compared with inner loops that depend on memory reads at possibly random addresses. 

For the M2, the gap decreases when the number of loop cycles increases.  

## Test setup with look-up tables 

To make a more realistic workload, we use a callback function that computes product quantizer distances.
In that case, the callback accesses `M2` bytes of contiguous memory and does `M2` look-ups at random locations in a memory block that is presumably in cache. 

This writes as 

```c++ 
float TVI::f(const uint8_t *codes) {
  const float *luti = lut.data();
  float accu = 0; 
  for(int i = 0; i < M2; i++) {
    accu += luti[codes[i]];
    luti += 256;
  }
  return accu; 
}
```

This operation is strongly memory bound on normal CPUs. 
The calling loop just measures the max of the results over a contiguous set of random codes. 

```c++ 
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
```
Note that there is no `M` value anymore. 

### Results 

On the M2 platform. 
Full logs in [this gist](https://gist.github.com/mdouze/061af9b38450d86d3609ea8e1d078812) 

| N    | M2=1  | M2=2  | M2=4  | M2=8  | M2=16  | M2=32  | M2=64  | M2=128  |
|------|------|------|-------|-------|-------|------|------|-------|
| 1M   | -5.17 % | -7.08 % | -3.32 % | -41.83 % | -67.69 % | -1.53 % | -0.04 % | -0.93 % |
| 10M  | -1.04 % | -0.17 % | -0.72 % | -41.18 % | -68.31 % | -1.21 % | -0.79 % | -0.95 % | 

Observations: 

- the runtimes do not depend significantly on `N`

- there is a huge difference depending on `M2`, which is not monotonous and not necessarily in favor of the template version... 

So this is quite hard to interpret. 

On the Xeon platform ([gist](https://gist.github.com/mdouze/702cca5cfb44dd6e2a1e3795b5d010e3)) 

| N    | M2=1  | M2=2  | M2=4  | M2=8  | M2=16  | M2=32  | M2=64  | M2=128  |
|------|------|------|-------|-------|-------|------|------|-------|
| 1M   | -57.09 % | -54.09 % | -58.57 % | -72.86 % | -85.09 % | -7.12 % | +0.14 % | -0.27 % | 
| 10M  | -59.29 % | -59.39 % | -61.38 % | -72.92 % | -86.11 % | -6.83 % | +1.45 % | -0.48 % | 

For the Xeon, the template version is much faster, when M2 < 32. 
For M2 >= 32 there is a sudden performance drop, presumably because the LUT does not fit in cache anymore. 

There is a gcc options, `-flto` that enables link-time optimization. In that case it should be possible to optimize between the two compile units. 
Results ([gist](https://gist.github.com/mdouze/4c1741d3e04aeada5753fff1c90023c3))

| N    | M2=1  | M2=2  | M2=4  | M2=8  | M2=16  | M2=32  | M2=64  | M2=128  |
|------|------|------|-------|-------|-------|------|------|-------|
| 1M   | +2.87 % | +1.77 % | -7.25 % | -0.95 % | +6.69 % | +13.74 % | -0.92 % | +0.06 % | 
| 10M  | +10.49 % | +0.45 % | -8.17 % | -7.76 % | +0.02 % | +13.42 % | -0.21 % | -0.13 % | 

Here it is obvious that the problem is largely fixed by the link-time optimization.

