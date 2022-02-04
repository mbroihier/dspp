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
  float * coefficients;
  int N;

 public:
  static Poly *  multiply(Poly * a, Poly * b);
  static Poly *  add(Poly * a, Poly * b);
  static Poly *  power(Poly * a, int n);
  float * getCoefficients();
  int getSize();
  Poly(float * coefficients, int size);
  ~Poly(void);
};
#endif  // POLY_H_
