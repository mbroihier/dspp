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
#include <list>
/* ---------------------------------------------------------------------- */
class Regression {
 private:
  std::list<float> listCopy;
  float slope;
  float yIntercept;
 public:
  float getSlope(void){ return slope; };
  float getYIntercept(void){ return yIntercept; };
  Regression(std::list<float> input);
  ~Regression(void);
};
#endif  // REGRESSION_H_
