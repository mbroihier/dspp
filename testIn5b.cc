/*
 *      Test input - operator 5
 *
 *      Copyright (C) 2019 
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */

#include <stdio.h>

/* ---------------------------------------------------------------------- */

int main(int argc, char *argv[]) {

  float val = 0.0;
  float step = 1.0;
  for (;;) {
    for (int i = 0; i < 511*50; i++) {
      fwrite(&val, sizeof(float), 1, stdout);
      fwrite(&val, sizeof(float), 1, stdout);
    }
    for (int i = 0; i < 50; i++) {
      fwrite(&step, sizeof(float), 1, stdout);
      fwrite(&val, sizeof(float), 1, stdout);
    }
  }
  return 0;

}

/* ---------------------------------------------------------------------- */
