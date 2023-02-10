#ifndef REGRESSION_H_
#define REGRESSION_H_
/*
 *      Regression.h - Create objects that fit a line to a list of real numbers
 *
 *      Copyright (C) 2023
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */
#include <vector>
/* ---------------------------------------------------------------------- */
class Regression {
 private:
  std::vector<float> listCopy;
  float slope;
  float yIntercept;
  float minCentroid;
  float maxCentroid;
 public:
  float getSlope(void){ return slope; };
  float getYIntercept(void){ return yIntercept; };
  float getMinCentroid(void){ return minCentroid; };
  float getMaxCentroid(void){ return maxCentroid; };
  Regression(std::vector<float> input);
  ~Regression(void);
};
#endif  // REGRESSION_H_
