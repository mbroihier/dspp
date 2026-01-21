/*
 *      FFT over time stream to Octave 3D
 *
 *      Copyright (C) 2022
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */
#include <cinttypes>
#include <math.h>
#include <signal.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
/* ---------------------------------------------------------------------- */
int run = 1;
void handler(int sig) {
  if (sig == SIGINT) {
    run = 0;
  }
}
int main(int argc, char *argv[]) {

  //FILE * octaveFH = popen("octave --persist 2>/dev/null", "w");
  FILE * octaveFH = fopen("_FFTOverTimeToOctave.m", "w");
  if (!octaveFH) {
    //fprintf(stderr, "octave pipe did not open\n");
    fprintf(stderr, "octave script file (FFTOverTimeToOctave.m) did not open\n");
    return -1;
  }
  float f;
  int size = 0;
  int duration = 0;
  int64_t centerFreq = 0;
  int64_t freq = 0;
  float r = 0.0;
  float i = 0.0;
  float mag = 0.0;

  if (argc == 5) {
    sscanf(argv[1], "%d", &size);
    sscanf(argv[2], "%" PRId64 , &freq);
    sscanf(argv[3], "%d", &duration);
    sscanf(argv[4], "%" PRId64, &centerFreq);
    fprintf(stderr, "size: %d, sampling frequency: %" PRId64 ", duration: %d, center frequency: %" PRId64 "\n",
            size, freq, duration, centerFreq);
  } else {
    fprintf(stderr,"Usage:  FFTOverTimeToOctave <size of fft (complex)> <sampling frequency> <duration>"
            " <center frequency>\n");
    return -1;
  }
  int graph = 0;
  int timeSample = 0;
  int index;
  int max_index;
  int half = size / 2;
  float max_value;
  float * bins = (float *) malloc(size * sizeof(float));
  fprintf(octaveFH, "frange = linspace(%" PRId64 ", %" PRId64 ", %d);\n", centerFreq - freq/2,
          centerFreq + freq/2, size);
  //fprintf(octaveFH, "frange = linspace(%ld/2, -%ld/2, %d);\n", freq, freq, size);
  fprintf(octaveFH, "trange = linspace(1, %d, %d);\n", duration, duration);
  fprintf(octaveFH, "[X, Y] = meshgrid(trange, frange);\n");
  signal(SIGINT, handler);
  while (run && octaveFH) {
    fprintf(octaveFH, "mag = [");
    while (timeSample < duration && run) {
      max_value = 0.0;
      max_index = 0;
      for (int j=0; j<size; j++) {
        if ((fread(&f, sizeof(float), 1, stdin)) < 1) {
          fprintf(stderr, "Too little data\n");
          if (octaveFH) fclose(octaveFH);
          //if (octaveFH) pclose(octaveFH);
          octaveFH = 0;
          break;
        }
        r = f*f;
        if ((fread(&f, sizeof(float), 1, stdin)) < 1) {
          fprintf(stderr, "Too little data\n");
          if (octaveFH) fclose(octaveFH);
          //if (octaveFH) pclose(octaveFH);
          octaveFH = 0;
          break;
        }
        i = f*f;
        mag = sqrt(r+i);
        if (j < half) {
          index = j + half;
        } else {
          index = j - half;
        }
        bins[index] = mag;
        if (max_value < mag) {
          max_value = mag;
          max_index = index;
        }
      }
      if (octaveFH) {
        for (int j=0; j<size; j++) {
          fprintf(octaveFH, "%f ", bins[j]);
        }
      }
      timeSample++;
      if (!octaveFH) break;
    }
    timeSample = 0;
    if (octaveFH) {
      fprintf(octaveFH, "];\n");
      fprintf(octaveFH, "mag = reshape(mag, %d, %d);\n", size, duration);
      fprintf(octaveFH, "waterfall(X, Y, mag);\n");
      fprintf(octaveFH, "title(\"Graph %d, peak freq: %.0f\")\n", graph++,
              centerFreq + freq * (max_index / (size - 1.0) - 0.5));
      fprintf(octaveFH, "view(0,90);\n");
      fprintf(octaveFH, "drawnow;\n");
    }
  }
  fprintf(stdout, "run value is: %d\n", run);
  fprintf(stdout, "peak frequency: %.0f\n",
          freq*(max_index/(size-1.0) - 0.5)+centerFreq);
  fprintf(stdout, "sampling frequency: %" PRId64 " half sampling frequency: %.0f, max_index: %d, size: %d, center frequ\
ency: %" PRId64 "\n", freq, freq/2.0, max_index, size-1, centerFreq);
  fprintf(stdout, "Controlled exit, pipe should be clean\n");
  if (octaveFH) fclose(octaveFH);
  FILE * trimFile = popen("grep -n drawnow _FFTOverTimeToOctave.m | tail -1 | cut -d \":\" -f 1 | xargs -I {} head -{} _FFTOverTimeToOctave.m > FFTOverTimeToOctave.m ; rm _FFTOverTimeToOctave.m", "w");
  pclose(trimFile);
  //if (octaveFH) pclose(octaveFH);
  return 0;
}

/* ---------------------------------------------------------------------- */
