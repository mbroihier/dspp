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

  fprintf(stdout, "signal = [");  
  for (int i=0; i<512; i++) {
    fread(&f, sizeof(float), 1, stdin);
    fprintf(stdout, "%f ", f);
  }
  fprintf(stdout, "]\n");
  return 0;

}

/* ---------------------------------------------------------------------- */
