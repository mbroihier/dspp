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
  int sampleFile = 0;
  FILE * fptr;
  char filePath[50];

  for (;;) {
    snprintf(filePath, 50, "test14_%d.m", sampleFile);
    sampleFile++;
    fptr = fopen(filePath, "w");

    fprintf(fptr, "signal = [");  
    for (int i=0; i<512; i++) {
      if ((fread(&f, sizeof(float), 1, stdin)) < 1) {
        return 0;
      }
      fprintf(fptr, "(%f +", f);
      fread(&f, sizeof(float), 1, stdin);
      fprintf(fptr, "%f*i) ", f);
    }
    fprintf(fptr, "];\n");
    fclose(fptr);
  }
  return 0;

}

/* ---------------------------------------------------------------------- */
