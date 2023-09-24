
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
#include <complex.h>
#include <ctype.h>
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


// morse code translation table taken from morse.cpp in https://github.com/F5OEO/rpitx
#define MORSECODES 37

typedef struct morse_code {
  uint8_t ch;
  const char ditDah[8];
} Morsecode;

const Morsecode translationTable[]  = {
                                       {' ', " "},
                                       {'0', "----- "},
                                       {'1', ".---- "},
                                       {'2', "..--- "},
                                       {'3', "...-- "},
                                       {'4', "....- "},
                                       {'5', "..... "},
                                       {'6', "-.... "},
                                       {'7', "--... "},
                                       {'8', "---.. "},
                                       {'9', "----. "},
                                       {'A', ".- "},
                                       {'B', "-... "},
                                       {'C', "-.-. "},
                                       {'D', "-.. "},
                                       {'E', ". "},
                                       {'F', "..-. "},
                                       {'G', "--. "},
                                       {'H', ".... "},
                                       {'I', ".. "},
                                       {'J', ".--- "},
                                       {'K', "-.- "},
                                       {'L', ".-.. "},
                                       {'M', "-- "},
                                       {'N', "-. "},
                                       {'O', "--- "},
                                       {'P', ".--. "},
                                       {'Q', "--.- "},
                                       {'R', ".-. "},
                                       {'S', "... "},
                                       {'T', "- "},
                                       {'U', "..- "},
                                       {'V', "...- "},
                                       {'W', ".-- "},
                                       {'X', "-..- "},
                                       {'Y', "-.-- "},
                                       {'Z', "--.. "}
};

size_t messageToMorse(const char * message, char * encodedMessage, size_t maxEncodedLength) {
  size_t messageLength = strlen(message);
  uint32_t encodedMessageIndex = 0;
  bool found;
  for (uint32_t index = 0; index < messageLength; index++) {
    char workingCharacter = toupper(message[index]);
    found = false;
    for (uint32_t tableIndex = 0; tableIndex < MORSECODES; tableIndex++) {
      if (workingCharacter == translationTable[tableIndex].ch) {
        const char * characterPattern = translationTable[tableIndex].ditDah;
        uint32_t patternSize = strlen(characterPattern);
        found = true;
        if (encodedMessageIndex + 4*patternSize < maxEncodedLength) {
          for (uint32_t patternIndex = 0; patternIndex < patternSize; patternIndex++) {
            if (characterPattern[patternIndex] == '.') {
              encodedMessage[encodedMessageIndex++] = 1;
              encodedMessage[encodedMessageIndex++] = 0;
            } else if (characterPattern[patternIndex] == '-') {
              encodedMessage[encodedMessageIndex++] = 1;
              encodedMessage[encodedMessageIndex++] = 1;
              encodedMessage[encodedMessageIndex++] = 1;
              encodedMessage[encodedMessageIndex++] = 0;
            } else {
              encodedMessage[encodedMessageIndex++] = 0;
            }
          }
        } else {
          fprintf(stderr, "Error during encoding - not enough space in encoded message buffer\n");
          exit(-1);
        }
      }
    }
    if (!found) {
      fprintf(stderr, "Error during encoding - character not found in translation table\n");
      exit(-1);
    }
  }
  fprintf(stderr, "Encoded message(%d, max: %d):\n", encodedMessageIndex, maxEncodedLength);
  for (uint32_t index = 0; index < encodedMessageIndex; index++) {
    fprintf(stderr, "%1.1d", encodedMessage[index]);
  }
  fprintf(stderr, "\n");
  return(encodedMessageIndex);
}

