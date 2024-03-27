/*
 *      FT4FT8Fields.cc - Tools to build bit fields for FT4/FT8
 *
 *      Copyright (C) 2024
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */
#include <math.h>
#include <cstring>
#include "FT4FT8Fields.h"
const uint32_t FSIZE[MTend][10] = { {28, 1, 28, 1, 1, 15, 3, 14, 83, 0 },
                                    {28, 1, 28, 1, 1, 15, 3, 14, 83, 0 },
                                    {28, 1, 28, 1, 1, 15, 3, 14, 83, 0 },
                                    {28, 1, 28, 1, 1, 15, 3, 14, 83, 0 },
                                    {12, 58, 1, 2, 1,  0, 0,  0,  0, 0 },
                                    {28, 1, 28, 1, 1, 15, 3, 14, 83, 0 } };
const char * OVERLY[MTend][10] = { {"c28", "r1", "c28", "r1", "R1", "g15", "i3", "cs14", "ldpc83", 0 },
                                   {"c28", "r1", "c28", "r1", "R1", "g15", "i3", "cs14", "ldpc83", 0 },
                                   {"c28", "r1", "c28", "r1", "R1", "g15", "i3", "cs14", "ldpc83", 0 },
                                   {"c28", "r1", "c28", "r1", "R1", "g15", "i3", "cs14", "ldpc83", 0 },
                                   {"h12", "c58", "h1", "r2", "c1",    0,     0,      0,        0, 0 },
                                   {"c28", "r1", "c28", "r1", "R1", "g15", "i3", "cs14", "ldpc83", 0 } };

