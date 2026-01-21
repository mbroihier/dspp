/*
 *      FFT stream to Octave stream
 *
 *      Copyright (C) 2026
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */

#include <cinttypes>
#include <math.h>
#include <signal.h>
#include <stdint.h>
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

  float f;
  int size = 0;
  int maxPlots = 0;
  int64_t freq = 0;
  int64_t half_freq = 0;
  int64_t centerFreq = 0;
  float sum = 0.0;
  float r = 0.0;
  float i = 0.0;
  float mag = 0.0;
  float peak = 0.0;

  if (argc == 5) {
    sscanf(argv[1], "%d", &size);
    sscanf(argv[2], "%" PRId64 , &freq);
    half_freq = freq / 2;
    fprintf(stderr, "size: %d, frequency: %" PRId64 ", raw input %s %s\n", size, freq, argv[1], argv[2]);
    sscanf(argv[3], "%d", &maxPlots);
    sscanf(argv[4], "%" PRId64 , &centerFreq);
  } else {
    fprintf(stderr,"Usage:  FFToOctave <size of fft (complex)> <sampling frequency> <max plots> <center frequency>\n");
    //fprintf(stderr,"Usage:  FFToOctavePipe <size of fft (complex)> <sampling frequency> <max plots> <center frequency>\n");
    return -1;
  }
  int graph = 0;
  FILE * octaveFH;
  octaveFH = fopen("FFTToOctave.m", "w");
  //octaveFH = popen("octave --persist", "w");
  if (!octaveFH) {
    fprintf(stderr, "FFTToOctave.m did not open\n");
    return -1;
  }
  int half = size / 2;
  int index;
  int max_index;
  float * bins = (float *) malloc(size * sizeof(float));
  signal(SIGINT, handler);
  while (run) {
    sum = 0.0;
    peak = 0.0;
    max_index = 0;
    for (int j=0; j<size; j++) {
      if ((fread(&f, sizeof(float), 1, stdin)) < 1) {
        fclose(octaveFH);
        //pclose(octaveFH);
        return 0;
      }
      if (maxPlots != 0 && maxPlots == graph) {
        fclose(octaveFH);
        //pclose(octaveFH);
        return 0;
      }
      r = f*f;
      fread(&f, sizeof(float), 1, stdin);
      i = f*f;
      sum += mag = sqrt(r+i);
      if (j < half) {
        index = j + half;
      } else {
        index = j - half;
      }
      index = j;
      bins[index] = mag;
      if (peak < mag) {
        peak = mag;
        max_index = index;
      }
    }
    const char * fmt = "\"%d : %d\\n\"";
    fprintf(octaveFH, "mag = [");
    for (int i = 0; i < size; i++) {
      fprintf(octaveFH, "%f ", bins[i]);
    }
    fprintf(octaveFH, "];\n");
    fprintf(octaveFH, "bins = 1:%d;\n", size);
    fprintf(octaveFH, "freq = (bins - 1) * %" PRId64 " / %d;\n", freq, size);
    fprintf(octaveFH, "for jj = int32(size(bins)(2)/2):size(bins)(2)\n");
    fprintf(octaveFH, "freq(jj) = -%" PRId64 " + freq(jj);\n", freq);
    fprintf(octaveFH, "endfor\n");
    fprintf(octaveFH, "fh = fopen(\"freq.txt\", \"w\");\n");
    fprintf(octaveFH, "for jj = 1:size(bins)(2)\n");
    fprintf(octaveFH, "freq(jj) = %" PRId64 " + freq(jj);\n", centerFreq);
    fprintf(octaveFH, "fprintf(fh, %s, jj, freq(jj));\n", fmt);
    fprintf(octaveFH, "endfor\n");
    fprintf(octaveFH, "fclose(fh);\n");
    fprintf(octaveFH, "peak = max(mag) * 2;\n");
    fprintf(octaveFH, "axis([min(freq) max(freq) 0.0001 peak]);\n");
    fprintf(octaveFH, "semilogy(freq(bins), mag(bins));\n");
    fprintf(octaveFH, "title(\"%d peak frequency: %.0f\")\n", graph++,
            max_index < half_freq ?
            freq*(max_index/(size-1.0))+centerFreq : freq*(max_index/(size-1.0))-freq+centerFreq);
    fprintf(octaveFH, "drawnow;\n");
  }
  fprintf(stdout, "peak frequency: %.0f\n",
            max_index < half_freq ?
            freq*(max_index/(size-1.0))+centerFreq : freq*(max_index/(size-1.0))-freq+centerFreq);
  fprintf(stdout, "sampling frequency: %" PRId64 " half sampling frequency: %.0f, max_index: %d, size: %d, center frequency: %" PRId64 "\n", freq, freq/2.0, max_index, size-1, centerFreq);
  fprintf(stdout, "Controlled exit, pipe should be clean\n");
  fclose(octaveFH);
  FILE * trimFile = popen("grep -n drawnow _FFTToOctave.m | tail -1 | cut -d \":\" -f 1 | xargs -I {} head -{} \
_FFTToOctave.m > FFTToOctave.m ; rm _FFTToOctave.m", "w");
  pclose(trimFile);
  return 0;

}

/* ---------------------------------------------------------------------- */
