/*
 *      Test Real to Complex quadrature
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

  float sampleRate = 48000.0;
  float spaceFrequency = 1200.0;
  float markFrequency = 2200.0;
  float frequency = 0.0;
  float signal = 0.0;

  unsigned char byte = 0x7e; // flag

  frequency = spaceFrequency;
  for (int j = 0; j < 30; j++) { 
    for (int i = 0; i < 48000; i++) { // one second of data
      signal = cos(2.0 * M_PI * frequency * i / sampleRate);
      fwrite(&signal, sizeof(float), 1, stdout);
    }
  }
  frequency = markFrequency;
  for (int j = 0; j < 30; j++) { 
    for (int i = 0; i < 48000; i++) { // one second of data
      signal = cos(2.0 * M_PI * frequency * i / sampleRate);
      fwrite(&signal, sizeof(float), 1, stdout);
    }
  }
  for(int i =0; i<4096; i++) {
    fwrite(&signal,sizeof(float),1,stdout);
  }
  return 0;

}

/* ---------------------------------------------------------------------- */
