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
    fprintf(stderr, "data (%ld) can't fit in %d bits\n", data, bits);
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
    fprintf(stderr, "data (%ld) can't fit in %d bits\n", data, bits);
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
    fprintf(stderr, "data vector of size %ld, can't fit in %d bits\n", data.size(), bits);
    exit(-1);
  }
  this->bits = bits;
  this->bytes = bits /8 + (((bits % 8) == 0) ? 0:1);
  for (int i = 0; i < bits; i++) {
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
    fprintf(stderr, "data vector of size %ld, can't fit in %d bits\n", data.size(), bits);
    exit(-1);
  }
  if (fields.size() != 1) {
    fprintf(stderr, "There can only be one field defined in the vector for this constructor\n");
    exit(-1);
  }
  this->bits = bits;
  this->bytes = bits /8 + (((bits % 8) == 0) ? 0:1);
  for (int i = 0; i < bits; i++) {
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
    fprintf(stderr, "data vector of size %ld, can't fit in %d bits\n", data.size(), bits);
    exit(-1);
  }
  this->bits = bits;
  this->bytes = bits /8 + (((bits % 8) == 0) ? 0:1);
  for (int i = 0; i < bits; i++) {
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
  for (int i = 0; i < bytes; i++) {
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
  for (int i = 0; i < bytes; i++) {
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
    for (int i = 0; i < newOne.bytes; i++) {
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
  for (int i = 0; i < bytes; i++) {
    fprintf(stderr, " %2.2x", fieldBytes[i]);
  }
  fprintf(stderr, "\n");
  if (fieldBits.size() == bits) {
    for (int i = 0; i < bits; i++) {
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
    for (int i = 0; i < bits; i++) {
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
    for (int i = 0; i < bits; i++) {
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
  for (int i = 0; i < rhs.bytes; i++) {
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
const char A1[] = " 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const char A2[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const char A3[] = "0123456789";
const char A4[] = " ABCDEFGHIJKLMNOPQRSTUVWXYZ";
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- */
uint32_t FT4FT8Fields::isIn(char b, const char * a) {
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
char * c28::decode(void) {
  // from Karlis Goba (YL3JG) ft8_lib https://github.com/kgoba/ft8_lib
  uint64_t binary = 0;
  for (int i = bits - 1; i >= 0; i--) {
    binary |= (fieldBits[i] ? 1:0) << (bits - 1 - i);
  }
  bool status = false;  // error occurred
  if (binary == 0) {
    snprintf(callSign, sizeof(callSign), "DE");
  } else if (binary == 1) {
    snprintf(callSign, sizeof(callSign), "QRZ");
  } else if (binary == 2) {
    snprintf(callSign, sizeof(callSign), "CQ");
  } else {  // see if standard call sign
    char working[7];
    memset(working, ' ', 6); working[6] = 0;
    if (binary > 2063592 + 4194304) {
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
      snprintf(callSign, sizeof(callSign), working);
    } else {
      fprintf(stderr, "Can't decode this callsign: %s\n", working);
      status = true;
    }
  }
  if (status) {  // if error occurred
    exit(-1);
  }
  return callSign;
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
  working[3] = binary % 10 + '0';
  binary /= 10;
  working[2] = binary % 10 +'0';
  binary /= 10;
  working[1] = binary % 18 + 'A';
  binary /= 18;
  if (binary < 18) {
    working[0] = binary + 'A';
    snprintf(grid, sizeof(grid), working);
  } else {
    fprintf(stderr, "Can't decode this grid: %s\n", working);
    status = true;
  }
  if (status) {  // if error occurred
    exit(-1);
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
  snprintf(r1char, sizeof(r1char), working);
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
  snprintf(R1char, sizeof(R1char), working);
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
    int count = 0;
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
    int count = 0;
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
      snprintf(cs, sizeof(cs), optarg);
      break;
    }
    case 2: {
      fprintf(stderr, "got grid_location: %s\n", optarg);
      snprintf(g, sizeof(g), optarg);
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
  // this loop should not any failures
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
      } else {
        fprintf(stderr, "Checksums don't match\n");
      }
    }
  }

  return 0;
}