int main(int argc, char ** argv) {

  float offset = 50.0;
  uint32_t wordRate = 0;
  const char * message = 0;
  bool wave = false;

  float noiseGain = 0.25;

  if (argc != 4 && argc != 5) {
    fprintf(stderr, "Usage: ./morseBaseBand <frequency offset> <transmission rate> <message - in quotes> [-w]\n");
    exit(-1);
  }
  sscanf(argv[1], "%f", &offset);
  wordRate = atoi(argv[2]);
  message = argv[3];

  if (argc == 5) {
    if (strncmp(argv[4], "-w", 2) == 0) {
      wave = true;
    } else {
      if (sscanf(argv[4], "%f", &noiseGain) != 1) {
        fprintf(stderr, "Usage: ./morseBaseBand <frequency offset> <transmission rate> <message - in quotes> [-w]\n");
        exit(-1);
      }
    }
  }
  size_t messageLen = strlen(message) * 7 * 4;  // 7 dit dahs, 4 bytes per dit dah (maximum)
  char * transmissionBuffer = reinterpret_cast<char *>(malloc(messageLen));
  messageLen = messageToMorse(message, transmissionBuffer, messageLen);

  const int SAMPLING_FREQUENCY = 375;  // Hz
  double t = 0.0;  // seconds
  const double deltaT = 1.0 / SAMPLING_FREQUENCY;  // seconds
  const int DURATION = 120;  // 2 min transmission
  std::complex<double> signal;

  struct wav_header header;
  
  sprintf(header.riffID, "RIFF");
  header.fileSize = sizeof(wav_header) + SAMPLING_FREQUENCY * DURATION * sizeof(int16_t);
  sprintf(header.fileType, "WAVE");
  sprintf(header.marker, "fmt ");
  header.len = 16;
  header.dataType = 1;
  header.channels = 1;
  header.sampleRate = SAMPLING_FREQUENCY;
  header.bitRate = SAMPLING_FREQUENCY * 16 * 1 / 8;
  header.bitsPerSampleChannels = 16 * 1 / 8;
  header.bitsPerSample = 16;
  sprintf(header.dataMarker, "data");
  header.dataSize = SAMPLING_FREQUENCY * DURATION * 2;

  if (wave) {
    fwrite(&header, sizeof(wav_header), 1, stdout);
  }

  float ditsPerSecond = wordRate / 20.0 * 1000.0 / 60.0;
  float secondsPerDit = 1.0 / ditsPerSecond;
  float toneGain = 0.5;
  float driftRate = 0.0;  // Hz/min
  float driftDelta = 0.0;
  float drift = 0.0;
  float startSignalAtF = 2.0;
  int decodedSignalAtT[DURATION * SAMPLING_FREQUENCY] = {0};
  int startSignalAt = startSignalAtF * SAMPLING_FREQUENCY;
  int index = 0;
  size_t indexEncodedMessage = 0;
  int16_t data;
  fprintf(stderr, "Filling on/off buffer\n");
  while (t < DURATION) {
    indexEncodedMessage = index * deltaT / secondsPerDit;
    if (indexEncodedMessage >= messageLen) indexEncodedMessage = messageLen - 1;
    decodedSignalAtT[startSignalAt + index] = transmissionBuffer[indexEncodedMessage];
    index++;
    t = (startSignalAt + index) * deltaT;
  }
  t = 0.0;
  double noiseFrequency = 0.0;
  std::complex<double> noise = 2.0 * M_PI * noiseFrequency * I;
  std::complex<double> omega0 = 2.0 * M_PI * offset * I;
  fprintf(stderr, "signal offset: %f, drift rate: %2.1f, tone gain %5.2f, noise gain %5.2f, start signal at: %f\n",
          offset, driftRate, toneGain, noiseGain, startSignalAtF);
  index = 0;
  float realPart;
  float imagPart;
  std::complex<double> toneGainC = std::complex<double>(toneGain + 0.0 * I);
  std::complex<double> noiseGainC = std::complex<double>(noiseGain + 0.0 * I);
  fprintf(stderr, "Message transmission started.\n");
  float HALF = (RAND_MAX / 2.0);
  while (t < DURATION) {
    noise = 2.0 * M_PI * noiseFrequency * I;
    omega0 = 2.0 * M_PI * (offset + drift) * I;
    signal = toneGainC * ((decodedSignalAtT[index] == 1) * 1.0) * exp(omega0 * t) + noiseGainC * exp(noise * t);
    realPart = real(signal);
    imagPart = imag(signal);
    if (wave) {
      data = 2.0* (std::min(sqrt(realPart * realPart + imagPart * imagPart), 1.0) * 32767.0 - 16383.5);
      fwrite(&data, sizeof(int16_t), 1, stdout);
    } else {
      fwrite(&realPart, sizeof(float), 1, stdout);
      fwrite(&imagPart, sizeof(float), 1, stdout);
    }
    t += deltaT;
    drift += driftDelta;
    noiseFrequency = (rand() - HALF) / RAND_MAX * 200.0;
    index++;
  }
  free(transmissionBuffer);
  return 0;
}
