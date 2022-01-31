/*
 *      SFIRilter.cc - Smooth FIR filter class - derived from Hamming's book "Digital Filters" chapter 7
 *
 *      Copyright (C) 2022
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */

#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "SFIRFilter.h"

/* ---------------------------------------------------------------------- */
SFIRFilter::SFIRFilter(float cutoff) {
  doWork(cutoff, 1);
}
/* ---------------------------------------------------------------------- */
SFIRFilter::SFIRFilter(float cutoff, int decimation) {
  doWork(cutoff, decimation);
}
/* ---------------------------------------------------------------------- */
void SFIRFilter::doWork(float cutoff, int decimation) {
  // Determine K - K is the ratio between 1.0 and -1.0 where (p-q)/(p+q) will ideally be K.  p is the integer power
  // of the (1 + t) term of Hamming's smooth transfer function.  q is the integer power of the (1 - t) term of
  // Hamming's smooth transfer function => (1 + t)^p * (1 - t)^q = H
  int a;
  int b;
  float K = cos(M_PI * cutoff);
  // a/b is approximation of K
  if (fabs(K) < 0.1) {
    a = 0;
    b = 10;
  } else {
    if (K > 0.90) {
      a = 9;
      b = 10;
    } else {
      if (K < -0.90) {
        a = -9;
        b = 10;
      } else {
        a = 10;
        b = 10.0 / K + 0.5;
        if (b < 0) {
          a = -a;
          b = -b;
        }
      }
    }
  }
  // a/b is approximation of K
  int p = a + b;
  int q = b - a;
  fprintf(stderr, "input cutoff was %f, K was %f, a/b is %f, p is %d, and q is %d\n", cutoff, K,
          float(a)/float(b), p, q);
  // (1 + t)^p
  int M = p + q; // maximum power of polynomial
  double *polyCoefficients = (double *) malloc((M + 2) * sizeof(double));
  double *timesT = (double *) malloc((M + 2) * sizeof(double));
  for (int coef = 0; coef < M + 2; coef++) {
    polyCoefficients[coef] = 0.0;
    timesT[coef] = 0.0;
  }
  polyCoefficients[0] = 1.0;
  polyCoefficients[1] = 1.0;
  for (int power = 2; power <= p; power++) {
    // shift current coefficients by "t"
    fprintf(stderr, "working on power %d\n", power);
    for (int shift = 0; shift < power+1; shift++) {
      timesT[1 + shift] = polyCoefficients[shift];
    }
    for (int sum = 0; sum <= power; sum++) {
      polyCoefficients[sum] += timesT[sum];
      timesT[sum] = 0.0;
    }
    fprintf(stderr, "Polynomial coefficients (1+t)^%d:\n", power);
    for (int index = 0; index < M + 2; index++) {
      fprintf(stderr, "[%d]: %lf\n", index, polyCoefficients[index]);
    }
  }
  // (1 + t)^p * (1 - t)^q
  for (int power = p+1; power <= p + q; power++) {
    // shift current coefficients by "-t" (change sign)
    for (int shift = 0; shift <= power; shift++) {
      timesT[1 + shift] = -polyCoefficients[shift];
    }
    for (int sum = 0; sum <= power; sum++) {
      polyCoefficients[sum] += timesT[sum];
      timesT[sum] = 0.0;
    }
    fprintf(stderr, "Polynomial coefficients (1+t)^%d*(1-t)^%d:\n", p, power-p);
    for (int index = 0; index < M + 2; index++) {
      fprintf(stderr, "[%d]: %lf\n", index, polyCoefficients[index]);
    }
  }
  free(timesT);
  // we will now integrate the polynomial
  int scale = M + 1;
  int term = M;
  for (int integral = term; integral > 0; integral--) {
    for (int coef = 0; coef <= term; coef++) {
      if (integral == coef) {
        polyCoefficients[coef] /= float(scale);
      }
    }
    scale--;
  }
  fprintf(stderr, "Polynomial coefficients after integration:\n");
  for (int index = 0; index < M + 2; index++) {
    fprintf(stderr, "[%d]: %lf\n", index, polyCoefficients[index]);
  }
  // Determine the value of the polynomial at t = -1, the inverse of this is the constant of integration
  float integrationConstant = 0.0;
  for (int power = 1; power < M + 2; power++) {
    fprintf(stderr, "Summing %lf into integration constant\n", powf(-1.0, power)*polyCoefficients[power-1]);
    integrationConstant += powf(-1.0, power)*polyCoefficients[power-1];
  }
  for (int shift = M; shift >= 0; shift--) {
    polyCoefficients[shift + 1] = polyCoefficients[shift];
  }
  polyCoefficients[0] = - integrationConstant;
  fprintf(stderr, "Final polynomial coefficients:\n");
  for (int index = 0; index < M + 2; index++) {
    fprintf(stderr, "[%d]: %lf\n", index, polyCoefficients[index]);
  }
  fprintf(stderr, "Sum of coefficients:\n");
  double sumOfCoefficients = 0.0;
  for (int index = 0; index < M + 2; index++) {
    sumOfCoefficients += polyCoefficients[index];
  }
  fprintf(stderr, "%lf\n", sumOfCoefficients);

  // Now convert to Fourier coefficients

  double * accumulator = (double *) malloc((M + 2) * sizeof(double));
  double * split = (double *) malloc((M + 2) * sizeof(double));
  for (int index = 0; index < M + 2; index++) {
    accumulator[index] = 0.0;
  }
  accumulator[0] = polyCoefficients[M];
  accumulator[1] = polyCoefficients[M + 1];
  int accumulatorIndex = 1;
  //  while (accumulatorIndex <= M + 2) {
  while (accumulatorIndex <= M) {
    for (int index = 0; index < M + 2; index++) {
      split[index] = 0.0;
    }
    for (int index = 0; index <= accumulatorIndex; index++) {
      if (index == 0) {
        split[1] = accumulator[index];
      } else {
        fprintf(stderr, "spliting into index %d and %d\n", index - 1, index + 1);
        split[index - 1] += accumulator[index] / 2.0;
        split[index + 1] += accumulator[index] / 2.0;
      }
    }
    accumulatorIndex++;
    for (int index = 0; index <= accumulatorIndex; index++) {
      if (index == 0) {
        accumulator[index] = polyCoefficients[M + 1 - accumulatorIndex];
      } else {
        accumulator[index] = 0.0;
      }
      fprintf(stderr, "accumulating into index %d\n", index);
      accumulator[index] += split[index];
    }
    fprintf(stderr, "Accumulator contents:\n");
    for (int index = 0; index < M + 2; index++) {
      fprintf(stderr, "[%d]: %lf\n", index, accumulator[index]);
    }
  }
  free(polyCoefficients);
  free(split);
  double sumOfFourierCoefficients = 0;
  for (int index = 0; index < M + 2; index++) {
    sumOfFourierCoefficients += accumulator[index];
  }
  fprintf(stderr, "sumOfCoefficients %lf, sumOfFourierCoefficients %f\n", sumOfCoefficients, sumOfFourierCoefficients);
  fprintf(stderr, "allocating filterCoefficients buffer of size %d bytes\n", ((2 * (M + 1))) + 1 * sizeof(float)) ;
  float * filterCoefficients = (float *) malloc(((2 * (M + 1)) + 1) * sizeof(float));
  for (int delta = 0; delta <= M + 1; delta++) {
    if (delta == 0) {
      filterCoefficients[M+1] = accumulator[0] / sumOfFourierCoefficients ;
    } else {
      fprintf(stderr, "putting %d into index %d and %d\n", delta, M+delta+1, M-delta+1);
      filterCoefficients[M + delta + 1] = accumulator[delta] / 2.0 / sumOfFourierCoefficients;
      filterCoefficients[M - delta + 1] = accumulator[delta] / 2.0 / sumOfFourierCoefficients;
    }
  }
  free(accumulator);
  fprintf(stderr, "Filter Coefficients are:\n");
  for (int index = 0; index < 2*(M + 1) + 1; index++) {
    fprintf(stderr, "[%d]: %f\n", index, filterCoefficients[index]);
  }
  this->M = 2 * (M + 1) + 1; // set number of coefficients
  this->coefficients = filterCoefficients;  // free this in destructor
  this->decimation = decimation;
  N = 1024;  // default to 1K of output sample (I and Q)
  INPUT_BUFFER_SIZE = (N * decimation) * sizeof(float) * 2;
  SIGNAL_BUFFER_SIZE = INPUT_BUFFER_SIZE + (this->M - 1) * sizeof(float) * 2 * decimation; // always delay the last M samples for the next buffer read
  OUTPUT_BUFFER_SIZE = N * 2 * sizeof(float);
  fprintf(stderr, "allocating signal buffer of size %d bytes\n", SIGNAL_BUFFER_SIZE);
  signalBuffer = (float *) malloc(SIGNAL_BUFFER_SIZE);
  fprintf(stderr, "allocating output buffer of size %d bytes\n", OUTPUT_BUFFER_SIZE);
  outputBuffer = (float *) malloc(OUTPUT_BUFFER_SIZE);
  inputBuffer = signalBuffer + (this->M - 1)*2; // buffer start for read
  int diff = inputBuffer - signalBuffer;
  fprintf(stderr, "inputBuffer location: %p, signalBuffer location: %p, difference: %d\n",
          inputBuffer, signalBuffer, diff);
  inputToDelay = inputBuffer + (INPUT_BUFFER_SIZE - (this->M - 1)* sizeof(float) * 2 * decimation) / sizeof(float);

};

