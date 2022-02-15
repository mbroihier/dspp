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

#include "Poly.h"
#include "SFIRFilter.h"

/* ---------------------------------------------------------------------- */
SFIRFilter::SFIRFilter(float cutoff) {
  doWork(cutoff, 1, false, true);
}
/* ---------------------------------------------------------------------- */
SFIRFilter::SFIRFilter(float cutoff, int decimation) {
  doWork(cutoff, decimation, false, true);
}
/* ---------------------------------------------------------------------- */
SFIRFilter::SFIRFilter(float cutoff, int decimation, bool highPass) {
  doWork(cutoff, decimation, highPass, true);
}
/* ---------------------------------------------------------------------- */
SFIRFilter::SFIRFilter(float cutoff, int decimation, bool highPass, bool complexFilter) {
  doWork(cutoff, decimation, highPass, complexFilter);
}
/* ---------------------------------------------------------------------- */
void SFIRFilter::doWork(float cutoff, int decimation, bool highPass, bool complexFilter) {
  this->complexFilter = complexFilter;
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
  if (debug) fprintf(stderr, "input cutoff was %f, K was %f, a/b is %f, p is %d, and q is %d\n", cutoff, K,
                     static_cast<float>(a)/static_cast<float>(b), p, q);
  int M = p + q;  // maximum power of polynomial
  float *polyCoefficients = reinterpret_cast<float *>(malloc((M + 2) * sizeof(float)));
  // (1 + t)^p
  {
    float coef1[] = {1.0, 1.0};
    float coef2[] = {1.0, -1.0};
    Poly onePlusT(coef1, 2);
    Poly * onePlusTPowerP = Poly::power(&onePlusT, p);
    if (debug) {
      fprintf(stderr, "(1 + t)^p:\n");
      for (int index = 0; index < onePlusTPowerP->getSize(); index++) {
        fprintf(stderr, "[%d]: %f\n", index, onePlusTPowerP->getCoefficients()[index]);
      }
    }
    Poly oneMinusT(coef2, 2);
    Poly * oneMinusTPowerQ = Poly::power(&oneMinusT, q);
    if (debug) {
      fprintf(stderr, "(1 - t)^q:\n");
      for (int index = 0; index < oneMinusTPowerQ->getSize(); index++) {
        fprintf(stderr, "[%d]: %f\n", index, oneMinusTPowerQ->getCoefficients()[index]);
      }
    }
    Poly * functionOfT = Poly::multiply(onePlusTPowerP, oneMinusTPowerQ);
    if (debug) {
      fprintf(stderr, "(1 + t)^p*(1 - t)^q:\n");
      for (int index = 0; index < functionOfT->getSize(); index++) {
        fprintf(stderr, "[%d]: %f\n", index, functionOfT->getCoefficients()[index]);
      }
    }
    Poly * integral = Poly::integrate(functionOfT, 0.0, -1.0);
    memcpy(polyCoefficients, integral->getCoefficients(), sizeof(float) * integral->getSize());
    if (debug) fprintf(stderr, "Sum of integral coefficients: %f\n", Poly::evaluate(integral, 1.0));
    if (debug) fprintf(stderr, "Value of integral at -1: %f\n", Poly::evaluate(integral, -1.0));
    delete(onePlusTPowerP);
    delete(oneMinusTPowerQ);
    delete(functionOfT);
    delete(integral);
  }
  if (debug) {
    fprintf(stderr, "Final polynomial coefficients:\n");
    for (int index = 0; index < M + 2; index++) {
      fprintf(stderr, "[%d]: %f\n", index, polyCoefficients[index]);
    }
  }

  // Now convert to Fourier coefficients

  double * accumulator = reinterpret_cast<double *>(malloc((M + 2) * sizeof(double)));
  double * split = reinterpret_cast<double *>(malloc((M + 2) * sizeof(double)));
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
        if (debug) fprintf(stderr, "spliting into index %d and %d\n", index - 1, index + 1);
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
      if (debug) fprintf(stderr, "accumulating into index %d\n", index);
      accumulator[index] += split[index];
    }
    if (debug) {
      fprintf(stderr, "Accumulator contents:\n");
      for (int index = 0; index < M + 2; index++) {
        fprintf(stderr, "[%d]: %lf\n", index, accumulator[index]);
      }
    }
  }
  free(polyCoefficients);
  free(split);
  double sumOfFourierCoefficients = 0;
  for (int index = 0; index < M + 2; index++) {
    sumOfFourierCoefficients += accumulator[index];
  }

  if (debug)
    fprintf(stderr, "allocating filterCoefficients buffer of size %d bytes\n", ((2 * (M + 1))) + 1 * sizeof(float));
  float * filterCoefficients = reinterpret_cast<float *>(malloc(((2 * (M + 1)) + 1) * sizeof(float)));
  for (int delta = 0; delta <= M + 1; delta++) {
    if (delta == 0) {
      if (highPass) {
        filterCoefficients[M+1] = 1.0 - (accumulator[0] / sumOfFourierCoefficients);
      } else {
        filterCoefficients[M+1] = accumulator[0] / sumOfFourierCoefficients;
      }
    } else {
      if (debug)fprintf(stderr, "putting %d into index %d and %d\n", delta, M+delta+1, M-delta+1);
      if (highPass) {
        filterCoefficients[M + delta + 1] = - accumulator[delta] / 2.0 / sumOfFourierCoefficients;
        filterCoefficients[M - delta + 1] = - accumulator[delta] / 2.0 / sumOfFourierCoefficients;
      } else {
        filterCoefficients[M + delta + 1] = accumulator[delta] / 2.0 / sumOfFourierCoefficients;
        filterCoefficients[M - delta + 1] = accumulator[delta] / 2.0 / sumOfFourierCoefficients;
      }
    }
  }
  free(accumulator);
  if (debug) {
    fprintf(stderr, "Filter Coefficients are:\n");
    for (int index = 0; index < 2*(M + 1) + 1; index++) {
      fprintf(stderr, "[%d]: %f\n", index, filterCoefficients[index]);
    }
  }
  this->M = 2 * (M + 1) + 1;  // set number of coefficients
  this->coefficients = filterCoefficients;  // free this in destructor
  this->decimation = decimation;
  N = 1024;  // default to 1K of output sample (I and Q or real)
  if (complexFilter) {
    INPUT_BUFFER_SIZE = (N * decimation) * sizeof(float) * 2;
    // always delay the last M samples for the next buffer read
    SIGNAL_BUFFER_SIZE = INPUT_BUFFER_SIZE + (this->M - 1) * sizeof(float) * 2 * decimation;
    OUTPUT_BUFFER_SIZE = N * 2 * sizeof(float);
    if (debug) fprintf(stderr, "allocating signal buffer of size %d bytes\n", SIGNAL_BUFFER_SIZE);
    signalBuffer = reinterpret_cast<float *>(malloc(SIGNAL_BUFFER_SIZE));
    if (debug) fprintf(stderr, "allocating output buffer of size %d bytes\n", OUTPUT_BUFFER_SIZE);
    outputBuffer = reinterpret_cast<float *>(malloc(OUTPUT_BUFFER_SIZE));
    inputBuffer = signalBuffer + (this->M - 1) * 2 * decimation;  // buffer start for read
    int diff = inputBuffer - signalBuffer;
    if (debug)fprintf(stderr, "inputBuffer location: %p, signalBuffer location: %p, difference: %d\n",
                      inputBuffer, signalBuffer, diff);
    inputToDelay = inputBuffer + (INPUT_BUFFER_SIZE - (this->M - 1)* sizeof(float) * 2 * decimation) / sizeof(float);
  } else {
    INPUT_BUFFER_SIZE = (N * decimation) * sizeof(float);
    // always delay the last M samples for the next buffer read
    SIGNAL_BUFFER_SIZE = INPUT_BUFFER_SIZE + (this->M - 1) * sizeof(float) * decimation;
    OUTPUT_BUFFER_SIZE = N * sizeof(float);
    if (debug) fprintf(stderr, "allocating signal buffer of size %d bytes\n", SIGNAL_BUFFER_SIZE);
    signalBuffer = reinterpret_cast<float *>(malloc(SIGNAL_BUFFER_SIZE));
    if (debug) fprintf(stderr, "allocating output buffer of size %d bytes\n", OUTPUT_BUFFER_SIZE);
    outputBuffer = reinterpret_cast<float *>(malloc(OUTPUT_BUFFER_SIZE));
    inputBuffer = signalBuffer + (this->M - 1) * decimation;  // buffer start for read
    int diff = inputBuffer - signalBuffer;
    if (debug)fprintf(stderr, "inputBuffer location: %p, signalBuffer location: %p, difference: %d\n",
                      inputBuffer, signalBuffer, diff);
    inputToDelay = inputBuffer + (INPUT_BUFFER_SIZE - (this->M - 1)* sizeof(float) * decimation) / sizeof(float);
  }
}

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
void SFIRFilter::filterSignal() {
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

    if (complexFilter) {
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
    } else {
      I = signalBuffer;
      // set I to the first input
      // that will be processed by
      // coefficient[0]
      output = outputBuffer;
      coefficientPtr = coefficients;
      int increment = decimation;
      firstI = I + increment;  // this is the next first I location
      //
      // Filter with this current buffer of input
      //
      for (int i = 0; i < N; i++) {  // for N outputs
        sumI = 0.0;
        coefficientPtr = coefficients;
        for (int j = 0; j < M; j++) {
          sumI += *coefficientPtr * *I;
          coefficientPtr++;
          I += increment;
        }
        I = firstI;
        firstI += increment;
        *output++ = sumI;
      }
      //
      // Copy the end of the buffer that will be used in the filtering
      // of the next buffer that arrives
      //
      writeSignalPipe();
      memcpy(signalBuffer, inputToDelay, (M-1) * 4 * decimation);
      // after copy, the end of the copied data should match up with the beginning of the input buffer, so
      // look at it after the read
    }
  }
}

/* ---------------------------------------------------------------------- */
SFIRFilter::~SFIRFilter(void) {
  if (coefficients) free(coefficients);
  if (signalBuffer) free(signalBuffer);
  if (outputBuffer) free(outputBuffer);
}

