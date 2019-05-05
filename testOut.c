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

  short integer;
  
  for (;;) {
    fread(&integer, 2, 1, stdin);
    fprintf(stdout, "%d\n", integer);
  }
  return 0;

}

/* ---------------------------------------------------------------------- */
