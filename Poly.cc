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
  this->coefficients = reinterpret_cast<float *>(malloc(size * sizeof(float)));
  N = size;
  memcpy(this->coefficients, coefficients, size * sizeof(float));
}
/* ---------------------------------------------------------------------- */
Poly * Poly::multiply(Poly * a, Poly * b) {
  int newN = a->getSize() + b->getSize() - 1;
  float * newCoefficients = reinterpret_cast<float *>(malloc(newN * sizeof(float)));
  for (int i = 0; i < newN; i++) {
    newCoefficients[i] = 0.0;
  }
  int shift = 0;
  for (int i = 0; i < b->getSize(); i++) {
    for (int j = 0; j < a->getSize(); j++) {
      newCoefficients[shift + j] += (a->getCoefficients())[j] * (b->getCoefficients()[i]);
    }
    shift++;
  }
  Poly * newPoly = new Poly(newCoefficients, newN);
  fprintf(stderr, "Freeing newCoefficients at location %p in multiply\n", newCoefficients);
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
  float * newCoefficients = reinterpret_cast<float *>(malloc(largestTerm * sizeof(float)));
  for (int i = 0; i < largestTerm; i++) {
    if (i <= smallestTerm) {
      newCoefficients[i] = (a->getCoefficients())[i] + (b->getCoefficients())[i];
    } else {
      if (aIsBigger) {
        newCoefficients[i] = (a->getCoefficients())[i];
      } else {
        newCoefficients[i] = (b->getCoefficients())[i];
      }
    }
  }
  Poly * newPoly = new Poly(newCoefficients, largestTerm);
  fprintf(stderr, "Freeing newCoefficients at location %p in add\n", newCoefficients);
  free(newCoefficients);
  newCoefficients = 0;
  return(newPoly);
}
/* ---------------------------------------------------------------------- */
Poly * Poly::power(Poly * a, int N) {
  if (N < 1) {
    fprintf(stderr, "Only positive powers of polynomials are supported\n");
    exit(-1);
  }
  Poly * newPoly = 0;
  Poly * rPoly = 0;
  float * newCoefficients = 0;
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
float * Poly::getCoefficients() {
  return(coefficients);
}
/* ---------------------------------------------------------------------- */
int Poly::getSize() {
  return(N);
}
/* ---------------------------------------------------------------------- */
Poly::~Poly(void) {
  fprintf(stderr, "Deleting polynomial space @ %p and includes %p\n", this, coefficients);
  if (coefficients) {
    free(coefficients);
    coefficients = 0;
  } else {
    fprintf(stderr, "Something went wrong - there should be coefficient space\n");
  }
}

