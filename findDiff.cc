/*
 *      Difference between two real arrays
 *
 *      Copyright (C) 2019 
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */

#include <stdio.h>

/* ---------------------------------------------------------------------- */

int main(int argc, char *argv[]) {

  float fs, fu, delta;
  FILE * S;
  S = fopen("./signed.bin", "r");
  FILE * U;
  U = fopen("./unsigned.bin", "r");
 
  while (!feof(S)) {
    fread(&fs, sizeof(float), 1, S);
    fread(&fu, sizeof(float), 1, U);
    delta = fs - fu;
    fprintf(stdout, "fs: %f\n", fs);
    fprintf(stdout, "fu: %f\n", fu);
    fprintf(stdout, "Delta: %f\n", delta);
  }
  return 0;

}

/* ---------------------------------------------------------------------- */
