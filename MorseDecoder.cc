/*
 *      MorseDecoder.cc - Object that collects a window of WSPR data
 *
 *      Copyright (C) 2023
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */

#include <algorithm>
#include <stdio.h>
#include "MorseDecoder.h"

/* ---------------------------------------------------------------------- */
int  MorseDecoder::generateClassifier(bool justRead=false) {
  int count;
  threshold = 0.0;
  float average = 0.0;
  count = fread(signal, sizeof(float), BUFFER_SIZE, stdin);
  if (count > 0  && ! justRead) {
    for (int i = 0; i < count; i++) {
      float sample = signal[i];
      if (threshold == 0.0) threshold = sample * THRESHOLD_GAIN;
      if (sample < threshold) {
        average = 0.9 * average + 0.1 * sample;
      }
    }
    threshold = THRESHOLD_GAIN * average;
    fprintf(stderr, "Threhold set to: %f\n", threshold);
      
    bool countingPluses = false;
    bool countingMinuses = false;
    int pluses = 0;
    int minuses = 0;
    for (int i = 0; i < count; i++) {
      if (signal[i] < threshold) {
        if (countingMinuses) {
          minuses++;
        } else if (countingPluses) {
          // transition store plus count in histogram
          if (pluses < MAX_DAH) {
            classificationHistogramPlus[pluses]++;
          }
          countingPluses = false;
          countingMinuses = true;
          pluses = 0;
          minuses++;
        } else {  // flags were both off, this is first minus
          countingPluses = false;
          countingMinuses = true;
          pluses = 0;
          minuses++;
        }
      } else if (countingPluses) {
        pluses++;
      } else if (countingMinuses) {
        // transition
        if (minuses < MAX_DAH) {
          classificationHistogramMinus[minuses]++;
        }
        countingPluses = true;
        countingMinuses = false;
        pluses++;
        minuses = 0;
      } else {  // flags were both off so first plus
        countingPluses = true;
        countingMinuses = false;
        pluses++;
        minuses = 0;
      }
    }
    for (int i = 0; i < MAX_DAH; i++) {
      fprintf(stderr, "classificationHistogramPlus[%3d]: %d\n", i, classificationHistogramPlus[i]);
    }
    for (int i = 0; i < MAX_DAH; i++) {
      fprintf(stderr, "classificationHistogramMinus[%3d]: %d\n", i, classificationHistogramMinus[i]);
    }
      
    // The plus classification histogram should have both dits and dahs in it.  From this histogram, we want
    // to make two classifiers that can be used to classify a signal as a dit or a dah or the lack of a
    // symbol as a space, character separator, or word separator

    // Lets find the two mode edges
    int largestNonZeroBinNumber = 0;
    for (int i = 0; i < MAX_DAH; i++) {
      if (classificationHistogramPlus[i] > 0) {
        largestNonZeroBinNumber = i;
      }
    }
    int smallestNonZeroBinNumber = 0;
    for (int i = 0; i < MAX_DAH; i++) {
      if (classificationHistogramPlus[i] > 0) {
        smallestNonZeroBinNumber = i;
        break;
      }
    }
    for (int i = smallestNonZeroBinNumber; i < largestNonZeroBinNumber; i++) {
      if (classificationHistogramPlus[i] > 0) {
        ditClassifier[i] = true;
      } else {
        break;
      }
    }
    for (int i = largestNonZeroBinNumber; i > smallestNonZeroBinNumber; i--) {
      if (classificationHistogramPlus[i] > 0) {
        dahClassifier[i] = true;
      } else {
        break;
      }
    }  
    for (int i = 0; i < MAX_DAH; i++) {
      fprintf(stderr, "ditClassifier[%3d]: %d\n", i, ditClassifier[i] ? 1:0);
    }
    for (int i = 0; i < MAX_DAH; i++) {
      fprintf(stderr, "dahClassifier[%3d]: %d\n", i, dahClassifier[i] ? 1:0);
    }
    // The minus classification histogram should have spaces, character separators, and words separators in it.
    // From this, we want to make three classifiers that can be used to classify the three entities.

    // First, the space classifier
    int firstMode = 0;
    for (int i = 0; i < MAX_DAH; i++) {
      if (classificationHistogramMinus[i] > 0) {
        firstMode = i;
        spaceClassifier[i] = true;
      } else {
        if (firstMode != 0) {
          break;
        }
      }
    }
    // Next, find the character classifier
    int secondMode = 0;
    for (int i = firstMode + 1; i < MAX_DAH; i++) {
      if (classificationHistogramMinus[i] > 0) {
        secondMode = i;
        characterClassifier[i] = true;
      } else {
        if (secondMode != 0) {
          break;
        }
      }
    }
    // Finally, set the word classifier
    //for (int i = firstMode/2 + secondMode + 1; i < MAX_DAH; i++) {
    int thirdMode = 0;
    for (int i = secondMode + 1; i < MAX_DAH; i++) {
      if (classificationHistogramMinus[i] > 0 || thirdMode != 0) {
        thirdMode = i;
        wordClassifier[i] = true;
      }
    }
    for (int i = 0; i < MAX_DAH; i++) {
      fprintf(stderr, "spaceClassifier[%3d]: %d\n", i, spaceClassifier[i] ? 1:0);
    }
    for (int i = 0; i < MAX_DAH; i++) {
      fprintf(stderr, "characterClassifier[%3d]: %d\n", i, characterClassifier[i] ? 1:0);
    }
    for (int i = 0; i < MAX_DAH; i++) {
      fprintf(stderr, "wordClassifier[%3d]: %d\n", i, wordClassifier[i] ? 1:0);
    }

  }
  return count;
}

