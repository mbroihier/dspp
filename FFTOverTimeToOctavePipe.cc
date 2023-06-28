/*
 *      FFT over time stream to Octave 3D
 *
 *      Copyright (C) 2022
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */

#include <math.h>
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
    sscanf(argv[2], "%lld", &freq);
    sscanf(argv[3], "%d", &duration);
    fprintf(stderr, "size: %d, frequency: %lld, duration: %d\n", size, freq, duration);
  } else {
    fprintf(stderr,"Usage:  FFTOverTimeToOctave <size of fft (complex)> <sampling frequency> <duration>\n");
    return -1;
  }
  int graph = 0;
  int timeSample = 0;
  int index;
  int half = size / 2;
  float * bins = (float *) malloc(size * sizeof(float));
  fprintf(octaveFH, "frange = linspace(-%lld/2, %lld/2, %d);\n", freq, freq, size);
  fprintf(octaveFH, "trange = linspace(1, %d, %d);\n", duration, duration);
  fprintf(octaveFH, "[X, Y] = meshgrid(trange, frange);\n");
  for (;;) {
    if (timeSample < duration) {
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
      fprintf(octaveFH, "];\n");
      fprintf(octaveFH, "mag = reshape(mag, %d, %d);\n", size, duration);
      //fprintf(octaveFH, "waterfall(mag);\n");
      //fprintf(octaveFH, "surf(X, Y, mag);\n");
      fprintf(octaveFH, "waterfall(X, Y, mag);\n");
      fprintf(octaveFH, "title(\"%d\")\n", graph++);
      fprintf(octaveFH, "view(0,90);\n");
      //fprintf(octaveFH, "colormap(colorcube);\n");
      //fprintf(octaveFH, "drawnow;\nhold on;\n");
      fprintf(octaveFH, "drawnow;\n");
    } else {
      for (int j=0; j<size; j++) {
        if ((fread(&f, sizeof(float), 1, stdin)) < 1) {
          fprintf(stderr, "Too little data, FFTOverTimeToOctavePipe\n");
          return 0;
        }
        r = f*f;
        if ((fread(&f, sizeof(float), 1, stdin)) < 1) {
          fprintf(stderr, "Too little data, FTTOverTimeToOctavePipe\n");
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
      fprintf(octaveFH, "column = [");
      for (int j=0; j<size; j++) {
        fprintf(octaveFH, "%f ", bins[j]);
      }          
      fprintf(octaveFH, "];\n");
      fprintf(octaveFH, "column = reshape(column, %d, 1);\n", size);
      fprintf(octaveFH, "mag = mag(1:end,2:end);\n");
      fprintf(octaveFH, "mag = [mag, column];\n");
      //fprintf(octaveFH, "waterfall(mag);\n");
      //fprintf(octaveFH, "surf(X, Y, mag);\n");
      fprintf(octaveFH, "waterfall(X, Y, mag);\n");
      fprintf(octaveFH, "view(0,90);\n");
      fprintf(octaveFH, "title(\"%d\")\n", graph++);
      //fprintf(octaveFH, "colormap(colorcube);\n");
      fprintf(octaveFH, "drawnow;\n");
    }
  }
  return 0;
}

/* ---------------------------------------------------------------------- */
