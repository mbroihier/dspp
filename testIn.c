/*
 *      Test input
 *
 *      Copyright (C) 2019 
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */

#include <stdio.h>

/* ---------------------------------------------------------------------- */

int main(int argc, char *argv[]) {

  char lsb;
  char msb = 0;
  for (;;) {
    fwrite(&lsb, 1, 1, stdout);
    fwrite(&msb, 1, 1, stdout);
    lsb++;
  }
  return 0;

}

/* ---------------------------------------------------------------------- */
