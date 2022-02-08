#ifndef POLY_H_
#define POLY_H_
/*
 *      Poly.h - polynomials
 *
 *      Copyright (C) 2022
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */

/* ---------------------------------------------------------------------- */
class Poly {
 private:
  static const bool debug = false;
  double * coefficients;
  float * externalCoefficients;
  int N;
  double * getInternalCoefficients();
  Poly(double * coefficients, int size);

 public:
  static Poly *  multiply(Poly * a, Poly * b);
  static Poly *  add(Poly * a, Poly * b);
  static Poly *  power(Poly * a, int n);
  static Poly * integrate(Poly * a, float value, float x);
  static float evaluate(Poly * a, float x);
  float * getCoefficients();
  int getSize();
  Poly(float * coefficients, int size);
  ~Poly(void);
};
#endif  // POLY_H_
