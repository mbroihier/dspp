#ifndef MORSEDECODER_H_
#define MORSEDECODER_H_
/*
 *      MorseDecoder.h - Decode Morse code
 *
 *      Copyright (C) 2023
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */
#include <cstring>
#include <stdint.h>
#include <sys/types.h>
/* ---------------------------------------------------------------------- */
class MorseDecoder {
 private:

  static const int BUFFER_SIZE = 4096;
  static const int MAX_DAH = 64;
  static constexpr float THRESHOLD_GAIN = 5.0;
  int classificationHistogramPlus[MAX_DAH] = {0};
  int classificationHistogramMinus[MAX_DAH] = {0};
  bool ditClassifier[MAX_DAH] = {false};
  bool dahClassifier[MAX_DAH] = {false};
  bool spaceClassifier[MAX_DAH] = {false};
  bool characterClassifier[MAX_DAH] = {false};
  bool wordClassifier[MAX_DAH] = {false};
  enum ENTITY_TYPE {DIT, DAH, SPACE, CHARACTER_SEPARATION, WORD_SEPARATION, ERROR};
  int intervalCount;
  float threshold;

// morse code translation table taken from morse.cpp in https://github.com/F5OEO/rpitx
#define MORSECODES 37

  typedef struct morse_code {
    uint8_t ch;
    const char ditDah[8];
  } Morsecode;

  const Morsecode translationTable[MORSECODES]  = {
                                         {' ', "    "},
                                         {'0', "-----  "},
                                         {'1', ".----  "},
                                         {'2', "..---  "},
                                         {'3', "...--  "},
                                         {'4', "....-  "},
                                         {'5', ".....  "},
                                         {'6', "-....  "},
                                         {'7', "--...  "},
                                         {'8', "---..  "},
                                         {'9', "----.  "},
                                         {'A', ".-  "},
                                         {'B', "-...  "},
                                         {'C', "-.-.  "},
                                         {'D', "-..  "},
                                         {'E', ".  "},
                                         {'F', "..-.  "},
                                         {'G', "--.  "},
                                         {'H', "....  "},
                                         {'I', "..  "},
                                         {'J', ".---  "},
                                         {'K', "-.-  "},
                                         {'L', ".-..  "},
                                         {'M', "--  "},
                                         {'N', "-.  "},
                                         {'O', "---  "},
                                         {'P', ".--.  "},
                                         {'Q', "--.-  "},
                                         {'R', ".-.  "},
                                         {'S', "...  "},
                                         {'T', "-  "},
                                         {'U', "..-  "},
                                         {'V', "...-  "},
                                         {'W', ".--  "},
                                         {'X', "-..-  "},
                                         {'Y', "-.--  "},
                                         {'Z', "--..  "}
  };

  float signal[BUFFER_SIZE] = {0.0};

  const char * toText(ENTITY_TYPE);
  const char toChar(char * ditDah);
  ENTITY_TYPE classify(int plusCount, int minuseCount);
  
 public:
  void generateClassifier(void);
  MorseDecoder(void);
  ~MorseDecoder(void);
};
#endif  // MORSEDECODER_H_
