/*
 *      Test Real to Complex - pure tone of 1,200 Hz
 *
 *      Copyright (C) 2019 
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */

#include <math.h>
#include <stdio.h>

/* ---------------------------------------------------------------------- */

int main(int argc, char *argv[]) {

  float spaceFrequency = 1200.0;
  float sampleRate = 48000.0;
  float frequency = 0.0;
  float signal = 0.0;
  float zero = 0.0;

  frequency = spaceFrequency;
  for (int j = 0; j < 30; j++) { 
    for (int i = 0; i < 48000; i++) { // one second of data
      signal = cos(2.0 * M_PI * frequency * i / sampleRate);
      fwrite(&signal, sizeof(float), 1, stdout);
      fwrite(&zero, sizeof(float), 1, stdout);
    }
  }
  return 0;

}

/* ---------------------------------------------------------------------- */