void MorseDecoder::decodeBuffer(int count) {
  bool countingPluses = false;
  bool countingMinuses = false;
  int pluses = 0;
  int minuses = 0;
  bool dataObserved = false;
  for (int i = 0; i < count; i++) {
    if (signal[i] < threshold) {
      if (countingMinuses) {
        minuses++;
        if (pluses != 0) fprintf(stderr, "ERROR - pluses not zero, %d\n", pluses);
        if (WORD_SEPARATION == classify(pluses, minuses)  && dataObserved) {
          addToPattern(' ');
          addToMessage(toChar(pattern));
          addToMessage(' ');
          fprintf(stderr, "End of data, %s, %d, %c\n", pattern, minuses, toChar(pattern));
          resetPattern();
          countingPluses = false;
          countingMinuses = false;
          pluses = 0;
          minuses = 0;
          dataObserved = false;
        }              
      } else if (countingPluses) {
        dataObserved = true;
        // transition - classify as dit or dah
        fprintf(stderr, "transition observed + to - : %s\n", toText(classify(pluses, minuses)));
        if (DAH == classify(pluses, minuses)) {
          fprintf(stderr, "Saw a %s\n", toText(DAH));
          addToPattern('-');
        } else if (DIT == classify(pluses, minuses)) {
          fprintf(stderr, "Saw a %s\n", toText(DIT));
          addToPattern('.');
        } else {
          fprintf(stderr, "Plus to minus transition error\n");
        }
        countingPluses = false;
        countingMinuses = true;
        pluses = 0;
        minuses++;
      } else {  // flags were both off, this is first minus
        countingPluses = false;
        countingMinuses = true;
        pluses = 0;
        minuses++;
      }
    } else if (countingPluses) {
      pluses++;
      if (minuses != 0) fprintf(stderr, "ERROR - minuses not zero, %d\n", minuses);
    } else if (countingMinuses) {
      // transition
      fprintf(stderr, "transition observed - to +: %s\n", toText(classify(pluses, minuses)));
      if (WORD_SEPARATION == classify(pluses, minuses)) {
        addToPattern(' ');
        addToMessage(toChar(pattern));
        addToMessage(' ');
        fprintf(stderr, "Saw a %s, %s, %c\n", toText(WORD_SEPARATION), pattern, toChar(pattern));
        resetPattern();
      } else {
        if (SPACE == classify(pluses, minuses)) {
          fprintf(stderr, "Saw a %s\n", toText(SPACE));
        } else if (CHARACTER_SEPARATION == classify(pluses, minuses)) {
          addToPattern(' ');
          addToMessage(toChar(pattern));
          fprintf(stderr, "Saw a %s, %s, %c\n", toText(CHARACTER_SEPARATION), pattern, toChar(pattern));
          resetPattern();
        } else {
          fprintf(stderr, "Start of data, %s, %d, nothing to add\n", pattern, minuses);
          resetPattern();
        }
      }
      countingPluses = true;
      countingMinuses = false;
      pluses++;
      minuses = 0;
    } else {  // flags were both off so first plus
      countingPluses = true;
      countingMinuses = false;
      pluses++;
      minuses = 0;
    }
  }
  fprintf(stdout, "Message(%d, %d): %s\n", strlen(message), count, message);
  message[0] = 0;
  messageIndex = 0;
}

