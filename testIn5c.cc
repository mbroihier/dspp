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
  for (;;) {
    fwrite(&val, sizeof(float), 1, stdout);
    fwrite(&val, sizeof(float), 1, stdout);
    val++;
  }
  return 0;

}

/* ---------------------------------------------------------------------- */
