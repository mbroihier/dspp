/*
 *      Test input - operator 5, impulse
 *
 *      Copyright (C) 2019 
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */

#include <stdio.h>
#include <unistd.h>

/* ---------------------------------------------------------------------- */

int main(int argc, char *argv[]) {

  float val = 0.0;
  float pulse = 1.0;
  int i = 0;
  for (;;) {
    if ((i % 128) == 0) {
      fwrite(&pulse, sizeof(float), 1, stdout);
      fwrite(&val, sizeof(float), 1, stdout);
    } else {
      fwrite(&val, sizeof(float), 1, stdout);
      fwrite(&val, sizeof(float), 1, stdout);
    }
    i++;
    if (i == 2048) {
      sleep(2);
      i = 0;
    }
  }
  return 0;

}

/* ---------------------------------------------------------------------- */
