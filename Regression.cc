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
Regression::Regression(std::vector<float> input) {
  float sumX = 0.0;
  float sumY = 0.0;
  float sumXY = 0.0;
  float sumX2 = 0.0;
  float scale = 0.0;
  int count = 0;
  minCentroid = 0.0;
  maxCentroid = 0.0;
  if (input.size() > 1) {
    minCentroid = input[0];
    maxCentroid = input[0];
  }
  // produce regression terms
  for (auto entry : input) {
    listCopy.push_back(entry);
    sumY += entry;
    sumX += count;
    sumXY += entry * count;
    sumX2 += count * count;
    count++;
    if (entry > maxCentroid) {
      maxCentroid = entry;
    } else if (entry < minCentroid) {
      minCentroid = entry;
    }
  }
  slope = (count * sumXY - sumX * sumY) / (count * sumX2 - sumX * sumX);
  yIntercept = sumY / count - slope * sumX / count;
  if (minCentroid != maxCentroid) {
    scale = 255.0 / (maxCentroid - minCentroid);
  }
  //fprintf(stderr, "linear fit of list - slope: %7.5f, y-intercept: %7.2f, min: %7.2f, max: %7.2f, scale: %7.2f\n",
  //        slope, yIntercept, minCentroid, maxCentroid, scale);
}
/* ---------------------------------------------------------------------- */
Regression::~Regression(void) {
  listCopy.clear();
}

