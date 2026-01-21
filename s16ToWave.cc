
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
    fprintf(stderr, "Usage: ./s16ToWave <frequency> <duration>\n");
    exit(-1);
  }
  samplingFrequency = atoi(argv[1]);
  duration = atoi(argv[2]);

  struct wav_header header;
  
  memcpy(header.riffID, "RIFF", 4);
  header.fileSize = sizeof(wav_header) + samplingFrequency * duration * sizeof(int16_t);
  memcpy(header.fileType, "WAVE", 4);
  memcpy(header.marker, "fmt ", 4);
  header.len = 16;
  header.dataType = 1;
  header.channels = 1;
  header.sampleRate = samplingFrequency;
  header.bitRate = samplingFrequency * 16 * 1 / 8;
  header.bitsPerSampleChannels = 16 * 1 / 8;
  header.bitsPerSample = 16;
  memcpy(header.dataMarker, "data", 4);
  header.dataSize = samplingFrequency * duration * 2;

  fwrite(&header, sizeof(wav_header), 1, stdout);

  int16_t data;
  int numberOfSamples = duration * samplingFrequency;
  fprintf(stderr, "writing %d samples to standard output\n", numberOfSamples);
  for (int i = 0; i < numberOfSamples ; i++) {
    fread(&data, sizeof(int16_t), 1, stdin);
    fwrite(&data, sizeof(int16_t), 1, stdout);
  }
  return 0;
}
