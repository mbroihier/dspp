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

  Poly *  multiply(Poly * a, Poly * b);
  Poly *  add(Poly * a, Poly * b);
  Poly *  power(Poly * a, int n);
  float * getCoefficients();
  int getSize();

  ~Poly(void);
    
};

