/*
 *      Regression.cc - create an object that fits a line to a list of real numbers
 *
 *      Copyright (C) 2023
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */

#include <stdio.h>
#include "Regression.h"

/* ---------------------------------------------------------------------- */
Regression::Regression(std::list<float> input) {
  float sumX = 0.0;
  float sumY = 0.0;
  float sumXY = 0.0;
  float sumX2 = 0.0;
  int count = 0;
  // produce regression terms
  for (std::list<float>::iterator iter = input.begin(); iter != input.end(); iter++) {
    listCopy.push_back(*iter);
    sumY += *iter;
    sumX += count;
    sumXY += *iter * count;
    sumX2 += count * count;
    count++;
  }
  slope = (count * sumXY - sumX * sumY) / (count * sumX2 - sumX * sumX);
  yIntercept = sumY / count - slope * sumX / count;
  fprintf(stderr, "linear fit of list - slope: %7.2f, y-intercept: %7.2f\n", slope, yIntercept);
}
/* ---------------------------------------------------------------------- */
Regression::~Regression(void) {
  listCopy.clear();
}