/* ---------------------------------------------------------------------- */
int SFIRFilter::readSignalPipe() {
  int count = fread(inputBuffer, sizeof(char), INPUT_BUFFER_SIZE, stdin);
  return count;
}

/* ---------------------------------------------------------------------- */
int SFIRFilter::writeSignalPipe() {
  int count = fwrite(outputBuffer, sizeof(char), OUTPUT_BUFFER_SIZE, stdout);
  return count;
}

/* ---------------------------------------------------------------------- */
void SFIRFilter::filterSignal(){

  float * I;
  float * firstI;
  float * Q;
  float * coefficientPtr;
  float * output;
  float sumI;
  float sumQ;


  for (;;) {
    if (readSignalPipe() != INPUT_BUFFER_SIZE) {
      fprintf(stderr, "Short read....\n");
      exit(-1);
    }

    I = signalBuffer;
                                                  // set I to the first input
                                                  // that will be processed by
                                                  // coefficient[0]
    Q = I + 1;
    output = outputBuffer;
    coefficientPtr = coefficients;
    int increment = 2 * decimation;
    firstI = I + increment;  // this is the next first I location
    //
    // Filter with this current buffer of input
    //
    for (int i = 0; i < N; i++) {  // for N outputs
      sumI = 0.0;
      sumQ = 0.0;
      coefficientPtr = coefficients;
      for (int j = 0; j < M; j++) {
        sumI += *coefficientPtr * *I;
        sumQ += *coefficientPtr * *Q;
        coefficientPtr++;
        I += increment;
        Q += increment;
      }
      I = firstI;
      firstI += increment;
      Q = I + 1;
      *output++ = sumI;
      *output++ = sumQ;
    }
    //
    // Copy the end of the buffer that will be used in the filtering
    // of the next buffer that arrives
    //
    writeSignalPipe();
    memcpy(signalBuffer, inputToDelay, (M-1) * 8 * decimation);
    // after copy, the end of the copied data should match up with the beginning of the input buffer, so
    // look at it after the read
  }
};

/* ---------------------------------------------------------------------- */
SFIRFilter::~SFIRFilter(void) {
  if (coefficients) free(coefficients);
  if (signalBuffer) free(signalBuffer);
  if (outputBuffer) free(outputBuffer);
};