const char * MorseDecoder::toText(ENTITY_TYPE entity) {
  switch (entity) {
  case DIT : return "DIT";
  case DAH : return "DAH";
  case SPACE : return "SPACE";
  case CHARACTER_SEPARATION : return "CHARACTER_SEPARATION";
  case WORD_SEPARATION : return "WORD_SEPARATION";
  default: return "ERROR";
  }
}

void MorseDecoder::addToPattern(const char c) {
  if (patternIndex < PATTERN_BUFFER_SIZE - 1) {
    pattern[patternIndex++] = c;
    pattern[patternIndex] = 0;
  } else {
    fprintf(stderr, "No room for %c\n", c);
  }
}

void MorseDecoder::resetPattern(void) {
  pattern[0] = 0;
  patternIndex = 0;
}

void MorseDecoder::addToMessage(const char c) {
  fprintf(stderr, "atm %c\n", c);
  if (messageIndex < MESSAGE_BUFFER_SIZE - 1 && c != 0) {
    message[messageIndex++] = c;
    message[messageIndex] = 0;
  } else if (messageIndex == MESSAGE_BUFFER_SIZE && c != 0) {
    fprintf(stdout, "%s\n", message);
    message[0] = c;
    message[1] = 0;
    messageIndex = 1;
  }
}

MorseDecoder::ENTITY_TYPE MorseDecoder::classify(int plusCount, int minusCount) {
  //fprintf(stderr, "%d %d\n", plusCount, minusCount);
  if (plusCount > 0 && minusCount == 0) {
    if (ditClassifier[plusCount]) {
      return DIT;
    }
    if (dahClassifier[plusCount]) {
      return DAH;
    }
  } else if (minusCount > 0 && plusCount == 0) {
    if (spaceClassifier[minusCount]) {
      return SPACE;
    }
    if (wordClassifier[minusCount]) {
      return WORD_SEPARATION;
    }
    if (characterClassifier[minusCount]) {
      return CHARACTER_SEPARATION;
    } else {
      return ERROR;
    }
  } else {
    if (plusCount > 0 && minusCount > 0) {
      fprintf(stderr, "ERROR - plus and minus counts both non-zero - this should not happen\n");
      exit(-1);
    }
  }
  return ERROR;
}

const char MorseDecoder::toChar(char * ditDah) {
  if (strlen(ditDah)) {
    for (int i = 0; i < MORSECODES; i++) {
      if (strncmp(ditDah, translationTable[i].ditDah, strlen(ditDah)) == 0) {
        return (translationTable[i].ch);
      }
    }
  }
  return 0;
}

MorseDecoder::MorseDecoder(void) {
  fprintf(stderr, "creating MorseDecoder object\n");
  intervalCount = 0;
  threshold = 0.0;
  fprintf(stderr, "done creating MorseDecoder object\n");
}



MorseDecoder::~MorseDecoder(void) {
  fprintf(stderr, "destructing MorseDecoder\n");
}
#define SELFTEST
#ifdef SELFTEST
int main() {
  int count = 0;
  MorseDecoder morseObj;
  bool justRead = false;
  do {
    count = morseObj.generateClassifier(justRead);
    if (count) morseObj.decodeBuffer(count);
    //justRead = true;
  } while (count > 0);
}
#endif
