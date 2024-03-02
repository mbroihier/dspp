/*
 *      FT4FT8Utilities.cc - FT4/FT8 utilities
 *
 *      Copyright (C) 2024
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */
#include <math.h>
#include <cstring>
#include "FT4FT8Utilities.h"

/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
std::vector<bool>  FT4FT8Utilities::crc(std::vector<bool> message) {
  const bool div[] = {true, true, false, false, true, true, true, false, true, false, true, false, true, true, true};
  if (message.size() != 77) {
    fprintf(stderr, "Message to CRC is not 77 bits, it is %d bits\n", message.size());
    exit(-1);
  }
  // zero pad to 82 bits and then add 14 more zeros that will eventually contain the checksum
  std::vector<bool> copy = message;
  for (int i = 0; i < 14+5; i++) {
    copy.push_back(false);
  }
  for (int i = 0; i < message.size()+5; i++) {
    if (copy[i]) {
      for (int j = 0; j < 15; j++) {
        copy[i+j] = copy[i+j] ^ div[j];
      }
    }
  }
  std::vector<bool> cs;
  for (int i = message.size()+5; i < copy.size(); i++) {
    cs.push_back(copy[i]);
  }
  return cs;
}
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
std::vector<bool>  FT4FT8Utilities::ldpc(std::vector<bool> message) {
  std::vector<bool> parityBits;
  for (int i = 0; i < 83; i++) {
    int sum = 0;
    int bitIndex = 0;
    for (auto bitValue : message) {
      sum += (ldpc_generator[i][bitIndex++] & bitValue) ? 1:0;
    }
    parityBits.push_back((sum & 1) ? true:false);
  }
  return parityBits;
}
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
bool  FT4FT8Utilities::checkLdpc(std::vector<bool> message, uint32_t index, std::map<uint32_t,
                              std::vector<uint32_t>> * possibleBits) {
  bool returnValue = false;
  std::vector<uint32_t> bits;
  if (index < 83) {
    int sum = 0;
    for (uint32_t messageIndex = 0; messageIndex < 91; messageIndex++) {
      sum += (ldpc_generator[index][messageIndex] & message[messageIndex]) ? 1:0;
      if (ldpc_generator[index][messageIndex]) bits.push_back(messageIndex);
    }
    returnValue = ((sum & 1) ? true:false) ==  message[index+91];
  }
  if (!returnValue) (*possibleBits)[index] = bits;
  return returnValue;
}
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
bool  FT4FT8Utilities::fastCheckLdpc(std::vector<bool> message, uint32_t index) {
  bool returnValue = false;
  if (index < 83) {
    int sum = 0;
    for (uint32_t messageIndex = 0; messageIndex < 91; messageIndex++) {
      sum += (ldpc_generator[index][messageIndex] & message[messageIndex]) ? 1:0;
    }
    returnValue = ((sum & 1) ? true:false) ==  message[index+91];
  }
  return returnValue;
}
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
uint32_t  FT4FT8Utilities::scoreLdpc(std::vector<bool> message, std::map<uint32_t,
                                  std::vector<uint32_t>> * possibleBits) {
  uint32_t score = 83;
  for (uint32_t index = 0; index < 83; index++) {
    if (!FT4FT8Utilities::checkLdpc(message, index, possibleBits)) {
      score--;
    }
  }
  return score;
}
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
uint32_t  FT4FT8Utilities::fastScoreLdpc(std::vector<bool> message) {
  uint32_t score = 83;
  for (uint32_t index = 0; index < 83; index++) {
    if (!FT4FT8Utilities::fastCheckLdpc(message, index)) {
      score--;
      break;
    }
  }
  return score;
}
/* ---------------------------------------------------------------------- */
