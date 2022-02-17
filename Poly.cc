/*
 *      Poly.cc - Process polynomials
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

/* ---------------------------------------------------------------------- */
Poly::Poly(float * coefficients, int size) {
  if (size < 1) {
    fprintf(stderr, "Polynomials must be of order >= 0\n");
    exit(-1);
  }
  this->externalCoefficients = reinterpret_cast<float *>(malloc(size * sizeof(float)));
  this->coefficients = reinterpret_cast<double *>(malloc(size * sizeof(double)));
  N = size;
  memcpy(this->externalCoefficients, coefficients, size * sizeof(float));
  for (int i = 0; i < size; i++) {
    this->coefficients[i] = externalCoefficients[i];
  }
}
/* ---------------------------------------------------------------------- */
Poly::Poly(double * coefficients, int size) {
  if (size < 1) {
    fprintf(stderr, "Polynomials must be of order >= 0\n");
    exit(-1);
  }
  this->externalCoefficients = reinterpret_cast<float *>(malloc(size * sizeof(float)));
  this->coefficients = reinterpret_cast<double *>(malloc(size * sizeof(double)));
  N = size;
  memcpy(this->coefficients, coefficients, size * sizeof(double));
  for (int i = 0; i < size; i++) {
    externalCoefficients[i] = coefficients[i];
  }
}
/* ---------------------------------------------------------------------- */
Poly * Poly::multiply(Poly * a, Poly * b) {
  int newN = a->getSize() + b->getSize() - 1;
  double * newCoefficients = reinterpret_cast<double *>(malloc(newN * sizeof(double)));
  for (int i = 0; i < newN; i++) {
    newCoefficients[i] = 0.0;
  }
  int shift = 0;
  for (int i = 0; i < b->getSize(); i++) {
    for (int j = 0; j < a->getSize(); j++) {
      newCoefficients[shift + j] += a->getInternalCoefficients()[j] * b->getInternalCoefficients()[i];
    }
    shift++;
  }
  Poly * newPoly = new Poly(newCoefficients, newN);
  if (debug) fprintf(stderr, "Freeing newCoefficients at location %p in multiply\n", newCoefficients);
  free(newCoefficients);
  newCoefficients = 0;
  return(newPoly);
}
/* ---------------------------------------------------------------------- */
Poly * Poly::add(Poly * a, Poly * b) {
  int smallestTerm = 0;
  int largestTerm = 0;
  bool aIsBigger;
  if (a->getSize() < b->getSize()) {
    smallestTerm = a->getSize();
    largestTerm = b->getSize();
    aIsBigger = false;
  } else {
    smallestTerm = b->getSize();
    largestTerm = a->getSize();
    aIsBigger = true;
  }
  double * newCoefficients = reinterpret_cast<double *>(malloc(largestTerm * sizeof(double)));
  for (int i = 0; i < largestTerm; i++) {
    if (i <= smallestTerm) {
      newCoefficients[i] = (a->getInternalCoefficients())[i] + (b->getInternalCoefficients())[i];
    } else {
      if (aIsBigger) {
        newCoefficients[i] = (a->getInternalCoefficients())[i];
      } else {
        newCoefficients[i] = (b->getInternalCoefficients())[i];
      }
    }
  }
  Poly * newPoly = new Poly(newCoefficients, largestTerm);
  if (debug) fprintf(stderr, "Freeing newCoefficients at location %p in add\n", newCoefficients);
  free(newCoefficients);
  newCoefficients = 0;
  return(newPoly);
}
/* ---------------------------------------------------------------------- */
Poly * Poly::power(Poly * a, int N) {
  Poly * newPoly = 0;
  Poly * rPoly = 0;
  if (N < 1) {
    if (N == 0) {
      float polyConst[] = {1.0};
      newPoly = new Poly(polyConst, 1.0);
      return(newPoly);
    }
    fprintf(stderr, "Only positive powers of polynomials are supported\n");
    exit(-1);
  }
  if (N > 1) {
    rPoly = power(a, N - 1);
    newPoly = a->multiply(rPoly, a);
    delete(rPoly);
  } else {
    newPoly = new Poly(a->getCoefficients(), a->getSize());
  }
  return(newPoly);
}
/* ---------------------------------------------------------------------- */
float Poly::evaluate(Poly * a, float x) {
  double value = 0.0;
  int size = a->getSize() - 1;
  if (size > 1) {
    for (int i = size; i > 0; i--) {
      value = (a->getInternalCoefficients()[i] + value) * x;
      if (debug) fprintf(stderr, "evaluation loop - new value: %lf, power of X: %d\n", value, i);
    }
  }
  value += a->getInternalCoefficients()[0];
  return(static_cast<float>(value));
}
/* ---------------------------------------------------------------------- */
Poly * Poly::integrate(Poly * a, float value, float x) {
  int newSize = a->getSize() + 1;
  double * originalCoefficients = a->getInternalCoefficients();
  double * newCoefficients = reinterpret_cast<double *>(malloc(sizeof(double) * newSize));
  for (int i = 0; i < newSize; i++) {
    if (i == 0) {
      newCoefficients[i] = 0.0;
    } else {
      newCoefficients[i] = originalCoefficients[i - 1] / static_cast<double>(i);
    }
  }
  Poly * unevaluatedPoly = new Poly(newCoefficients, newSize);
  float constant = Poly::evaluate(unevaluatedPoly, x);
  newCoefficients[0] = value - constant;
  delete(unevaluatedPoly);
  return(new Poly(newCoefficients, newSize));
}
/* ---------------------------------------------------------------------- */
float * Poly::getCoefficients() {
  return(externalCoefficients);
}
/* ---------------------------------------------------------------------- */
double * Poly::getInternalCoefficients() {
  return(coefficients);
}
/* ---------------------------------------------------------------------- */
int Poly::getSize() {
  return(N);
}
/* ---------------------------------------------------------------------- */
Poly::~Poly(void) {
  if (debug) fprintf(stderr, "Deleting polynomial space @ %p and includes %p\n", this, coefficients);
  if (coefficients) {
    free(coefficients);
    coefficients = 0;
  } else {
    fprintf(stderr, "Something went wrong - there should be internal coefficient space\n");
  }
  if (externalCoefficients) {
    free(externalCoefficients);
    externalCoefficients = 0;
  } else {
    fprintf(stderr, "Something went wrong - there should be external coefficient space\n");
  }
}

