/*
 *      Test output - operator 2
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
  
  for (;;) {
    if (fread(&f, sizeof(float), 1, stdin) < 1) {
      break;
    }
    fprintf(stdout, "%f\n", f);
  }
  return 0;

}

/* ---------------------------------------------------------------------- */
