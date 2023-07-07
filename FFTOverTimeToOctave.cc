/*
 *      FFT stream to Octave 3D
 *
 *      Copyright (C) 2022
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
  fprintf(stdout, "mag = [");
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
      fprintf(stdout, "%f ", mag);
    }
    timeSample++;
  }
  fprintf(stdout, "];\n");
  fprintf(stdout, "mag = reshape(mag, %d, %d);\n", size, duration);
  fprintf(stdout, "waterfall(mag);\n");
  fprintf(stdout, "view(0,90);\n");
  fprintf(stdout, "title(\"%d\")\n", graph++);
  fprintf(stdout, "drawnow;\n");
  fprintf(stdout, "m = max(max(mag));\n");
  fprintf(stdout, "ind = (mag > 0.5 * m);\n");
  fprintf(stdout, "b = (1:%d) - 1;\n", size / 2);
  fprintf(stdout, "nb = (-flip(b)) - 1;\n");
  fprintf(stdout, "b = horzcat(b, nb);\n");
  fprintf(stdout, "b = b * %lld / %d;\n", freq, size);
  fprintf(stdout, "b(ind(:,%d:%d))\n", duration / 2, duration / 2);
  return 0;
}

/* ---------------------------------------------------------------------- */