/* ---------------------------------------------------------------------- */
FT4FT8Fields::FT4FT8Fields(uint32_t bits) {
  if (bits <= 0) {
    fprintf(stderr, "Can't have a bit field with less than 1 bit\n");
    exit(-1);
  }
  // fprintf(stderr, "in base constructor, setting bits and fieldBytes %p\n", this);
  this->bits = bits;
  this->bytes = bits /8 + (((bits % 8) == 0) ? 0:1);
  fieldBytes = reinterpret_cast<uint8_t *>(malloc(bytes));
  memset(fieldBytes, 0, bytes);
  fieldIndices.push_back(0);
  fieldSizes.push_back(bits);
}
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
FT4FT8Fields::FT4FT8Fields(uint32_t bits, uint64_t data) {
  // fprintf(stderr, "in base class constructor, setting input: %d bits, %llu data %p\n", bits, data, this);
  if (pow(2.0, bits) <= data) {
    fprintf(stderr, "data (%llu) can't fit in %d bits\n", data, bits);
    exit(-1);
  }
  this->bits = bits;
  this->bytes = bits /8 + (((bits % 8) == 0) ? 0:1);
  fieldBytes = reinterpret_cast<uint8_t *>(malloc(bytes));
  uint64_t copy = data;
  uint32_t bitsFilled = 0;
  std::vector<bool> working;
  for (int i = bytes - 1; i >= 0; i--) {
    fieldBytes[i] = copy & 0xff;
    if (bitsFilled < bits) {
      working.push_back((copy & 0x01) != 0);
      bitsFilled++;
    }
    if (bitsFilled < bits) {
      working.push_back((copy & 0x02) != 0);
      bitsFilled++;
    }
    if (bitsFilled < bits) {
      working.push_back((copy & 0x04) != 0);
      bitsFilled++;
    }
    if (bitsFilled < bits) {
      working.push_back((copy & 0x08) != 0);
      bitsFilled++;
    }
    if (bitsFilled < bits) {
      working.push_back((copy & 0x10) != 0);
      bitsFilled++;
    }
    if (bitsFilled < bits) {
      working.push_back((copy & 0x20) != 0);
      bitsFilled++;
    }
    if (bitsFilled < bits) {
      working.push_back((copy & 0x40) != 0);
      bitsFilled++;
    }
    if (bitsFilled < bits) {
      working.push_back((copy & 0x80) != 0);
      bitsFilled++;
    }
    copy >>= 8;
  }
  for (int i = bits - 1; i >= 0; i--) {
    fieldBits.push_back(working[i]);
  }
  fieldIndices.push_back(0);
  fieldSizes.push_back(bits);
}
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
FT4FT8Fields::FT4FT8Fields(uint32_t bits, uint64_t data, const char * fieldType) {
  // fprintf(stderr, "in base class constructor, setting input: %d bits, %llu data %p\n", bits, data, this);
  if (pow(2.0, bits) <= data) {
    fprintf(stderr, "data (%llu) can't fit in %d bits\n", data, bits);
    exit(-1);
  }
  this->bits = bits;
  this->bytes = bits /8 + (((bits % 8) == 0) ? 0:1);
  fieldBytes = reinterpret_cast<uint8_t *>(malloc(bytes));
  uint64_t copy = data;
  uint32_t bitsFilled = 0;
  fieldTypes.push_back(fieldType);
  fieldIndices.push_back(0);
  fieldSizes.push_back(bits);
  std::vector<bool> working;
  for (int i = bytes - 1; i >= 0; i--) {
    fieldBytes[i] = copy & 0xff;
    if (bitsFilled < bits) {
      working.push_back((copy & 0x01) != 0);
      bitsFilled++;
    }
    if (bitsFilled < bits) {
      working.push_back((copy & 0x02) != 0);
      bitsFilled++;
    }
    if (bitsFilled < bits) {
      working.push_back((copy & 0x04) != 0);
      bitsFilled++;
    }
    if (bitsFilled < bits) {
      working.push_back((copy & 0x08) != 0);
      bitsFilled++;
    }
    if (bitsFilled < bits) {
      working.push_back((copy & 0x10) != 0);
      bitsFilled++;
    }
    if (bitsFilled < bits) {
      working.push_back((copy & 0x20) != 0);
      bitsFilled++;
    }
    if (bitsFilled < bits) {
      working.push_back((copy & 0x40) != 0);
      bitsFilled++;
    }
    if (bitsFilled < bits) {
      working.push_back((copy & 0x80) != 0);
      bitsFilled++;
    }
    copy >>= 8;
  }
  for (int i = bits - 1; i >= 0; i--) {
    fieldBits.push_back(working[i]);
  }
  fieldIndices.push_back(0);
  fieldSizes.push_back(bits);
}
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
FT4FT8Fields::FT4FT8Fields(uint32_t bits, std::vector<bool> data) {
  // fprintf(stderr, "in base class constructor, setting input: %d bits, vector of boolean data size: %d %p\n", bits,
  //         data.size(), this);
  if (bits != data.size()) {
    fprintf(stderr, "data vector of size %u, can't fit in %d bits\n", data.size(), bits);
    exit(-1);
  }
  this->bits = bits;
  this->bytes = bits /8 + (((bits % 8) == 0) ? 0:1);
  for (uint32_t i = 0; i < bits; i++) {
    fieldBits.push_back(data[i]);
  }
  fieldBytes = reinterpret_cast<uint8_t *>(malloc(bytes));
  memset(fieldBytes, 0, bytes);
  int bitIndex = 0;
  int byteIndex = bytes - 1;
  for (int i = bits - 1; i >= 0; i--) {
    fieldBytes[byteIndex] |= (fieldBits[i] ? 1:0) << (bitIndex % 8);
    bitIndex++;
    if ((bitIndex % 8) == 0) byteIndex--;
  }
  fieldIndices.push_back(0);
  fieldSizes.push_back(bits);
}
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
FT4FT8Fields::FT4FT8Fields(uint32_t bits, std::vector<bool> data, std::vector<const char *> fields) {
  // fprintf(stderr, "in base class constructor, setting input: %d bits, vector of boolean data size: %d %p\n", bits,
  //         data.size(), this);
  if (bits != data.size()) {
    fprintf(stderr, "data vector of size %u, can't fit in %d bits\n", data.size(), bits);
    exit(-1);
  }
  if (fields.size() != 1) {
    fprintf(stderr, "There can only be one field defined in the vector for this constructor\n");
    exit(-1);
  }
  this->bits = bits;
  this->bytes = bits /8 + (((bits % 8) == 0) ? 0:1);
  for (uint32_t i = 0; i < bits; i++) {
    fieldBits.push_back(data[i]);
  }
  fieldBytes = reinterpret_cast<uint8_t *>(malloc(bytes));
  memset(fieldBytes, 0, bytes);
  int bitIndex = 0;
  int byteIndex = bytes - 1;
  for (int i = bits - 1; i >= 0; i--) {
    fieldBytes[byteIndex] |= (fieldBits[i] ? 1:0) << (bitIndex % 8);
    bitIndex++;
    if ((bitIndex % 8) == 0) byteIndex--;
  }
  fieldTypes = fields;
  fieldIndices.push_back(0);
  fieldSizes.push_back(bits);
}
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
FT4FT8Fields::FT4FT8Fields(uint32_t bits, std::vector<bool> data, std::vector<const char *> fields,
                           std::vector<uint32_t> fieldIndices, std::vector<uint32_t> fieldSizes) {
  // fprintf(stderr, "in base class constructor, setting input: %d bits, vector of boolean data size: %d %p\n", bits,
  //         data.size(), this);
  if (bits != data.size()) {
    fprintf(stderr, "data vector of size %u, can't fit in %d bits\n", data.size(), bits);
    exit(-1);
  }
  this->bits = bits;
  this->bytes = bits /8 + (((bits % 8) == 0) ? 0:1);
  for (uint32_t i = 0; i < bits; i++) {
    fieldBits.push_back(data[i]);
  }
  fieldBytes = reinterpret_cast<uint8_t *>(malloc(bytes));
  memset(fieldBytes, 0, bytes);
  int bitIndex = 0;
  int byteIndex = bytes - 1;
  for (int i = bits - 1; i >= 0; i--) {
    fieldBytes[byteIndex] |= (fieldBits[i] ? 1:0) << (bitIndex % 8);
    bitIndex++;
    if ((bitIndex % 8) == 0) byteIndex--;
  }
  fieldTypes = fields;
  this->fieldIndices = fieldIndices;
  this->fieldSizes = fieldSizes;
}
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
FT4FT8Fields::~FT4FT8Fields(void) {
  if (fieldBytes) free(fieldBytes);
  // fprintf(stderr, "Field object destroyed %p\n", this);
}
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
FT4FT8Fields::FT4FT8Fields(const FT4FT8Fields& orig) {
  bits = orig.bits;
  bytes = orig.bytes;
  fieldBytes = reinterpret_cast<uint8_t *>(malloc(bytes));
  for (uint32_t i = 0; i < bytes; i++) {
    fieldBytes[i] = orig.fieldBytes[i];
  }
  fieldBits = orig.fieldBits;
  fieldTypes = orig.fieldTypes;
  fieldIndices = orig.fieldIndices;
  fieldSizes = orig.fieldSizes;
  // fprintf(stderr, "const Field object copied %p\n", this);
}
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
c28::c28(const c28& orig) {
  bits = orig.bits;
  bytes = orig.bytes;
  fieldBytes = reinterpret_cast<uint8_t *>(malloc(bytes));
  for (uint32_t i = 0; i < bytes; i++) {
    fieldBytes[i] = orig.fieldBytes[i];
  }
  fieldBits = orig.fieldBits;
  fieldTypes = orig.fieldTypes;
  fieldIndices = orig.fieldIndices;
  fieldSizes = orig.fieldSizes;
  // fprintf(stderr, "const c28 Field object copied %p\n", this);
}
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
c28 c28::convertToC28(const FT4FT8Fields& orig) {
  c28 newOne;
  uint8_t * oldBytes = orig.getFieldBytes();
  if (orig.getBits() == 28) {
    newOne.bits = orig.getBits();
    newOne.bytes = orig.getBytes();
    newOne.fieldBytes = reinterpret_cast<uint8_t *>(malloc(newOne.bytes));
    for (uint32_t i = 0; i < newOne.bytes; i++) {
      newOne.fieldBytes[i] = oldBytes[i];
    }
    newOne.fieldBits = orig.getFieldBits();
    newOne.fieldTypes = orig.getFieldTypes();
    newOne.fieldIndices = orig.getFieldIndices();
    newOne.fieldSizes = orig.getFieldSizes();
  } else {
    fprintf(stderr, "Objects can't be overlayed - C28 constructor\n");
    exit(-1);
  }
  return newOne;
}
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
void FT4FT8Fields::print(void) const {
  fprintf(stderr, "FT4FT8Fields object at %p\n", this);
  fprintf(stderr, "bits: %d, number of bytes: %d, bit vector size: %d\n", bits, bytes, fieldBits.size());
  if (bytes) { fprintf(stderr, "%p", fieldBytes); }
  for (uint32_t i = 0; i < bytes; i++) {
    fprintf(stderr, " %2.2x", fieldBytes[i]);
  }
  fprintf(stderr, "\n");
  if (fieldBits.size() == bits) {
    for (uint32_t i = 0; i < bits; i++) {
      fprintf(stderr, "%d", fieldBits[i]?1:0);
    }
    fprintf(stderr, "\n");
  }
  fprintf(stderr, "field types in this object: ");
  for (auto t : fieldTypes) {
    fprintf(stderr, "%s ", t);
  }
  fprintf(stderr, "\n");
  fprintf(stderr, "field indices in this object: ");
  for (auto t : fieldIndices) {
    fprintf(stderr, "%d ", t);
  }
  fprintf(stderr, "\n");
  fprintf(stderr, "field sizes in this object: ");
  for (auto t : fieldSizes) {
    fprintf(stderr, "%d ", t);
  }
  fprintf(stderr, "\n");
}
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
void FT4FT8Fields::toOctal(void) const {
  if (fieldBits.size() == bits) {
    int octet = 0;
    bool printed = false;
    fprintf(stderr, "Octal values\n");
    for (uint32_t i = 0; i < bits; i++) {
      printed = false;
      octet |= (fieldBits[i]?1:0) << (2 - (i % 3));
      if ((i % 3) == 2) {
        fprintf(stderr, "%d ", octet);
        octet = 0;
        printed = true;
      }
    }
    if (printed) {
      fprintf(stderr, "\n");
    } else {
      fprintf(stderr, "%d\n", octet);
    }
    printed = false;
    octet = 0;
    fprintf(stderr, "Octal values - gray coded\n");
    for (uint32_t i = 0; i < bits; i++) {
      printed = false;
      octet |= (fieldBits[i]?1:0) << (2 - (i % 3));
      if ((i % 3) == 2) {
        fprintf(stderr, "%d ", FT4FT8Utilities::toGray(octet));
        octet = 0;
        printed = true;
      }
    }
    if (printed) {
      fprintf(stderr, "\n");
    } else {
      fprintf(stderr, "%d\n", octet);
    }
  }
}
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
FT4FT8Fields  FT4FT8Fields::operator+(const FT4FT8Fields & rhs) {
  std::vector<bool> rhsFieldBits = rhs.getField();
  std::vector<bool> lhsFieldBits = this->getField();
  std::vector<bool> concatBits;
  std::vector<const char *> concatTypes;
  std::vector<uint32_t> concatIndices;
  std::vector<uint32_t> concatSizes;
  for (auto b : lhsFieldBits) {
    concatBits.push_back(b);
  }
  for (auto b : rhsFieldBits) {
    concatBits.push_back(b);
  }
  for (auto t : this->fieldTypes) {
    concatTypes.push_back(t);
  }
  for (auto t : rhs.fieldTypes) {
    concatTypes.push_back(t);
  }
  concatIndices = this->fieldIndices;
  concatSizes = this->fieldSizes;
  uint32_t lastIndex = concatIndices.back();
  uint32_t lastSize = this->fieldSizes.back();
  for (auto i : rhs.fieldSizes) {
    concatSizes.push_back(i);
    concatIndices.push_back(lastIndex+lastSize);
    lastIndex = concatIndices.back();
    lastSize = i;
  }
  return FT4FT8Fields(concatBits.size(), concatBits, concatTypes, concatIndices, concatSizes);
}
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
FT4FT8Fields&  FT4FT8Fields::operator=(const FT4FT8Fields & rhs) {
  this->fieldBits = rhs.getField();
  this->fieldTypes = rhs.fieldTypes;
  this->fieldIndices = rhs.fieldIndices;
  this->fieldSizes = rhs.fieldSizes;
  this->bits = rhs.bits;
  this->bytes = rhs.bytes;
  if (this->fieldBytes) { free(this->fieldBytes); }
  this->fieldBytes = reinterpret_cast<uint8_t *>(malloc(rhs.bytes));
  for (uint32_t i = 0; i < rhs.bytes; i++) {
    this->fieldBytes[i] = rhs.fieldBytes[i];
  }
  // fprintf(stderr, "FT4FT8Fields assignment %p\n", this);
  return *this;
}
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
FT4FT8Fields  FT4FT8Fields::operator()(const char * index, uint32_t instance) {
  bool found = false;
  uint32_t vectorIndex = 0;
  for (auto ft : this->fieldTypes) {
    if ( strcmp(ft, index) == 0 ) {
      if (instance == 0) {
        // fprintf(stderr, "Type found, we can return a vector, index = %d\n", vectorIndex);
        found = true;
        break;
      } else {
        instance--;
      }
    }
    vectorIndex++;
  }
  if (found) {
    std::vector<bool> data;
    std::vector<const char *> fieldType;
    fieldType.push_back(this->fieldTypes[vectorIndex]);
    uint32_t startAt = this->fieldIndices[vectorIndex];
    uint32_t endAt = this->fieldSizes[vectorIndex] + startAt;
    for (uint32_t i = startAt; i < endAt; i++) {
      data.push_back(this->fieldBits[i]);
    }
    return FT4FT8Fields(this->fieldSizes[vectorIndex], data, fieldType);
  }
  return FT4FT8Fields(1, 1);;
}
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
std::vector<bool> FT4FT8Fields::operator ()(const char * index, uint32_t instance, bool dumpBits) {
  bool found = false;
  uint32_t vectorIndex = 0;
  for (auto ft : this->fieldTypes) {
    if ( strcmp(ft, index) == 0 ) {
      if (instance == 0) {
        // fprintf(stderr, "Type found, we can return a vector, index = %d\n", vectorIndex);
        found = true;
        break;
      } else {
        instance--;
      }
    }
    vectorIndex++;
  }
  std::vector<bool> data;
  if (found) {
    uint32_t startAt = this->fieldIndices[vectorIndex];
    uint32_t endAt = this->fieldSizes[vectorIndex] + startAt;
    for (uint32_t i = startAt; i < endAt; i++) {
      data.push_back(this->fieldBits[i]);
    }
  }
  return data;
}
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
std::vector<bool> FT4FT8Fields::overlay(MESSAGE_TYPES mt, const FT4FT8Fields & object, const char * selector,
                                        uint32_t instance) {
  uint32_t index = 0;
  uint32_t nextIndex = 0;
  uint32_t startAt = 0;
  uint32_t endAt = 0;
  bool found = false;
  std::vector<bool> returnVector;
  do {
    if (strcmp(OVERLY[mt][index], selector) == 0) {
      if (!instance) {
        found = true;
        startAt = nextIndex;
        endAt = startAt + FSIZE[mt][index];
      } else {
        instance--;
      }
    }
    nextIndex += FSIZE[mt][index];
    index++;
  } while (!found & (OVERLY[mt][index] != 0));
  if (found) {
    std::vector<bool> allBits = object.getFieldBits();
    for (uint32_t i = startAt; i < endAt; i++) {
      returnVector.push_back(allBits[i]);
    }
  }
  return returnVector;
}
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
const char A1[] = " 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const char A2[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const char A3[] = "0123456789";
const char A4[] = " ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const char A5[] = " 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ/";
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
int32_t FT4FT8Fields::isIn(char b, const char * a) {
  int len = strlen(a);
  int rtn = -1;
  for (int i = 0; i < len; i++) {
    if (b == a[i]) {
      rtn = i;
      break;
    }
  }
  return rtn;
}
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
c28 c28::encode(char * displayFormat) {
  // from Karlis Goba (YL3JG) ft8_lib https://github.com/kgoba/ft8_lib
  uint64_t binary = 0;
  char * rptr = 0;
  char * copy = strdup(displayFormat);
  char * token = strtok_r(copy, " ", &rptr);
  bool status = false;  // error occurred
  if (strcmp(token, "DE") == 0) {
    binary = 0;
  } else if (strcmp(token, "QRZ") == 0) {
    binary = 1;
  } else if (strcmp(token, "CQ") == 0) {
    binary = 2;
  } else if (strncmp(token, "CQ_", 3) == 0) {
    fprintf(stderr, "CQ_... not supported\n");
    status = true;
  } else {  // we'll see if it is a standard call sign
    char working[7];
    memset(working, ' ', 6); working[6] = 0;
    strncpy(working, token, 6);
    if (strlen(token) < 6) working[strlen(token)] = ' ';  // change terminating null to blank
    if (isIn(working[0], A1) < 0) fprintf(stderr, "failed with %c\n", working[0]);
    if (isIn(working[1], A2) < 0) fprintf(stderr, "failed with %c\n", working[1]);
    if (isIn(working[2], A3) == 0) fprintf(stderr, "failed with %c\n", working[2]);
    if (isIn(working[3], A4) == 0) fprintf(stderr, "failed with %c\n", working[3]);
    if (isIn(working[4], A4) == 0) fprintf(stderr, "failed with %c\n", working[4]);
    if (isIn(working[5], A4) == 0) fprintf(stderr, "failed with %c\n", working[5]);
    if (isIn(working[0], A4) && isIn(working[1], A2) && isIn(working[2], A3) &&
        isIn(working[3], A4) && isIn(working[4], A4) && isIn(working[5], A4)) {
      binary = isIn(working[0], A1);
      binary = binary * 36 + isIn(working[1], A2);
      binary = binary * 10 + isIn(working[2], A3);
      binary = binary * 27 + isIn(working[3], A4);
      binary = binary * 27 + isIn(working[4], A4);
      binary = binary * 27 + isIn(working[5], A4);
      binary += 2063592;  // add on offsets that bypass other token ranges
      binary += 4194304;
    } else {
      fprintf(stderr, "Can't encode this callsign: %s\n", working);
      status = true;
    }
  }
  free(copy);
  if (status) {  // if error occurred
    exit(-1);
  }
  return c28(binary);
}
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
char * c28::decode(std::map<uint32_t, char *> * hash22, std::map<uint32_t, char *> *hash12, std::map<uint32_t, char *>
                   * hash10) {
  // from Karlis Goba (YL3JG) ft8_lib https://github.com/kgoba/ft8_lib
  uint64_t binary = 0;
  for (int i = bits - 1; i >= 0; i--) {
    binary |= (fieldBits[i] ? 1:0) << (bits - 1 - i);
  }
  if (binary == 0) {
    snprintf(callSign, sizeof(callSign), "DE");
  } else if (binary == 1) {
    snprintf(callSign, sizeof(callSign), "QRZ");
  } else if (binary == 2) {
    snprintf(callSign, sizeof(callSign), "CQ");
  } else {  // see if standard call sign
    char working[12];
    memset(working, ' ', sizeof(working)-1); working[sizeof(working)-1] = 0;
    if (binary >= 2063592 + 4194304) {
      binary = binary - 2063592 - 4194304;
      working[5] = A4[binary % 27];
      binary /= 27;
      working[4] = A4[binary % 27];
      binary /= 27;
      working[3] = A4[binary % 27];
      binary /= 27;
      working[2] = A3[binary % 10];
      binary /= 10;
      working[1] = A2[binary % 36];
      binary /= 36;
      working[0] = A1[binary];
      snprintf(callSign, sizeof(callSign), "%s", working);
      // working may have leading blanks, suppress them
      while (working[0] == ' ') {
        bool allBlanks = true;
        for (uint32_t i = 0; i < sizeof(working) - 1; i++) {
          working[i] = (working[i+1] == 0) ? ' ' : working[i+1];
          if (working[i] != ' ') allBlanks = false;
        }
        if (allBlanks) break;
      }
      // now create and store a hash of this call sign - this is not thread safe
      uint64_t hash = 0;
      bool good = true;
      for (uint32_t i = 0; i < sizeof(working) - 1; i++) {
        int32_t index = isIn(working[i], A5);
        if (index < 0) {  // can't hash
          good = false;
          break;
        }
        hash = 38 * hash + index;
      }
      // now trim trailing blanks
      char * tthash = strdup(working);
      for (int32_t i = strlen(tthash) - 1; i >= 0; i--) {
        if (tthash[i] == ' ') {
          tthash[i] = 0;
        } else {
          break;
        }
      }
      snprintf(callSign, sizeof(callSign), "%s", tthash);
      if (good) {
        hash *= 47055833459LL;
        hash >>= 64 - 22;
        if ((*hash22).find(hash) == (*hash22).end()) {
          // don't overwrite an existing hash, or if you do, free the memory that has been allocated
          // fprintf(stdout, "new hash of %6.6llx for %s\n", hash, working);
          (*hash22)[hash] = tthash;  // release this memory in destructor and do it first
          hash >>= 10;  // calculate a 12 bit hash
          (*hash12)[hash] = tthash;  // don't release this memory in destructor
          hash >>= 2;  // calculate a 10 bit hash
          (*hash10)[hash] = tthash;  // don't release this memory in destructor
        } else {
          free(tthash);  //  free memory that isn't used in the hash tables
          // fprintf(stdout, "suppressing hash of %6.6llx for %s\n", hash, working);
        }
      } else {
        free(tthash);  //  free memory that isn't used in the hash tables
        fprintf(stdout, "failed to hash %s\n", working);
      }
    } else if (binary >= 2063592) {
      binary -= 2063592;
      if ((*hash22).find(binary) != (*hash22).end()) {
        fprintf(stdout, "hash found and being translated to %s\n", (*hash22)[binary]);
        snprintf(callSign, sizeof(callSign), "<%s>", (*hash22)[binary]);
      } else {
        fprintf(stdout, "Can't decode this hash callsign: %16.16llx, not in table\n", binary);
        snprintf(callSign, sizeof(callSign), "<hash>");
      }
    } else {
        fprintf(stdout, "Can't decode this callsign: %16.16llx, out of range\n", binary);
        snprintf(callSign, sizeof(callSign), "unknown");
    }
  }
  return callSign;
}
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
char * c58::decode(void) {
  // from Karlis Goba (YL3JG) ft8_lib https://github.com/kgoba/ft8_lib
  uint64_t binary = 0;
  for (int i = bits - 1; i >= 0; i--) {
    binary |= (fieldBits[i] ? 1LL:0LL) << (bits - 1 - i);
  }
  char working[12];
  memset(working, ' ', sizeof(working)-1); working[sizeof(working)-1] = 0;
  for (int32_t i = sizeof(working) - 2; i >= 0; i--) {
    working[i] = A5[binary % 38];
    binary /= 38;
  }
  snprintf(callSign, sizeof(callSign), "%s", working);
  return callSign;
}
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
c58 c58::encode(char * displayFormat) {
  // from Karlis Goba (YL3JG) ft8_lib https://github.com/kgoba/ft8_lib
  uint64_t binary = 0;
  if (strlen(displayFormat) < sizeof(callSign)) {
    for (uint32_t i = 0; i < sizeof(callSign) - 1; i++) {
      int32_t index = 0;
      if (i < strlen(displayFormat)) index = isIn(displayFormat[i], A5);
      if (index < 0) {  // couldn't encode
        fprintf(stderr, "c58 encode could not encode the value: %c\n", displayFormat[i]);
        exit(-1);
      } else {
        binary *= 38;
        binary += index;
      }
    }
  } else {
    fprintf(stderr, "c58 can't encode this callsign: %s\n", displayFormat);
    exit(-1);
  }
  return c58(binary);
}
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
g15 g15::encode(char * displayFormat) {
  // from Karlis Goba (YL3JG) ft8_lib https://github.com/kgoba/ft8_lib
  uint64_t binary = 0;
  char * copy = strdup(displayFormat);
  bool status = false;  // error occurred
  if ((strlen(copy) == 4) && (copy[0] >= 'A' && copy[0] <= 'R') &&
      (copy[1] >= 'A' && copy[1] <= 'R') &&
      (copy[2] >= '0' && copy[2] <= '9') &&
      (copy[3] >= '0' && copy[3] <= '9')) {
    binary = copy[0] - 'A';
    binary = binary * 18 + copy[1] - 'A';
    binary = binary * 10 + copy[2] - '0';
    binary = binary * 10 + copy[3] - '0';
  } else {
    fprintf(stderr, "Can't encode this grid: %s\n", copy);
    status = true;
  }
  free(copy);
  if (status) {  // if error occurred
    exit(-1);
  }
  return g15(binary);
}
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
char * g15::decode(void) {
  // from Karlis Goba (YL3JG) ft8_lib https://github.com/kgoba/ft8_lib
  uint64_t binary = 0;
  for (int i = bits - 1; i >= 0; i--) {
    binary |= (fieldBits[i] ? 1:0) << (bits - 1 - i);
  }
  bool status = false;  // error occurred
  char working[5];
  memset(working, ' ', 5); working[4] = 0;
  if (binary >= 32400) {  // signal report
    binary -= 32400;
    if (binary == 1) {
      grid[0] = 0;  // blank
    } else if (binary == 2) {
      snprintf(grid, sizeof(grid), "RRR");
    } else if (binary == 3) {
      snprintf(grid, sizeof(grid), "RR73");
    } else if (binary == 4) {
      snprintf(grid, sizeof(grid), "73");
    } else {
      if (binary < 0x7fffffff) {
        int32_t signedBinary = binary;
        signedBinary -= 35;
        snprintf(grid, sizeof(grid), "%c%d", signedBinary < 0 ? '-':'+', abs(signedBinary));
      } else {
        grid[0] = 0;
        fprintf(stderr, "data field too large to be decoded as g15\n");
      }
    }
  } else {
    working[3] = binary % 10 + '0';
    binary /= 10;
    working[2] = binary % 10 +'0';
    binary /= 10;
    working[1] = binary % 18 + 'A';
    binary /= 18;
    if (binary < 18) {
      working[0] = binary + 'A';
      snprintf(grid, sizeof(grid), "%s", working);
    } else {
      fprintf(stderr, "Can't decode this grid: %s\n", working);
      status = true;
    }
    if (status) {  // if error occurred
      exit(-1);
    }
  }
  return grid;
}
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
r1 r1::encode(char * displayFormat) {
  // from Karlis Goba (YL3JG) ft8_lib https://github.com/kgoba/ft8_lib
  uint64_t binary = 0;
  char * copy = strdup(displayFormat);
  bool status = false;  // error occurred
  if (strcmp(copy, "/R") == 0) {
    binary = 1;
  } else if (strlen(copy) == 0) {
    binary = 0;
  } else {  // it is not valide
    fprintf(stderr, "Can't encode this suffix: %s\n", copy);
    status = true;
  }
  free(copy);
  if (status) {  // if error occurred
    exit(-1);
  }
  return r1(binary);
}
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
char * r1::decode(void) {
  // from Karlis Goba (YL3JG) ft8_lib https://github.com/kgoba/ft8_lib
  uint64_t binary = 0;
  for (int i = bits - 1; i >= 0; i--) {
    binary |= (fieldBits[i] ? 1:0) << (bits - 1 - i);
  }
  bool status = false;  // error occurred
  char working[3];
  if (binary == 0) {
    working[0] = 0;
  } else if (binary == 1) {
    working[0]= '/';
    working[1]= 'R';
    working[2] = 0;
  } else {
    status = true;
  }
  if (status) {  // if error occurred
    exit(-1);
  }
  snprintf(r1char, sizeof(r1char), "%s", working);
  return r1char;
}
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
R1 R1::encode(char * displayFormat) {
  // from Karlis Goba (YL3JG) ft8_lib https://github.com/kgoba/ft8_lib
  uint64_t binary = 0;
  char * copy = strdup(displayFormat);
  bool status = false;  // error occurred
  if (strcmp(copy, "R") == 0) {
    binary = 1;
  } else if (strlen(copy) == 0) {
    binary = 0;
  } else {  // it is not valid
    fprintf(stderr, "Can't encode R: %s\n", copy);
    status = true;
  }
  free(copy);
  if (status) {  // if error occurred
    exit(-1);
  }
  return R1(binary);
}
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
char * R1::decode(void) {
  // from Karlis Goba (YL3JG) ft8_lib https://github.com/kgoba/ft8_lib
  uint64_t binary = 0;
  for (int i = bits - 1; i >= 0; i--) {
    binary |= (fieldBits[i] ? 1:0) << (bits - 1 - i);
  }
  bool status = false;  // error occurred
  char working[2];
  if (binary == 0) {
    working[0] = 0;
  } else if (binary == 1) {
    working[0]= 'R';
    working[1] = 0;
  } else {
    status = true;
  }
  if (status) {  // if error occurred
    exit(-1);
  }
  snprintf(R1char, sizeof(R1char), "%s", working);
  return R1char;
}
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
i3 i3::encode(char * displayFormat) {
  // from Karlis Goba (YL3JG) ft8_lib https://github.com/kgoba/ft8_lib
  uint64_t binary = 0;
  bool status = false;  // error occurred
  if (strlen(displayFormat) != 1  || displayFormat[0] < '0' || displayFormat[0] > '9') {
    fprintf(stderr, "Can't encode i3: %s\n", displayFormat);
    status = true;
  } else {
    binary = atoi(displayFormat);
  }
  if (status) {  // if error occurred
    exit(-1);
  }
  return i3(binary);
}
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
char * i3::decode(void) {
  // from Karlis Goba (YL3JG) ft8_lib https://github.com/kgoba/ft8_lib
  uint64_t binary = 0;
  for (int i = bits - 1; i >= 0; i--) {
    binary |= (fieldBits[i] ? 1:0) << (bits - 1 - i);
  }
  i3Char[0] = A3[binary];
  i3Char[1] = 0;
  return i3Char;
}
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
n3 n3::encode(char * displayFormat) {
  // from Karlis Goba (YL3JG) ft8_lib https://github.com/kgoba/ft8_lib
  uint64_t binary = 0;
  bool status = false;  // error occurred
  if (strlen(displayFormat) != 1  || displayFormat[0] < '0' || displayFormat[0] > '5') {
    fprintf(stderr, "Can't encode n3: %s\n", displayFormat);
    status = true;
  } else {
    binary = atoi(displayFormat);
    // fprintf(stderr, "i=3 is encoded to %d\n", binary);
  }
  if (status) {  // if error occurred
    exit(-1);
  }
  return n3(binary);
}
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
char * n3::decode(void) {
  // from Karlis Goba (YL3JG) ft8_lib https://github.com/kgoba/ft8_lib
  uint64_t binary = 0;
  for (int i = bits - 1; i >= 0; i--) {
    binary |= (fieldBits[i] ? 1:0) << (bits - 1 - i);
  }
  n3Char[0] = A3[binary];
  n3Char[1] = 0;
  return n3Char;
}
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
char * h12::decode(std::map<uint32_t, char *> * hash12) {
  uint64_t binary = 0;
  for (int i = bits - 1; i >= 0; i--) {
    binary |= (fieldBits[i] ? 1:0) << (bits - 1 - i);
  }
  if ((*hash12).find(binary) != (*hash12).end()) {
    snprintf(callSign, sizeof(callSign), "<%s>", (*hash12)[binary]);
  } else {
    snprintf(callSign, sizeof(callSign), "<hash>");
  }
  return callSign;
}
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
h12 h12::encode(char * displayFormat, std::map<uint32_t, char *> * hash22, std::map<uint32_t, char *> * hash12,
                std::map<uint32_t, char *> * hash10) {
  uint64_t hash = 0;
  uint64_t returnHash = 0;
  bool good = true;
  char working[12];
  memset(working, ' ', sizeof(working)); working[11] = 0;
  uint32_t targetIndex = 0;
  for (uint32_t i = 0; i < strlen(displayFormat); i++) {
    if ((displayFormat[i] == ' ') && (targetIndex == 0)) continue;
    working[targetIndex++] = displayFormat[i];
  }
  for (uint32_t i = 0; i < strlen(working); i++) {
    int32_t index = isIn(working[i], A5);
    if (index < 0) {  // can't hash
      good = false;
      break;
    }
    hash = 38 * hash + index;
  }
  // now trim trailing blanks
  char * tthash = strdup(working);
  for (int32_t i = strlen(tthash) - 1; i >= 0; i--) {
    if (tthash[i] == ' ') {
      tthash[i] = 0;
    } else {
      break;
    }
  }
  if (good) {
    hash *= 47055833459LL;
    hash >>= 64 - 22;
    if ((*hash22).find(hash) == (*hash22).end()) {
      // don't overwrite an existing hash, or if you do, free the memory that has been allocated
      // fprintf(stdout, "new hash of %6.6llx for %s\n", hash, working);
      (*hash22)[hash] = tthash;  // release this memory in destructor and do it first
      hash >>= 10;  // calculate a 12 bit hash
      returnHash = hash;
      (*hash12)[hash] = tthash;  // don't release this memory in destructor
      hash >>= 2;  // calculate a 10 bit hash
      (*hash10)[hash] = tthash;  // don't release this memory in destructor
    } else {
      free(tthash);  //  free memory that isn't used in the hash tables
      // fprintf(stdout, "suppressing hash of %6.6llx for %s\n", hash, working);
    }
  } else {
    free(tthash);  //  free memory that isn't used in the hash tables
    fprintf(stdout, "h12 couldn't encode hash for %s\n", displayFormat);
    exit(-1);
  }
  return h12(returnHash);
}
/* ---------------------------------------------------------------------- */
bool h1::decode(void) {
  uint64_t binary = 0;
  for (int i = bits - 1; i >= 0; i--) {
    binary |= (fieldBits[i] ? 1:0) << (bits - 1 - i);
  }
  bool isSecond = binary == 1;
  return isSecond;
}
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
h1 h1::encode(bool isSecond) {
  uint64_t binary = isSecond ? 1 : 0;
  return h1(binary);
}
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
bool c1::decode(void) {
  uint64_t binary = 0;
  for (int i = bits - 1; i >= 0; i--) {
    binary |= (fieldBits[i] ? 1:0) << (bits - 1 - i);
  }
  bool isCQ = binary == 1;
  return isCQ;
}
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
c1 c1::encode(bool isCQ) {
  uint64_t binary = isCQ ? 1 : 0;
  return c1(binary);
}
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
char * r2::decode(void) {
  uint64_t binary = 0;
  for (int i = bits - 1; i >= 0; i--) {
    binary |= (fieldBits[i] ? 1:0) << (bits - 1 - i);
  }
  if (binary == 1) {
    snprintf(RR, sizeof(RR), "RRR");
  } else if (binary == 2) {
    snprintf(RR, sizeof(RR), "RR73");
  } else if (binary == 3) {
    snprintf(RR, sizeof(RR), "73");
  } else {
    RR[0] = 0;
  }
  return RR;
}
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
r2 r2::encode(char * displayFormat) {
  uint64_t binary = 0;
  if (strcmp(displayFormat, "RRR") == 0) {
    binary = 1;
  } else if (strcmp(displayFormat, "RR73") == 0) {
    binary = 2;
  } else if (strcmp(displayFormat, "73") == 0) {
    binary = 3;
  } else if (strcmp(displayFormat, "") == 0) {
    binary = 0;
  } else {
    fprintf(stderr, "Could not encode %s into r2 field\n", displayFormat);
    exit(-1);
  }
  return r2(binary);
}
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
costas21::costas21(void): FT4FT8Fields(21) {
  setType("costas21");
  fieldBits.push_back(false); fieldBits.push_back(true); fieldBits.push_back(true);    // 3
  fieldBits.push_back(false); fieldBits.push_back(false); fieldBits.push_back(true);   // 1
  fieldBits.push_back(true); fieldBits.push_back(false); fieldBits.push_back(false);   // 4
  fieldBits.push_back(false); fieldBits.push_back(false); fieldBits.push_back(false);  // 0
  fieldBits.push_back(true); fieldBits.push_back(true); fieldBits.push_back(false);    // 6
  fieldBits.push_back(true); fieldBits.push_back(false); fieldBits.push_back(true);    // 5
  fieldBits.push_back(false); fieldBits.push_back(true); fieldBits.push_back(false);   // 2
  int bitIndex = 0;
  int byteIndex = bytes - 1;
  for (int i = bits - 1; i >= 0; i--) {
    fieldBytes[byteIndex] |= (fieldBits[i] ? 1:0) << (bitIndex % 8);
    bitIndex++;
    if ((bitIndex % 8) == 0) byteIndex--;
  }
}
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
payload174::payload174(const FT4FT8Fields & parts): FT4FT8Fields(174) {
  fieldBits = parts.getFieldBits();
  int bitIndex = 0;
  int byteIndex = bytes - 1;
  for (int i = bits - 1; i >= 0; i--) {
    fieldBytes[byteIndex] |= (fieldBits[i] ? 1:0) << (bitIndex % 8);
    bitIndex++;
    if ((bitIndex % 8) == 0) byteIndex--;
  }
  fieldTypes = parts.getFieldTypes();
  fieldTypes.push_back("part1");
  fieldTypes.push_back("part2");
  fieldTypes.push_back("payload174");
  fieldIndices = parts. getFieldIndices();
  fieldIndices.push_back(0);
  fieldIndices.push_back(bits/2);
  fieldIndices.push_back(0);
  fieldSizes = parts.getFieldSizes();
  fieldSizes.push_back(bits/2);
  fieldSizes.push_back(bits/2);
  fieldSizes.push_back(bits);
}
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
payload174::payload174(const std::vector<bool> & data): FT4FT8Fields(174) {
  if (data.size() != 174) {
    fprintf(stderr, "Bit vector (%d) is not a consistent size for a payload (174)\n", data.size());
    exit(-1);
  }
  fieldBits = data;
  int bitIndex = 0;
  int byteIndex = bytes - 1;
  for (int i = bits - 1; i >= 0; i--) {
    fieldBytes[byteIndex] |= (fieldBits[i] ? 1:0) << (bitIndex % 8);
    bitIndex++;
    if ((bitIndex % 8) == 0) byteIndex--;
  }
  fieldTypes.push_back("generic77");
  fieldTypes.push_back("cs14");
  fieldTypes.push_back("ldpc83");
  fieldTypes.push_back("payload174");
  fieldIndices.clear();
  fieldIndices.push_back(0);
  fieldIndices.push_back(77);
  fieldIndices.push_back(91);
  fieldIndices.push_back(0);
  fieldSizes.clear();
  fieldSizes.push_back(77);
  fieldSizes.push_back(14);
  fieldSizes.push_back(83);
  fieldSizes.push_back(174);
}
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
FT8Message237::FT8Message237(const FT4FT8Fields & orig): FT4FT8Fields(237) {
  std::vector<const char *> origTypes = orig.getFieldTypes();
  if (strcmp(origTypes.back(), "payload174") == 0) {  // this is a payload, so make a message from it
    const uint32_t map[]   = {0, 1, 3, 2, 5, 6, 4, 7 };
    unmappedPayloadBits = orig.getFieldBits();
    uint32_t count = 0;
    int triplet = 0;
    for (auto b : unmappedPayloadBits) {
      triplet = (triplet << 1) | (b ? 1:0);
      count++;
      if (count == 3) {
        triplet = map[triplet];
        mappedPayloadBits.push_back((triplet & 0x4) != 0);
        mappedPayloadBits.push_back((triplet & 0x2) != 0);
        mappedPayloadBits.push_back((triplet & 0x1) != 0);
        triplet = 0;
        count = 0;
      }
    }
    std::vector<bool> firstHalf;
    std::vector<bool> secondHalf;
    count = 0;
    uint32_t half = mappedPayloadBits.size() / 2;
    for (auto b : mappedPayloadBits) {
      if (count++ < half) {
        firstHalf.push_back(b);
      } else {
        secondHalf.push_back(b);
      }
    }
    std::vector<const char *> p1;
    std::vector<const char *> p2;
    p1.push_back("part1");
    p2.push_back("part2");
    FT4FT8Fields part1 = FT4FT8Fields(87, firstHalf, p1);
    FT4FT8Fields part2 = FT4FT8Fields(87, secondHalf, p2);
    costas21 costas;
    FT4FT8Fields messageWithCostas = costas + part1 + costas + part2 + costas;
    if (messageWithCostas.getBits() != 237) {
      fprintf(stderr, "Attempting to make a full FT8Message237 object and components don't match\n");
      exit(-1);
    }
    bits = messageWithCostas.getBits();
    bytes = messageWithCostas.getBytes();
    uint8_t * orig = messageWithCostas.getFieldBytes();
    for (uint32_t i = 0; i < bytes; i++) {
      fieldBytes[i] = *orig++;
    }
    fieldBits = messageWithCostas.getFieldBits();
    fieldTypes = messageWithCostas.getFieldTypes();
    fieldTypes.push_back("FT8Message237");
    fieldSizes = messageWithCostas.getFieldSizes();
    fieldSizes.push_back(bits);
    fieldIndices = messageWithCostas.getFieldIndices();
    fieldIndices.push_back(0);
  } else if ((strcmp(origTypes.back(), "FT8Message237") == 0) || (orig.getBits() == 237)) {  // this is a message
    const uint32_t unmap[] = {0, 1, 3, 2, 6, 4, 5, 7 };
    FT4FT8Fields copyOrig = orig;
    mappedPayloadBits = copyOrig("part1", 0).getFieldBits();
    for (auto b : copyOrig("part2", 0).getFieldBits()) {
      mappedPayloadBits.push_back(b);
    }
    uint32_t count = 0;
    int triplet = 0;
    for (auto b : mappedPayloadBits) {
      triplet = (triplet << 1) | (b ? 1:0);
      count++;
      if (count == 3) {
        triplet = unmap[triplet];
        unmappedPayloadBits.push_back((triplet & 0x4) != 0);
        unmappedPayloadBits.push_back((triplet & 0x2) != 0);
        unmappedPayloadBits.push_back((triplet & 0x1) != 0);
        triplet = 0;
        count = 0;
      }
    }
    std::vector<bool> firstHalf;
    std::vector<bool> secondHalf;
    count = 0;
    uint32_t half = mappedPayloadBits.size() / 2;
    for (auto b : mappedPayloadBits) {
      if (count++ < half) {
        firstHalf.push_back(b);
      } else {
        secondHalf.push_back(b);
      }
    }
    std::vector<const char *> p1;
    std::vector<const char *> p2;
    p1.push_back("part1");
    p2.push_back("part2");
    FT4FT8Fields part1 = FT4FT8Fields(87, firstHalf, p1);
    FT4FT8Fields part2 = FT4FT8Fields(87, secondHalf, p2);
    costas21 costas;
    FT4FT8Fields messageWithCostas = costas + part1 + costas + part2 + costas;
    if (messageWithCostas.getBits() != 237) {
      fprintf(stderr, "Attempting to make a full FT8Message237 object and components don't match\n");
      exit(-1);
    }
    bits = messageWithCostas.getBits();
    bytes = messageWithCostas.getBytes();
    uint8_t * orig = messageWithCostas.getFieldBytes();
    for (uint32_t i = 0; i < bytes; i++) {
      fieldBytes[i] = *orig++;
    }
    fieldBits = messageWithCostas.getFieldBits();
    fieldTypes = messageWithCostas.getFieldTypes();
    fieldTypes.push_back("FT8Message237");
    fieldSizes = messageWithCostas.getFieldSizes();
    fieldSizes.push_back(bits);
    fieldIndices = messageWithCostas.getFieldIndices();
    fieldIndices.push_back(0);
  } else {
    fprintf(stderr, "Input is not compatible with FT8Message237\n");
    exit(-1);
  }
}
/* ---------------------------------------------------------------------- */
// #define SELFTEST 0
#ifdef SELFTEST
#include <getopt.h>
const char USAGE_STR[] = "%s --call_sign <KG5YJE>\n";
const struct option longOpts[] = {
                                  { "call_sign",     required_argument, NULL, 1 },
                                  { "grid_location", required_argument, NULL, 2 },
                                  {0,                0,                 0,    0 }
};
int main(int argc, char *argv[]) {
  int c = 0;
  char cs[100];
  char g[100] = "EM13";
  snprintf(cs, sizeof(cs), "KG5YJE");
  snprintf(g, sizeof(g), "EM13");
  while ((c = getopt_long(argc, argv, "h", longOpts, NULL)) >= 0) {
    switch (c) {
    case 'h' : {
      fprintf(stderr, USAGE_STR, argv[0]);
      return -2;
    }
    case 1: {
      fprintf(stderr, "got call_sign: %s\n", optarg);
      snprintf(cs, sizeof(cs), "%s", optarg);
      break;
    }
    case 2: {
      fprintf(stderr, "got grid_location: %s\n", optarg);
      snprintf(g, sizeof(g), "%s", optarg);
      break;
    }
    default:
      return -2;
    }
  }
  char cq[] = "CQ";
  char empty[] = "";
  FT4FT8Fields f1 = c28::encode(cq);
  FT4FT8Fields f2 = r1::encode(empty);
  FT4FT8Fields f3 = c28::encode(cs);
  FT4FT8Fields f4 = r1::encode(empty);
  FT4FT8Fields f5 = R1::encode(empty);
  FT4FT8Fields f6 = g15::encode(g);
  char one[] = "1";
  FT4FT8Fields f7 = i3::encode(one);
  FT4FT8Fields type1 = f1 + f2 + f3 + f4 + f5 + f6 + f7;
  FT4FT8Fields f8 = cs14(FT4FT8Utilities::crc(type1.getField()));
  FT4FT8Fields type1Pcs = type1 + f8;
  FT4FT8Fields ldpcPart = ldpc83(FT4FT8Utilities::ldpc(type1Pcs.getField()));
  payload174 fullPayload = payload174(type1Pcs + ldpcPart);
  fullPayload.print();
  fullPayload.toOctal();
  costas21 costas;
  costas.print();
  costas.toOctal();
  FT8Message237 messageWithCostas = FT8Message237(fullPayload);
  messageWithCostas.print();
  messageWithCostas.toOctal();
  FT8Message237 corruptedMessage = FT8Message237(messageWithCostas);
  corruptedMessage.print();
  corruptedMessage.toOctal();
  std::map<uint32_t, std::vector<uint32_t>> possibleBits;
  std::vector<bool> cm = corruptedMessage.getUnmappedPayloadBits();
  // this loop should not have any failures
  for (uint32_t index = 0; index < 83; index++) {
    if (!FT4FT8Utilities::checkLdpc(cm, index, &possibleBits)) {
      fprintf(stderr, "LDPC parity check failed at index: %d\n", index);
    }
  }
  // corrupt bits
  cm[90] = !cm[90];
  cm[92] = cm[90];
  cm[93] = cm[90];
  cm[94] = cm[90];
  cm[95] = cm[90];

  std::vector<bool> revert = cm;
  uint32_t score = FT4FT8Utilities::scoreLdpc(cm, &possibleBits);
  fprintf(stderr, "Score of corrupted message %d\n", score);
  // toggle a bit and see if score changes
  for (uint32_t bl = 1; bl <  7; bl++) {
    fprintf(stderr, "working on %d bit error\n", bl);
    for (uint32_t b = 0; b < 174 - bl; b++) {
      uint32_t patternSize = powf(2.0, bl);
      for (uint32_t pattern = 0; pattern < patternSize; pattern++) {
        for (uint32_t shifts = 0; shifts < bl; shifts++) {
          cm[b+shifts] = pattern & (0x1 << shifts);
        }
        score = FT4FT8Utilities::fastScoreLdpc(cm);
        if (score == 83) {
          fprintf(stderr, "Errors may have been corrected at bit position %d to %d\n", b, b+bl-1);
          break;
        }
        cm = revert;
      }
      if (score == 83) break;
    }
    if (score == 83) break;
  }
  fprintf(stderr, "final score: %d, %s\n", score, score == 83? "successful":"failed");
  payload174 extractedPayload = payload174(cm);
  extractedPayload.print();
  extractedPayload.toOctal();

  // read in real vectors and see how the above algorithm works, these vectors are unmapped (not gray coded)

  const uint32_t testVectors[][58] = {
                                      { 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 3, 3, 7, 6, 1, 4, 2, 3, 0, 4, 0, 6, 7, 2, 2,
                                        0, 0, 0, 2, 2, 2, 5, 3, 1, 3, 5, 0, 3, 6, 6, 7, 3, 7, 6, 4, 1, 5, 1, 3, 1, 5,
                                        5, 4, 6, 2, 1, 0 },
                                      { 6,  0,  6,  5,  3,  3,  1,  3,  7,  1,  2,  0,  3,  5,  6,  7,  2,  6,  2,  1,
                                        7,  2,  3,  5,  2,  2,  5,  2,  3,  6,  2,  2,  0,  0,  0,  6,  2,  7,  5,  3,
                                        5,  2,  7,  7,  1,  6,  1,  0,  5,  4,  0,  3,  2,  0,  5,  4,  2,  0 },
                                      { 1,  0,  0,  0,  0,  0,  0,  0,  1,  0,  0,  6,  0,  3,  7,  6,  5,  0,  3,  1,
                                        2,  2,  4,  0,  5,  2,  1,  4,  6,  0,  3,  1,  4,  4,  1,  3,  3,  5,  5,  4,
                                        3,  5,  0,  2,  3,  6,  2,  6,  1,  6,  7,  0,  6,  3,  4,  6,  1,  6 },
                                      { 3,  0,  0,  7,  4,  6,  7,  0,  3,  4,  0,  5,  1,  2,  6,  7,  3,  3,  1,  1,
                                        7,  6,  5,  4,  0,  2,  2,  2,  3,  3,  3,  5,  5,  6,  1,  3,  5,  2,  2,  2,
                                        1,  2,  1,  1,  7,  4,  6,  4,  4,  3,  0,  2,  7,  7,  6,  1,  7,  6 },
                                      { 7,  3,  4,  6,  3,  1,  1,  3,  6,  0,  0,  5,  3,  5,  6,  7,  2,  7,  0,  1,
                                        3,  1,  6,  4,  0,  2,  1,  3,  2,  1,  5,  4,  5,  1,  7,  2,  3,  6,  4,  2,
                                        3,  0,  0,  6,  4,  6,  4,  6,  6,  1,  7,  7,  2,  6,  6,  7,  3,  2 },
                                      { 3,  0,  0,  7,  4,  6,  7,  0,  3,  4,  0,  4,  0,  3,  6,  7,  5,  3,  1,  1,
                                        7,  6,  3,  5,  2,  2,  4,  7,  0,  2,  2,  5,  1,  6,  7,  6,  2,  1,  1,  7,
                                        7,  1,  3,  7,  7,  1,  6,  5,  2,  5,  6,  6,  6,  2,  6,  0,  3,  7 },
                                      { 0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  6,  5,  5,  7,  2,  0,  4,  4,  3,  1,
                                        1,  1,  7,  5,  4,  2,  7,  7,  0,  2,  2,  2,  0,  5,  0,  4,  5,  3,  1,  5,
                                        7,  5,  4,  4,  7,  4,  3,  5,  3,  2,  4,  5,  3,  6,  5,  2,  3,  2 },
                                      { 7,  3,  4,  6,  2,  1,  1,  3,  6,  0,  0,  5,  1,  5,  4,  7,  2,  5,  6,  3,
                                        7,  4,  5,  3,  2,  2,  4,  1,  6,  2,  0,  6,  5,  6,  6,  7,  7,  4,  7,  6,
                                        3,  3,  1,  7,  0,  0,  1,  6,  4,  3,  3,  0,  3,  6,  5,  4,  1,  1 },
                                      { 0,  0,  0,  0,  1,  0,  0,  0,  3,  0,  0,  6,  1,  3,  7,  2,  7,  1,  3,  3,
                                        6,  3,  4,  0,  4,  3,  1,  4,  6,  0,  3,  0,  4,  5,  2,  3,  3,  5,  7,  6,
                                        3,  5,  0,  2,  3,  6,  2,  6,  1,  6,  7,  0,  6,  3,  4,  2,  1,  6 },
                                      { 7,  7,  5,  6,  0,  4,  3,  0,  3,  4,  1,  4,  7,  2,  1,  3,  1,  2,  1,  1,
                                        7,  6,  4,  5,  0,  3,  6,  2,  2,  3,  4,  3,  3,  7,  5,  2,  1,  4,  1,  5,
                                        1,  7,  2,  6,  5,  3,  2,  2,  7,  4,  3,  1,  4,  6,  3,  7,  5,  1 },
                                      { 7,  7,  5,  6,  0,  4,  3,  0,  3,  4,  1,  4,  7,  2,  1,  3,  1,  2,  1,  1,
                                        7,  6,  4,  5,  0,  3,  6,  2,  2,  3,  4,  3,  3,  7,  5,  2,  1,  4,  1,  5,
                                        1,  7,  2,  6,  5,  3,  2,  2,  7,  4,  3,  1,  4,  6,  3,  7,  7,  7 },
                                      { 1,  0,  4,  4,  7,  7,  4,  5,  0,  0,  0,  5,  1,  2,  6,  3,  3,  2,  7,  1,
                                        7,  6,  4,  4,  2,  3,  4,  4,  2,  6,  5,  0,  3,  2,  7,  1,  4,  6,  6,  6,
                                        5,  7,  3,  3,  1,  7,  3,  4,  7,  1,  3,  4,  0,  0,  3,  0,  6,  0 } };

  uint32_t rows = sizeof(testVectors)/sizeof(uint32_t) / 58;
  std::map<uint32_t, char *> hash22;
  std::map<uint32_t, char *> hash12;
  std::map<uint32_t, char *> hash10;
  for (uint32_t r = 0; r < rows; r++) {
    std::vector<bool> tV;
    for (uint32_t c = 0; c < 58; c++) {
      uint32_t value = testVectors[r][c];
      tV.push_back(value & 0x4);
      tV.push_back(value & 0x2);
      tV.push_back(value & 0x1);
    }
    std::vector<bool> origTV = tV;
    std::vector<bool> resultTV;
    uint32_t score = FT4FT8Utilities::ldpcDecode(origTV, 15, &resultTV);
    fprintf(stderr, "Candidate %d, score: %d\n", r, score);
    if (score == 83) {
      payload174 payload = payload174(resultTV);  // create a payload object with the corrected vector
      if (FT4FT8Utilities::crc(payload("generic77", 0, true)) == payload("cs14", 0, true)) {
        fprintf(stderr, "Checksums match\n");
        std::vector<bool> i0 = FT4FT8Fields::overlay(MESSAGE_TYPES::type1, payload, "i3", 0);
        i3 mI3 = i3(i0);
        fprintf(stderr, "Message type (i3): %s\n", mI3.decode());
        if (strcmp(mI3.decode(), "1") == 0) {
          std::vector<bool> b0 = FT4FT8Fields::overlay(MESSAGE_TYPES::type1, payload, "c28", 0);
          c28 receivedCS = c28(b0);
          std::vector<bool> b1 = FT4FT8Fields::overlay(MESSAGE_TYPES::type1, payload, "c28", 1);
          c28 senderCS = c28(b1);
          std::vector<bool> l0 = FT4FT8Fields::overlay(MESSAGE_TYPES::type1, payload, "g15", 0);
          g15 location = g15(l0);
          fprintf(stderr, "%s %s %s\n", receivedCS.decode(&hash22, &hash12, &hash10),
                  senderCS.decode(&hash22, &hash12, &hash10), location.decode());
        } else {
          fprintf(stderr, "decode of message type %s is not supported yet.\n", mI3.decode());
        }
      } else {
        fprintf(stderr, "Checksums don't match\n");
      }
    }
  }

  // tests for type 4 messages
  char example[] = "ISAQU";
  FT4FT8Fields f10 = h12::encode(example, &hash22, &hash12, &hash10);
  f10.print();
  char c58cs[] = "KG5YJE/1";
  FT4FT8Fields f11 = c58::encode(c58cs);
  fprintf(stderr, "f11 with c58cs\n");
  f11.print();
  FT4FT8Fields f12 = h1::encode(false);
  fprintf(stderr, "f12 with false\n");
  f12.print();
  char r2field[] = "RR73";
  FT4FT8Fields f13 = r2::encode(r2field);
  fprintf(stderr, "f13 with RR73\n");
  f13.print();
  FT4FT8Fields f14 = c1::encode(true);
  fprintf(stderr, "f14 with c1 true\n");
  f14.print();
  FT4FT8Fields type4 = f10 + f11 + f12 + f13 + f14;
  fprintf(stderr, "type4 message\n");
  type4.print();
  char msg[128];
  std::vector<bool> b0 = FT4FT8Fields::overlay(MESSAGE_TYPES::type4, type4,
                                               "h12", 0);
  h12 hashedCS = h12(b0);
  std::vector<bool> b1 = FT4FT8Fields::overlay(MESSAGE_TYPES::type4, type4,
                                               "c58", 0);
  c58 extendedCS = c58(b1);
  extendedCS.print();
  std::vector<bool> b2 = FT4FT8Fields::overlay(MESSAGE_TYPES::type4, type4,
                                               "h1", 0);
  h1 hashIsSecond = h1(b2);
  std::vector<bool> b3 = FT4FT8Fields::overlay(MESSAGE_TYPES::type4, type4,
                                               "r2", 0);
  r2 extra = r2(b3);
  std::vector<bool> b4 = FT4FT8Fields::overlay(MESSAGE_TYPES::type4, type4,
                                               "c1", 0);
  c1 firstIsCQ = c1(b4);
  if (firstIsCQ.decode()) {  // if first is CQ ignore hash field and extra
    snprintf(msg, sizeof(msg), "CQ %s" , extendedCS.decode());
  } else {
    if (hashIsSecond.decode()) {  // flip the order of the call signs
      snprintf(msg, sizeof(msg), "%s %s %s", extendedCS.decode(),
               hashedCS.decode(&hash12), extra.decode());
    } else {
      snprintf(msg, sizeof(msg), "%s %s %s",
               hashedCS.decode(&hash12), extendedCS.decode(), extra.decode());
    }
  }
  fprintf(stderr, "Decoded type 4 message: %s\n", msg);
  return 0;
}
#endif
