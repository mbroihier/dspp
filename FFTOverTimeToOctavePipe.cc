/*
 *      FFT over time stream to Octave 3D
 *
 *      Copyright (C) 2022
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */

#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* ---------------------------------------------------------------------- */

int main(int argc, char *argv[]) {

  FILE * octaveFH = popen("octave --persist 2>/dev/null", "w");
  //FILE * octaveFH = fopen("debugScript.m", "w");
  if (!octaveFH) {
    fprintf(stderr, "octave pipe did not open\n");
    return -1;
  }
  float f;
  int size = 0;
  int duration = 0;
  int64_t freq = 0;
  float r = 0.0;
  float i = 0.0;
  float mag = 0.0;

  if (argc == 4) {
    sscanf(argv[1], "%d", &size);
    sscanf(argv[2], "%ld", &freq);
    sscanf(argv[3], "%d", &duration);
    fprintf(stderr, "size: %d, frequency: %ld, duration: %d\n", size, freq, duration);
  } else {
    fprintf(stderr,"Usage:  FFTOverTimeToOctave <size of fft (complex)> <sampling frequency> <duration>\n");
    return -1;
  }
  int graph = 0;
  int timeSample = 0;
  int index;
  int half = size / 2;
  float * bins = (float *) malloc(size * sizeof(float));
  fprintf(octaveFH, "frange = linspace(-%ld/2, %ld/2, %d);\n", freq, freq, size);
  fprintf(octaveFH, "trange = linspace(1, %d, %d);\n", duration, duration);
  fprintf(octaveFH, "[X, Y] = meshgrid(trange, frange);\n");
  for (;;) {
    while (timeSample < duration) {
      for (int j=0; j<size; j++) {
        if ((fread(&f, sizeof(float), 1, stdin)) < 1) {
          fprintf(stderr, "Too little data\n");
          return 0;
        }
        r = f*f;
        if ((fread(&f, sizeof(float), 1, stdin)) < 1) {
          fprintf(stderr, "Too little data\n");
          return 0;
        }
        i = f*f;
        mag = sqrt(r+i);
        if (j < half) {
          index = j + half;
        } else {
          index = j - half;
        }
        bins[index] = mag;
      }
      if (timeSample == 0) fprintf(octaveFH, "mag = [");
      for (int j=0; j<size; j++) {
        fprintf(octaveFH, "%f ", bins[j]);
      }          
      timeSample++;
    }
    timeSample = 0;
    fprintf(octaveFH, "];\n");
    fprintf(octaveFH, "mag = reshape(mag, %d, %d);\n", size, duration);
    fprintf(octaveFH, "waterfall(X, Y, mag);\n");
    fprintf(octaveFH, "title(\"%d\")\n", graph++);
    fprintf(octaveFH, "view(0,90);\n");
    fprintf(octaveFH, "drawnow;\n");
  }
  return 0;
}

/* ---------------------------------------------------------------------- */
