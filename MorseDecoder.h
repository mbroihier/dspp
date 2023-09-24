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
  static constexpr float THRESHOLD_GAIN = 1.25;
  int classificationHistogramPlus[MAX_DAH] = {0};
  int classificationHistogramMinus[MAX_DAH] = {0};
  bool ditClassifier[MAX_DAH] = {false};
  bool dahClassifier[MAX_DAH] = {false};
  bool spaceClassifier[MAX_DAH] = {false};
  bool characterClassifier[MAX_DAH] = {false};
  bool wordClassifier[MAX_DAH] = {false};
  static const int PATTERN_BUFFER_SIZE = 10;
  char pattern[PATTERN_BUFFER_SIZE] = {0};
  int patternIndex = 0;
  static const int MESSAGE_BUFFER_SIZE = 100;
  char message[MESSAGE_BUFFER_SIZE] = {0};
  char lastMessage[MESSAGE_BUFFER_SIZE] = {0};
  int messageIndex = 0;
  int lastBufferSizeRead = 0;
  
  enum ENTITY_TYPE {DIT, DAH, SPACE, CHARACTER_SEPARATION, WORD_SEPARATION, ERROR};
  int intervalCount;
  float deltaThreshold = 0.0;
  float threshold;
  float historicalThreshold = 0.0;

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
  void addToPattern(const char c);
  void resetPattern(void);
  void addToMessage(const char c);
  
 public:
  void setThreshold(float threshold) { this->threshold = threshold; };
  void resetClassifierInfo(void);
  int generateClassifier(float & threshold, bool justRead);
  bool decodeBuffer(int count);
  MorseDecoder(void);
  ~MorseDecoder(void);
};
#endif  // MORSEDECODER_H_
