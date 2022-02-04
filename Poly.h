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

  Poly(float * coefficients, int size);

  static Poly *  multiply(Poly * a, Poly * b);
  static Poly *  add(Poly * a, Poly * b);
  static Poly *  power(Poly * a, int n);
  float * getCoefficients();
  int getSize();

  ~Poly(void);
    
};

