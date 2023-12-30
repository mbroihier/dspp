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
  int64_t centerFreq = 0;
  bool firstTime = true;
  float sum = 0.0;
  float r = 0.0;
  float i = 0.0;
  float mag = 0.0;
  float peak = 0.0;

  if (argc == 5) {
    sscanf(argv[1], "%d", &size);
    sscanf(argv[2], "%ld", &freq);
    fprintf(stderr, "size: %d, frequency: %ld, raw input %s %s\n", size, freq, argv[1], argv[2]);
    sscanf(argv[3], "%d", &maxPlots);
    sscanf(argv[4], "%ld", &centerFreq);
  } else {
    fprintf(stderr,"Usage:  FFToOctavePipe <size of fft (complex)> <sampling frequency> <max plots> <center frequency>\n");
    return -1;
  }
  int graph = 0;
  FILE * octaveFH;
  octaveFH = popen("octave --persist", "w");
  //octaveFH = fopen("script.txt", "w");
  if (!octaveFH) {
    fprintf(stderr, "octave pipe did not open\n");
    return -1;
  }
  for (;;) {
    sum = 0.0;
    peak = 0.0;
    for (int j=0; j<size; j++) {
      if ((fread(&f, sizeof(float), 1, stdin)) < 1) {
        pclose(octaveFH);
        return 0;
      }
      if (maxPlots != 0 && maxPlots == graph) {
        pclose(octaveFH);
        return 0;
      }
      if (j == 0) fprintf(octaveFH, "signal = [");
      fprintf(octaveFH, "(%f +", f);
      r = f*f;
      fread(&f, sizeof(float), 1, stdin);
      fprintf(octaveFH, "%f*i) ", f);
      i = f*f;
      sum += mag = sqrt(r+i);
      if (peak < mag) {
        peak = mag;
      }
    }
    const char * fmt = "\"%d : %d\\n\"";
    fprintf(octaveFH, "];\n");
    if (firstTime) {
      firstTime = false;
      fprintf(octaveFH, "bins = 1:%d;\n", size);
      fprintf(octaveFH, "freq = (bins - 1) * %ld / %d;\n", freq, size);
      fprintf(octaveFH, "for jj = int32(size(bins)(2)/2):size(bins)(2)\n");
      fprintf(octaveFH, "freq(jj) = -%ld + freq(jj);\n", freq);
      fprintf(octaveFH, "endfor\n");
      fprintf(octaveFH, "fh = fopen(\"freq.txt\", \"w\");\n");
      fprintf(octaveFH, "for jj = 1:size(bins)(2)\n");
      fprintf(octaveFH, "freq(jj) = %ld + freq(jj);\n", centerFreq);
      fprintf(octaveFH, "fprintf(fh, %s, jj, freq(jj));\n", fmt);
      fprintf(octaveFH, "endfor\n");
      fprintf(octaveFH, "fclose(fh);\n");
      fprintf(octaveFH, "mag = abs(signal);\n");
      fprintf(octaveFH, "peak = max(mag) * 2;\n");
      //fprintf(octaveFH, "axis([%lld %lld 0.0 peak]);\n", -freq/2 + centerFreq, freq/2 + centerFreq);
      fprintf(octaveFH, "axis([min(freq) max(freq) 0.0 peak]);\n");
      fprintf(octaveFH, "semilogy(freq(bins), mag(bins));\n");
    } else {
      fprintf(octaveFH, "mag = abs(signal);\n");
      fprintf(octaveFH, "semilogy(freq(bins), mag(bins));\n");
    }
    fprintf(octaveFH, "title(\"%d\")\n", graph++);
    fprintf(octaveFH, "drawnow;\n");
  }
  return 0;

}

/* ---------------------------------------------------------------------- */
