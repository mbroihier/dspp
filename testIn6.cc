/*
 *      Test input - operator 6
 *
 *      Copyright (C) 2019 
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */
#include <math.h>
#include <stdio.h>

/* ---------------------------------------------------------------------- */

int main(int argc, char *argv[]) {

  float I, Q;
  float k = 2.0 * M_PI * 5000.0;
  float t = 0.0;
  float deltaT = 1.0/48000.0;
  for (;;) {
    I = cos(k * t);
    Q = - sin(k * t);
    t += deltaT;
    //fprintf(stderr, "t: %f, I: %f, Q:%f\n", t, I, Q);
    fwrite(&I, sizeof(float), 1, stdout);
    fwrite(&Q, sizeof(float), 1, stdout);
    if (t >= 10.0) {
      t = 0.0;
      k *= 1.1;
    }
  }    
  return 0;

}

/* ---------------------------------------------------------------------- */
