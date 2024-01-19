
/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>

Mark Broihier
*/

#include <cstring>
#include <ctype.h>
#include <fftw3.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

struct wav_header {
  char riffID[4];
  int32_t fileSize;
  char fileType[4];
  char marker[4];
  int32_t len;
  int16_t dataType;
  int16_t channels;
  int32_t sampleRate;
  int32_t bitRate;
  int16_t bitsPerSampleChannels;
  int16_t bitsPerSample;
  char dataMarker[4];
  int32_t dataSize;
};

int main(int argc, char ** argv) {

  int samplingFrequency;  // Hz
  int duration;

  if (argc != 3) {
    fprintf(stderr, "Usage: ./iqToWave <frequency> <duration>\n");
    exit(-1);
  }
  samplingFrequency = atoi(argv[1]);
  duration = atoi(argv[2]);

  struct wav_header header;
  
  sprintf(header.riffID, "RIFF");
  header.fileSize = sizeof(wav_header) + samplingFrequency * duration * sizeof(int16_t);
  sprintf(header.fileType, "WAVE");
  sprintf(header.marker, "fmt ");
  header.len = 16;
  header.dataType = 1;
  header.channels = 1;
  header.sampleRate = samplingFrequency;
  header.bitRate = samplingFrequency * 16 * 1 / 8;
  header.bitsPerSampleChannels = 16 * 1 / 8;
  header.bitsPerSample = 16;
  sprintf(header.dataMarker, "data");
  header.dataSize = samplingFrequency * duration * 2;

  fwrite(&header, sizeof(wav_header), 1, stdout);

  float realPart;
  float imagPart;
  float thisSample = 0.0;
  int16_t data;
  int numberOfSamples = duration * samplingFrequency;
  int average = 0;
  int accumulator = 0;
  float deltaT = 1.0 / samplingFrequency;
  fprintf(stderr, "writing %d samples to standard output\n", numberOfSamples);
  fftw_complex * signal;
  fftw_complex * signalInFreqDomain;
  signal = (fftw_complex *) fftw_malloc(sizeof(fftw_complex)*numberOfSamples);
  signalInFreqDomain = (fftw_complex *) fftw_malloc(sizeof(fftw_complex)*numberOfSamples);
  fftw_plan planF = fftw_plan_dft_1d(numberOfSamples, signal, signalInFreqDomain, FFTW_FORWARD, FFTW_ESTIMATE);
  fftw_plan planB = fftw_plan_dft_1d(numberOfSamples, signalInFreqDomain, signal, FFTW_BACKWARD, FFTW_ESTIMATE);
  double * doublePtr = (double *) signal;
  for (int i = 0; i < numberOfSamples ; i++) {
    fread(&realPart, sizeof(float), 1, stdin);
    fread(&imagPart, sizeof(float), 1, stdin);
    *doublePtr++ = realPart;
    *doublePtr++ = imagPart;
  }
  fftw_execute(planF);
  for (int i = numberOfSamples/2; i < numberOfSamples; i++) { // zero out negative frequencies
    signalInFreqDomain[i][0] = 0.0;
    signalInFreqDomain[i][1] = 0.0;
  }
  fftw_execute(planB);
  doublePtr = (double *) signal;
  for (int i = 0; i < numberOfSamples; i++) {
    thisSample = *doublePtr++ / numberOfSamples;  // normalize inverse FFT
    thisSample = fmax(fmin(thisSample, 1.0), -1.0) * 32767.0;
    if (i < 1000) {  // don't saturate accumulator
      accumulator += thisSample;
      average = accumulator / (i + 1);
      if((i % 200) == 0) fprintf(stderr, "average: %d, accumulator: %d, time: %4.1f\n", average,
                                 accumulator, i * deltaT);
    }
    data = thisSample - average;
    fwrite(&data, sizeof(int16_t), 1, stdout);
    doublePtr++; // skip complex component
  }
  return 0;
}
