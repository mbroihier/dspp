/*
 *      Test output - operator 5
 *
 *      Copyright (C) 2019 
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */

#include <stdio.h>

/* ---------------------------------------------------------------------- */

int main(int argc, char *argv[]) {

  float f;
  int sample = 0;
  
  for (;;) {
    sample++;
    fprintf(stdout, "Sample: %i\n", sample);
    fread(&f, sizeof(float), 1, stdin);
    fprintf(stdout, "I: %f\n", f);
    fread(&f, sizeof(float), 1, stdin);
    fprintf(stdout, "Q: %f\n", f);
  }
  return 0;

}

/* ---------------------------------------------------------------------- */
