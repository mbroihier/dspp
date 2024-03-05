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
/* ---------------------------------------------------------------------- */
uint32_t FT4FT8Utilities::ldpcDecode(std::vector<bool> const & pIn174, uint32_t iterations,
                                            std::vector<bool> * pOut174) {
  double m[83][174];
  double e[83][174];
  double codeword[174];

  // to translate from log-likelihood x to probability p,
  // p = e**x / (1 + e**x)
  // it's P(zero), not P(one).
  uint32_t index = 0;
  for (auto b : pIn174) {
    double ex = expl(b ? -4.99 : 4.99);  // if a true(1), -4.99 else 4.99
    double p = ex / (1.0 + ex);
    codeword[index++] = p;
  }

  // m[j][i] tells the j'th check bit the P(zero) of
  // each of its codeword inputs, based on check
  // bits other than j.
  for (int i = 0; i < 174; i++)
    for (int j = 0; j < 83; j++)
      m[j][i] = codeword[i];

  // e[j][i]: each check j tells each codeword bit i the
  // probability of the bit being zero based
  // on the *other* bits contributing to that check.
  for (int i = 0; i < 174; i++)
    for (int j = 0; j < 83; j++)
      e[j][i] = 0.0;

  for (int iter = 0; iter < iterations; iter++) {
    for (int j = 0; j < 83; j++) {
      for (int ii1 = 0; ii1 < 7; ii1++) {
        int i1 = Nm[j][ii1] - 1;
        if (i1 < 0)
          continue;
        double a = 1.0;
        for (int ii2 = 0; ii2 < 7; ii2++) {
          int i2 = Nm[j][ii2] - 1;
          if (i2 >= 0 && i2 != i1) {
            // tmp ranges from 1.0 to -1.0, for
            // definitely zero to definitely one.
            double tmp = 1.0 - 2.0*(1.0-m[j][i2]);
            a *= tmp;
          }
        }
        // a ranges from 1.0 to -1.0, meaning
        // bit i1 should be zero .. one.
        // so e[j][i1] will be 0.0 .. 1.0 meaning
        // bit i1 is one .. zero.
        double tmp = 0.5 + 0.5*a;
        e[j][i1] = tmp;
      }
    }
    std::vector<bool> cw;
    for (int i = 0; i < 174; i++) {
      double q0 = codeword[i];
      double q1 = 1.0 - q0;
      for (int j = 0; j < 3; j++) {
        int j2 = Mn[i][j] - 1;
        q0 *= e[j2][i];
        q1 *= 1.0 - e[j2][i];
      }
      // REAL p = q0 / (q0 + q1);
      double p;
      if (q0 == 0.0) {
        p = 1.0;
      } else {
        p = 1.0 / (1.0 + (q1 / q0));
      }
      cw.push_back(p <= 0.5);
    }
    if (fastScoreLdpc(cw)  == 83) {
      *pOut174 = cw;
      return 83;
    }

    for (int i = 0; i < 174; i++) {
      for (int ji1 = 0; ji1 < 3; ji1++) {
        int j1 = Mn[i][ji1] - 1;
        double q0 = codeword[i];
        double q1 = 1.0 - q0;
        for (int ji2 = 0; ji2 < 3; ji2++) {
          int j2 = Mn[i][ji2] - 1;
          if (j1 != j2) {
            q0 *= e[j2][i];
            q1 *= 1.0 - e[j2][i];
          }
        }
        // REAL p = q0 / (q0 + q1);
        double p;
        if (q0 == 0.0) {
          p = 1.0;
        } else {
          p = 1.0 / (1.0 + (q1 / q0));
        }
        m[j1][i] = p;
      }
    }
  }
  // decode didn't work, don't pass back a modified vector and set the score to zero
  return 0;
}
/* ---------------------------------------------------------------------- */
