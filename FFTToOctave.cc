/*
 *      FFT stream to Octave stream
 *
 *      Copyright (C) 2019 
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* ---------------------------------------------------------------------- */

int main(int argc, char *argv[]) {

  float f;
  int size = 0;
  int maxPlots = 0;
  int64_t freq = 0;
  bool firstTime = true;
  bool pause = true;
  float sum = 0.0;
  float r = 0.0;
  float i = 0.0;
  float mag = 0.0;
  float peak = 0.0;

  if (argc == 3 || argc == 4 || argc == 5) {
    sscanf(argv[1], "%d", &size);
    sscanf(argv[2], "%lld", &freq);
    fprintf(stderr, "size: %d, frequency: %lld, raw input %s %s\n", size, freq, argv[1], argv[2]);
    if (argc == 4) {
      if (strcmp(argv[3], "NOPAUSE") == 0) {
        pause = false;
      } else {
        sscanf(argv[3], "%d", &maxPlots);
      }
    } else if (argc == 5) {
      if (strcmp(argv[3], "NOPAUSE") == 0) {
        pause = false;
        sscanf(argv[4], "%d", &maxPlots);
      } else {
        if (strcmp(argv[4], "NOPAUSE") == 0) {
          pause = false;
          sscanf(argv[3], "%d", &maxPlots);
        } else {
          fprintf(stderr,"Usage:  FFToOctave <size of fft (complex)> < sampling frequency> [max plots] [NOPAUSE]\n");
          return -1;
        }
      }
    }
  } else {
    fprintf(stderr,"Usage:  FFToOctave <size of fft (complex)> <sampling frequency> [max plots] [NOPAUSE]\n");
    return -1;
  }
  int graph = 0;
  for (;;) {
    //fprintf(stdout, "signal = [");
    sum = 0.0;
    peak = 0.0;
    for (int j=0; j<size; j++) {
      if ((fread(&f, sizeof(float), 1, stdin)) < 1) {
        return 0;
      }
      if (maxPlots != 0 && maxPlots == graph) {
        return 0;
      }
      if (j == 0) fprintf(stdout, "signal = [");
      fprintf(stdout, "(%f +", f);
      r = f*f;
      fread(&f, sizeof(float), 1, stdin);
      fprintf(stdout, "%f*i) ", f);
      i = f*f;
      sum += mag = sqrt(r+i);
      if (peak < mag) {
        peak = mag;
      }
    }
    fprintf(stdout, "];\n");
    if (firstTime) {
      firstTime = false;
      fprintf(stdout, "bins = 1:%d;\n", size);
      fprintf(stdout, "freq = (bins - 1) * %lld / %d;\n", freq, size);
      fprintf(stdout, "for jj = int32(size(bins)(2)/2):size(bins)(2)\n");
      fprintf(stdout, "freq(jj) = -%lld + freq(jj);\n", freq);
      fprintf(stdout, "endfor\n");
    }
    fprintf(stdout, "mag = abs(signal);\n");
    fprintf(stdout, "semilogy(freq(bins), mag(bins));\n");
    fprintf(stdout, "peak = max(mag) * 2;\n");
    fprintf(stdout, "axis([%lld %lld 0.0 peak]);\n", -freq/2, freq/2);
    fprintf(stdout, "title(\"%d\")\n", graph++);
    if (pause) {
      fprintf(stdout, "pause;\n");
    } else {
      fprintf(stdout, "drawnow;\n");
    }
  }
  return 0;

}

/* ---------------------------------------------------------------------- */
