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
  fprintf(stderr, "in base constructor, setting bits and fieldBytes %p\n", this);
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
  fprintf(stderr, "in base class constructor, setting input: %d bits, %llu data %p\n", bits, data, this);
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
  fprintf(stderr, "in base class constructor, setting input: %d bits, %llu data %p\n", bits, data, this);
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
  fprintf(stderr, "in base class constructor, setting input: %d bits, vector of boolean data size: %d %p\n", bits,
          data.size(), this);
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
  fprintf(stderr, "in base class constructor, setting input: %d bits, vector of boolean data size: %d %p\n", bits,
          data.size(), this);
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
  fprintf(stderr, "in base class constructor, setting input: %d bits, vector of boolean data size: %d %p\n", bits,
          data.size(), this);
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
  fprintf(stderr, "Field object destroyed %p\n", this);
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
  fprintf(stderr, "const Field object copied %p\n", this);
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
  fprintf(stderr, "const c28 Field object copied %p\n", this);
}
/* ---------------------------------------------------------------------- */
/* ---------------------------------------------------------------------- 
c28::c28(c28& orig) {
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
        fprintf(stderr, "%d ", FT4FT8Fields::toGray(octet));
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
std::vector<bool>  FT4FT8Fields::crc(std::vector<bool> message) {
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
std::vector<bool>  FT4FT8Fields::ldpc(std::vector<bool> message) {
  const std::vector<bool> ldpc_generator[83] =
    { { true, false, false, false, false, false, true, true,
        false, false, true, false, true, false, false, true,
        true, true, false, false, true, true, true, false,
        false, false, false, true, false, false, false, true,
        true, false, true, true, true, true, true, true,
        false, false, true, true, false, false, false, true,
        true, true, true, false, true, false, true, false,
        true, true, true, true, false, true, false, true,
        false, false, false, false, true, false, false, true,
        true, true, true, true, false, false, true, false,
        false, true, true, true, true, true, true, true,
        true, true, false, false },
      { false, true, true, true, false, true, true, false,
        false, false, false, true, true, true, false, false,
        false, false, true, false, false, true, true, false,
        false, true, false, false, true, true, true, false,
        false, false, true, false, false, true, false, true,
        true, true, false, false, false, false, true, false,
        false, true, false, true, true, false, false, true,
        false, false, true, true, false, false, true, true,
        false, true, false, true, false, true, false, false,
        true, false, false, true, false, false, true, true,
        false, false, false, true, false, false, true, true,
        false, false, true, false },
      { true, true, false, true, true, true, false, false,
        false, false, true, false, false, true, true, false,
        false, true, false, true, true, false, false, true,
        false, false, false, false, false, false, true, false,
        true, true, true, true, true, false, true, true,
        false, false, true, false, false, true, true, true,
        false, true, true, true, true, true, false, false,
        false, true, true, false, false, true, false, false,
        false, false, false, true, false, false, false, false,
        true, false, true, false, false, false, false, true,
        true, false, true, true, true, true, false, true,
        true, true, false, false },
      { false, false, false, true, true, false, true, true,
        false, false, true, true, true, true, true, true,
        false, true, false, false, false, false, false, true,
        false, true, true, true, true, false, false, false,
        false, true, false, true, true, false, false, false,
        true, true, false, false, true, true, false, true,
        false, false, true, false, true, true, false, true,
        true, true, false, true, false, false, true, true,
        false, false, true, true, true, true, true, false,
        true, true, false, false, false, true, true, true,
        true, true, true, true, false, true, true, false,
        false, false, true, false },
      { false, false, false, false, true, false, false, true,
        true, true, true, true, true, true, false, true,
        true, false, true, false, false, true, false, false,
        true, true, true, true, true, true, true, false,
        true, true, true, false, false, false, false, false,
        false, true, false, false, false, false, false, true,
        true, false, false, true, false, true, false, true,
        true, true, true, true, true, true, false, true,
        false, false, false, false, false, false, true, true,
        false, true, false, false, false, true, true, true,
        true, false, false, false, false, false, true, true,
        true, false, true, false },
      { false, false, false, false, false, true, true, true,
        false, true, true, true, true, true, false, false,
        true, true, false, false, true, true, false, false,
        true, true, false, false, false, false, false, true,
        false, false, false, true, true, false, true, true,
        true, false, false, false, true, false, false, false,
        false, true, true, true, false, false, true, true,
        true, true, true, false, true, true, false, true,
        false, true, false, true, true, true, false, false,
        false, false, true, true, true, true, false, true,
        false, true, false, false, true, false, false, false,
        true, false, true, false },
      { false, false, true, false, true, false, false, true,
        true, false, true, true, false, true, true, false,
        false, false, true, false, true, false, true, false,
        true, true, true, true, true, true, true, false,
        false, false, true, true, true, true, false, false,
        true, false, true, false, false, false, false, false,
        false, false, true, true, false, true, true, false,
        true, true, true, true, false, true, false, false,
        true, true, true, true, true, true, true, false,
        false, false, false, true, true, false, true, false,
        true, false, false, true, true, true, false, true,
        true, false, true, false },
      { false, true, true, false, false, false, false, false,
        false, true, false, true, false, true, false, false,
        true, true, true, true, true, false, true, false,
        true, true, true, true, false, true, false, true,
        true, true, true, true, false, false, true, true,
        false, true, false, true, true, true, false, true,
        true, false, false, true, false, true, true, false,
        true, true, false, true, false, false, true, true,
        true, false, true, true, false, false, false, false,
        true, true, false, false, true, false, false, false,
        true, true, false, false, false, false, true, true,
        true, true, true, false },
      { true, true, true, false, false, false, true, false,
        false, false, false, false, false, true, true, true,
        true, false, false, true, true, false, false, false,
        true, true, true, false, false, true, false, false,
        false, false, true, true, false, false, false, true,
        false, false, false, false, true, true, true, false,
        true, true, true, false, true, true, false, true,
        false, false, true, false, false, true, true, true,
        true, false, false, false, true, false, false, false,
        false, true, false, false, true, false, true, false,
        true, true, true, false, true, false, false, true,
        false, false, false, false },
      { false, true, true, true, false, true, true, true,
        false, true, false, true, true, true, false, false,
        true, false, false, true, true, true, false, false,
        false, false, false, false, true, false, false, false,
        true, true, true, false, true, false, false, false,
        false, false, false, false, true, true, true, false,
        false, false, true, false, false, true, true, false,
        true, true, false, true, true, true, false, true,
        true, false, true, false, true, true, true, false,
        false, true, false, true, false, true, true, false,
        false, false, true, true, false, false, false, true,
        true, false, false, false },
      { true, false, true, true, false, false, false, false,
        true, false, true, true, true, false, false, false,
        false, false, false, true, false, false, false, true,
        false, false, false, false, false, false, true, false,
        true, false, false, false, true, true, false, false,
        false, false, true, false, true, false, true, true,
        true, true, true, true, true, false, false, true,
        true, false, false, true, false, true, true, true,
        false, false, true, false, false, false, false, true,
        false, false, true, true, false, true, false, false,
        true, false, false, false, false, true, true, true,
        true, true, false, false },
      { false, false, false, true, true, false, false, false,
        true, false, true, false, false, false, false, false,
        true, true, false, false, true, false, false, true,
        false, false, true, false, false, false, true, true,
        false, false, false, true, true, true, true, true,
        true, true, false, false, false, true, true, false,
        false, false, false, false, true, false, true, false,
        true, true, false, true, true, true, true, true,
        false, true, false, true, true, true, false, false,
        false, true, false, true, true, true, true, false,
        true, false, true, false, false, false, true, true,
        false, false, true, false },
      { false, true, true, true, false, true, true, false,
        false, true, false, false, false, true, true, true,
        false, false, false, true, true, true, true, false,
        true, false, false, false, false, false, true, true,
        false, false, false, false, false, false, true, false,
        true, false, true, false, false, false, false, false,
        false, true, true, true, false, false, true, false,
        false, false, false, true, true, true, true, false,
        false, false, false, false, false, false, false, true,
        true, false, true, true, false, false, false, true,
        false, false, true, false, true, false, true, true,
        true, false, false, false },
      { true, true, true, true, true, true, true, true,
        true, false, true, true, true, true, false, false,
        true, true, false, false, true, false, true, true,
        true, false, false, false, false, false, false, false,
        true, true, false, false, true, false, true, false,
        true, false, false, false, false, false, true, true,
        false, true, false, false, false, false, false, true,
        true, true, true, true, true, false, true, false,
        true, true, true, true, true, false, true, true,
        false, true, false, false, false, true, true, true,
        true, false, true, true, false, false, true, false,
        true, true, true, false },
      { false, true, true, false, false, true, true, false,
        true, false, true, false, false, true, true, true,
        false, false, true, false, true, false, true, false,
        false, false, false, true, false, true, false, true,
        true, false, false, false, true, true, true, true,
        true, false, false, true, false, false, true, true,
        false, false, true, false, false, true, false, true,
        true, false, true, false, false, false, true, false,
        true, false, true, true, true, true, true, true,
        false, true, true, false, false, true, true, true,
        false, false, false, true, false, true, true, true,
        false, false, false, false },
      { true, true, false, false, false, true, false, false,
        false, false, true, false, false, true, false, false,
        false, false, true, true, false, true, true, false,
        true, false, false, false, true, false, false, true,
        true, true, true, true, true, true, true, false,
        true, false, false, false, false, true, false, true,
        true, false, true, true, false, false, false, true,
        true, true, false, false, false, true, false, true,
        false, false, false, true, false, false, true, true,
        false, true, true, false, false, false, true, true,
        true, false, true, false, false, false, false, true,
        true, false, false, false },
      { false, false, false, false, true, true, false, true,
        true, true, true, true, true, true, true, true,
        false, true, true, true, false, false, true, true,
        true, false, false, true, false, true, false, false,
        false, false, false, true, false, true, false, false,
        true, true, false, true, false, false, false, true,
        true, false, true, false, false, false, false, true,
        true, false, true, true, false, false, true, true,
        false, true, false, false, true, false, true, true,
        false, false, false, true, true, true, false, false,
        false, false, true, false, false, true, true, true,
        false, false, false, false },
      { false, false, false, true, false, true, false, true,
        true, false, true, true, false, true, false, false,
        true, false, false, false, true, false, false, false,
        false, false, true, true, false, false, false, false,
        false, true, true, false, false, false, true, true,
        false, true, true, false, true, true, false, false,
        true, false, false, false, true, false, true, true,
        true, false, false, true, true, false, false, true,
        true, false, false, false, true, false, false, true,
        false, true, false, false, true, false, false, true,
        false, true, true, true, false, false, true, false,
        true, true, true, false },
      { false, false, true, false, true, false, false, true,
        true, false, true, false, true, false, false, false,
        true, false, false, true, true, true, false, false,
        false, false, false, false, true, true, false, true,
        false, false, true, true, true, true, false, true,
        true, true, true, false, true, false, false, false,
        false, false, false, true, true, true, false, true,
        false, true, true, false, false, true, true, false,
        false, true, false, true, false, true, false, false,
        true, false, false, false, true, false, false, true,
        true, false, true, true, false, false, false, false,
        true, true, true, false },
      { false, true, false, false, true, true, true, true,
        false, false, false, true, false, false, true, false,
        false, true, true, false, true, true, true, true,
        false, false, true, true, false, true, true, true,
        true, true, true, true, true, false, true, false,
        false, true, false, true, false, false, false, true,
        true, true, false, false, true, false, true, true,
        true, true, true, false, false, true, true, false,
        false, false, false, true, true, false, true, true,
        true, true, false, true, false, true, true, false,
        true, false, true, true, true, false, false, true,
        false, true, false, false },
      { true, false, false, true, true, false, false, true,
        true, true, false, false, false, true, false, false,
        false, true, true, true, false, false, true, false,
        false, false, true, true, true, false, false, true,
        true, true, false, true, false, false, false, false,
        true, true, false, true, true, false, false, true,
        false, true, true, true, true, true, false, true,
        false, false, true, true, true, true, false, false,
        true, false, false, false, false, true, false, false,
        true, true, true, false, false, false, false, false,
        true, false, false, true, false, true, false, false,
        false, false, false, false },
      { false, false, false, true, true, false, false, true,
        false, false, false, true, true, false, false, true,
        true, false, true, true, false, true, true, true,
        false, true, false, true, false, false, false, true,
        false, false, false, true, true, false, false, true,
        false, true, true, true, false, true, true, false,
        false, true, false, true, false, true, true, false,
        false, false, true, false, false, false, false, true,
        true, false, true, true, true, false, true, true,
        false, true, false, false, true, true, true, true,
        false, false, false, true, true, true, true, false,
        true, false, false, false },
      { false, false, false, false, true, false, false, true,
        true, true, false, true, true, false, true, true,
        false, false, false, true, false, false, true, false,
        true, true, false, true, false, true, true, true,
        false, false, true, true, false, false, false, true,
        true, true, true, true, true, false, true, false,
        true, true, true, false, true, true, true, false,
        false, false, false, false, true, false, true, true,
        true, false, false, false, false, true, true, false,
        true, true, false, true, true, true, true, true,
        false, true, true, false, true, false, true, true,
        true, false, false, false },
      { false, true, false, false, true, false, false, false,
        true, false, false, false, true, true, true, true,
        true, true, false, false, false, false, true, true,
        false, false, true, true, true, true, false, true,
        true, true, true, true, false, true, false, false,
        false, false, true, true, true, true, true, true,
        true, false, true, true, true, true, false, true,
        true, true, true, false, true, true, true, false,
        true, false, true, false, false, true, false, false,
        true, true, true, false, true, false, true, false,
        true, true, true, true, true, false, true, true,
        false, true, false, false },
      { true, false, false, false, false, false, true, false,
        false, true, true, true, false, true, false, false,
        false, false, true, false, false, false, true, true,
        true, true, true, false, true, true, true, false,
        false, true, false, false, false, false, false, false,
        true, false, true, true, false, true, true, false,
        false, true, true, true, false, true, false, true,
        true, true, true, true, false, true, true, true,
        false, true, false, true, false, true, true, false,
        true, true, true, false, true, false, true, true,
        false, true, false, true, true, true, true, true,
        true, true, true, false },
      { true, false, true, false, true, false, true, true,
        true, true, true, false, false, false, false, true,
        true, false, false, true, false, true, true, true,
        true, true, false, false, false, true, false, false,
        true, false, false, false, false, true, false, false,
        true, true, false, false, true, false, true, true,
        false, true, true, true, false, true, false, false,
        false, true, true, true, false, true, false, true,
        false, true, true, true, false, false, false, true,
        false, true, false, false, false, true, false, false,
        true, false, true, false, true, false, false, true,
        true, false, true, false },
      { false, false, true, false, true, false, true, true,
        false, true, false, true, false, false, false, false,
        false, false, false, false, true, true, true, false,
        false, true, false, false, true, false, true, true,
        true, true, false, false, false, false, false, false,
        true, true, true, false, true, true, false, false,
        false, true, false, true, true, false, true, false,
        false, true, true, false, true, true, false, true,
        false, false, true, false, true, false, true, true,
        true, true, false, true, true, false, true, true,
        true, true, false, true, true, true, false, true,
        false, false, false, false },
      { true, true, false, false, false, true, false, false,
        false, true, true, true, false, true, false, false,
        true, false, true, false, true, false, true, false,
        false, true, false, true, false, false, true, true,
        true, true, false, true, false, true, true, true,
        false, false, false, false, false, false, true, false,
        false, false, false, true, true, false, false, false,
        false, true, true, true, false, true, true, false,
        false, false, false, true, false, true, true, false,
        false, true, true, false, true, false, false, true,
        false, false, true, true, false, true, true, false,
        false, false, false, false },
      { true, false, false, false, true, true, true, false,
        true, false, true, true, true, false, true, false,
        false, false, false, true, true, false, true, false,
        false, false, false, true, false, false, true, true,
        true, true, false, true, true, false, true, true,
        false, false, true, true, false, false, true, true,
        true, false, false, true, false, false, false, false,
        true, false, true, true, true, true, false, true,
        false, true, true, false, false, true, true, true,
        false, false, false, true, true, false, false, false,
        true, true, false, false, true, true, true, false,
        true, true, false, false },
      { false, true, true, true, false, true, false, true,
        false, false, true, true, true, false, false, false,
        false, true, false, false, false, true, false, false,
        false, true, true, false, false, true, true, true,
        false, false, true, true, true, false, true, false,
        false, false, true, false, false, true, true, true,
        false, true, true, true, true, false, false, false,
        false, false, true, false, true, true, false, false,
        true, true, false, false, false, true, false, false,
        false, false, true, false, false, false, false, false,
        false, false, false, true, false, false, true, false,
        true, true, true, false },
      { false, false, false, false, false, true, true, false,
        true, true, true, true, true, true, true, true,
        true, false, false, false, false, false, true, true,
        true, false, true, false, false, false, false, true,
        false, true, false, false, false, true, false, true,
        true, true, false, false, false, false, true, true,
        false, true, true, true, false, false, false, false,
        false, false, true, true, false, true, false, true,
        true, false, true, false, false, true, false, true,
        true, true, false, false, false, false, false, true,
        false, false, true, false, false, true, true, false,
        true, false, false, false },
      { false, false, true, true, true, false, true, true,
        false, false, true, true, false, true, true, true,
        false, true, false, false, false, false, false, true,
        false, true, true, true, true, false, false, false,
        false, true, false, true, true, false, false, false,
        true, true, false, false, true, true, false, false,
        false, false, true, false, true, true, false, true,
        true, true, false, true, false, false, true, true,
        false, false, true, true, true, true, true, false,
        true, true, false, false, false, false, true, true,
        true, true, true, true, false, true, true, false,
        false, false, true, false },
      { true, false, false, true, true, false, true, false,
        false, true, false, false, true, false, true, false,
        false, true, false, true, true, false, true, false,
        false, false, true, false, true, false, false, false,
        true, true, true, false, true, true, true, false,
        false, false, false, true, false, true, true, true,
        true, true, false, false, true, false, true, false,
        true, false, false, true, true, true, false, false,
        false, false, true, true, false, false, true, false,
        false, true, false, false, true, false, false, false,
        false, true, false, false, false, false, true, false,
        true, true, false, false },
      { true, false, true, true, true, true, false, false,
        false, false, true, false, true, false, false, true,
        true, true, true, true, false, true, false, false,
        false, true, true, false, false, true, false, true,
        false, false, true, true, false, false, false, false,
        true, false, false, true, true, true, false, false,
        true, false, false, true, false, true, true, true,
        false, true, true, true, true, true, true, false,
        true, false, false, false, true, false, false, true,
        false, true, true, false, false, false, false, true,
        false, false, false, false, true, false, true, false,
        false, true, false, false },
      { false, false, true, false, false, true, true, false,
        false, true, true, false, false, false, true, true,
        true, false, true, false, true, true, true, false,
        false, true, true, false, true, true, false, true,
        true, true, false, true, true, true, true, true,
        true, false, false, false, true, false, true, true,
        false, true, false, true, true, true, false, false,
        true, true, true, false, false, false, true, false,
        true, false, true, true, true, false, true, true,
        false, false, true, false, true, false, false, true,
        false, true, false, false, true, false, false, false,
        true, false, false, false },
      { false, true, false, false, false, true, true, false,
        true, true, true, true, false, false, true, false,
        false, false, true, true, false, false, false, true,
        true, true, true, false, true, true, true, true,
        true, true, true, false, false, true, false, false,
        false, true, false, true, false, true, true, true,
        false, false, false, false, false, false, true, true,
        false, true, false, false, true, true, false, false,
        false, false, false, true, true, false, false, false,
        false, false, false, true, false, true, false, false,
        false, true, false, false, false, false, false, true,
        true, false, false, false },
      { false, false, true, true, true, true, true, true,
        true, false, true, true, false, false, true, false,
        true, true, false, false, true, true, true, false,
        true, false, false, false, false, true, false, true,
        true, false, true, false, true, false, true, true,
        true, true, true, false, true, false, false, true,
        true, false, true, true, false, false, false, false,
        true, true, false, false, false, true, true, true,
        false, false, true, false, true, true, true, false,
        false, false, false, false, false, true, true, false,
        true, true, true, true, true, false, true, true,
        true, true, true, false },
      { true, true, false, true, true, true, true, false,
        true, false, false, false, false, true, true, true,
        false, true, false, false, true, false, false, false,
        false, false, false, true, true, true, true, true,
        false, false, true, false, true, false, false, false,
        false, false, true, false, true, true, false, false,
        false, false, false, true, false, true, false, true,
        false, false, true, true, true, false, false, true,
        false, true, true, true, false, false, false, true,
        true, false, true, false, false, false, false, false,
        true, false, true, false, false, false, true, false,
        true, true, true, false },
      { true, true, true, true, true, true, false, false,
        true, true, false, true, false, true, true, true,
        true, true, false, false, true, true, false, false,
        true, true, true, true, false, false, true, false,
        false, false, true, true, true, true, false, false,
        false, true, true, false, true, false, false, true,
        true, true, true, true, true, false, true, false,
        true, false, false, true, true, false, false, true,
        true, false, true, true, true, false, true, true,
        true, false, true, false, false, false, false, true,
        false, true, false, false, false, false, false, true,
        false, false, true, false },
      { true, true, true, true, false, false, false, false,
        false, false, true, false, false, true, true, false,
        false, false, false, true, false, true, false, false,
        false, true, false, false, false, true, true, true,
        true, true, true, false, true, false, false, true,
        false, true, false, false, true, false, false, true,
        false, false, false, false, true, true, false, false,
        true, false, true, false, true, false, false, false,
        true, true, true, false, false, true, false, false,
        false, true, true, true, false, true, false, false,
        true, true, false, false, true, true, true, false,
        true, true, false, false },
      { false, true, false, false, false, true, false, false,
        false, false, false, true, false, false, false, false,
        false, false, false, true, false, false, false, true,
        false, true, false, true, true, false, false, false,
        false, false, false, true, true, false, false, false,
        false, false, false, true, true, false, false, true,
        false, true, true, false, true, true, true, true,
        true, false, false, true, false, true, false, true,
        true, true, false, false, true, true, false, true,
        true, true, false, true, false, true, true, true,
        false, false, false, false, false, false, false, true,
        false, false, true, false },
      { false, false, false, false, true, false, false, false,
        true, false, false, false, true, true, true, true,
        true, true, false, false, false, false, true, true,
        false, false, false, true, true, true, false, true,
        true, true, true, true, false, true, false, false,
        true, false, true, true, true, true, true, true,
        true, false, true, true, true, true, false, true,
        true, true, true, false, false, false, true, false,
        true, false, true, false, false, true, false, false,
        true, true, true, false, true, false, true, false,
        true, true, true, true, true, false, true, true,
        false, true, false, false },
      { true, false, true, true, true, false, false, false,
        true, true, true, true, true, true, true, false,
        true, true, true, true, false, false, false, true,
        true, false, true, true, false, true, true, false,
        false, false, true, true, false, false, false, false,
        false, true, true, true, false, true, true, true,
        false, false, true, false, true, false, false, true,
        true, true, true, true, true, false, true, true,
        false, false, false, false, true, false, true, false,
        false, false, false, false, false, true, true, true,
        true, false, false, false, true, true, false, false,
        false, false, false, false },
      { false, true, false, true, true, false, true, false,
        true, true, true, true, true, true, true, false,
        true, false, true, false, false, true, true, true,
        true, false, true, false, true, true, false, false,
        true, true, false, false, true, true, false, false,
        true, false, true, true, false, true, true, true,
        false, true, true, true, true, false, true, true,
        true, false, true, true, true, true, false, false,
        true, false, false, true, true, true, false, true,
        true, false, false, true, true, false, false, true,
        true, false, true, false, true, false, false, true,
        false, false, false, false },
      { false, true, false, false, true, false, false, true,
        true, false, true, false, false, true, true, true,
        false, false, false, false, false, false, false, true,
        false, true, true, false, true, false, true, false,
        true, true, false, false, false, true, true, false,
        false, true, false, true, false, false, true, true,
        true, true, true, true, false, true, true, false,
        false, true, false, true, true, true, true, false,
        true, true, false, false, true, true, false, true,
        true, true, false, false, true, false, false, true,
        false, false, false, false, false, true, true, true,
        false, true, true, false },
      { false, false, false, true, true, false, false, true,
        false, true, false, false, false, true, false, false,
        true, true, false, true, false, false, false, false,
        true, false, false, false, false, true, false, true,
        true, false, true, true, true, true, true, false,
        false, true, false, false, true, true, true, false,
        false, true, true, true, true, true, false, true,
        true, false, true, false, true, false, false, false,
        true, true, false, true, false, true, true, false,
        true, true, false, false, true, true, false, false,
        false, true, true, true, true, true, false, true,
        false, false, false, false },
      { false, false, true, false, false, true, false, true,
        false, false, false, true, true, true, true, true,
        false, true, true, false, false, false, true, false,
        true, false, true, false, true, true, false, true,
        true, true, false, false, false, true, false, false,
        false, false, false, false, false, false, true, true,
        false, false, true, false, true, true, true, true,
        false, false, false, false, true, true, true, false,
        true, true, true, false, false, true, true, true,
        false, false, false, true, false, true, false, false,
        false, false, false, false, false, false, false, false,
        false, false, true, false },
      { false, true, false, true, false, true, true, false,
        false, true, false, false, false, true, true, true,
        false, false, false, true, true, true, true, true,
        true, false, false, false, false, true, true, true,
        false, false, false, false, false, false, true, false,
        true, false, true, false, false, false, false, false,
        false, true, true, true, false, false, true, false,
        false, false, false, true, true, true, true, false,
        false, false, false, false, false, false, false, false,
        true, false, true, true, false, false, false, true,
        false, false, true, false, true, false, true, true,
        true, false, false, false },
      { false, false, true, false, true, false, true, true,
        true, false, false, false, true, true, true, false,
        false, true, false, false, true, false, false, true,
        false, false, true, false, false, false, true, true,
        true, true, true, true, false, false, true, false,
        true, true, false, true, true, true, false, true,
        false, true, false, true, false, false, false, true,
        true, true, true, false, false, false, true, false,
        true, true, false, true, false, true, false, true,
        false, false, true, true, false, true, true, true,
        true, true, true, true, true, false, true, false,
        false, false, false, false },
      { false, true, true, false, true, false, true, true,
        false, true, false, true, false, true, false, true,
        false, false, false, false, true, false, true, false,
        false, true, false, false, false, false, false, false,
        true, false, true, false, false, true, true, false,
        false, true, true, false, true, true, true, true,
        false, true, false, false, false, true, true, true,
        false, true, false, true, false, true, false, true,
        true, true, false, true, true, true, true, false,
        true, false, false, true, false, true, false, true,
        true, true, false, false, false, false, true, false,
        false, true, true, false },
      { true, false, true, false, false, false, false, true,
        true, false, false, false, true, false, true, false,
        true, true, false, true, false, false, true, false,
        true, false, false, false, true, true, false, true,
        false, true, false, false, true, true, true, false,
        false, false, true, false, false, true, true, true,
        true, true, true, true, true, true, true, false,
        true, false, false, true, false, false, true, false,
        true, false, true, false, false, true, false, false,
        true, true, true, true, false, true, true, false,
        true, true, false, false, true, false, false, false,
        false, true, false, false },
      { false, false, false, true, false, false, false, false,
        true, true, false, false, false, false, true, false,
        true, true, true, false, false, true, false, true,
        true, false, false, false, false, true, true, false,
        false, false, true, true, true, false, false, false,
        true, false, false, false, true, true, false, false,
        true, false, true, true, true, false, false, false,
        false, false, true, false, true, false, true, false,
        false, false, true, true, true, true, false, true,
        true, false, false, false, false, false, false, false,
        false, true, true, true, false, true, false, true,
        true, false, false, false },
      { true, true, true, false, true, true, true, true,
        false, false, true, true, false, true, false, false,
        true, false, true, false, false, true, false, false,
        false, false, false, true, true, false, false, false,
        false, false, false, true, false, true, true, true,
        true, true, true, false, true, true, true, false,
        false, false, false, false, false, false, true, false,
        false, false, false, true, false, false, true, true,
        false, false, true, true, true, true, false, true,
        true, false, true, true, false, false, true, false,
        true, true, true, false, true, false, true, true,
        false, false, false, false },
      { false, true, true, true, true, true, true, false,
        true, false, false, true, true, true, false, false,
        false, false, false, false, true, true, false, false,
        false, true, false, true, false, true, false, false,
        false, false, true, true, false, false, true, false,
        false, true, false, true, true, false, true, false,
        true, false, false, true, true, true, false, false,
        false, false, false, true, false, true, false, true,
        true, false, false, false, false, false, true, true,
        false, true, true, false, true, true, true, false,
        false, false, false, false, false, false, false, false,
        false, false, false, false },
      { false, false, true, true, false, true, true, false,
        true, false, false, true, false, false, true, true,
        true, true, true, false, false, true, false, true,
        false, true, true, true, false, false, true, false,
        true, true, false, true, false, false, false, true,
        true, true, true, true, true, true, false, true,
        true, true, true, false, false, true, false, false,
        true, true, false, false, true, true, false, true,
        true, true, true, true, false, false, false, false,
        false, true, true, true, true, false, false, true,
        true, true, true, false, true, false, false, false,
        false, true, true, false },
      { true, false, true, true, true, true, true, true,
        true, false, true, true, false, false, true, false,
        true, true, false, false, true, true, true, false,
        true, true, false, false, false, true, false, true,
        true, false, true, false, true, false, true, true,
        true, true, true, false, false, false, false, true,
        true, false, true, true, false, false, false, false,
        true, true, false, false, false, true, true, true,
        false, false, true, false, true, true, true, false,
        false, false, false, false, false, true, true, true,
        true, true, true, true, true, false, true, true,
        true, true, true, false },
      { false, true, true, true, true, true, true, false,
        true, true, true, false, false, false, false, true,
        true, false, false, false, false, false, true, false,
        false, false, true, true, false, false, false, false,
        true, true, false, false, false, true, false, true,
        true, false, false, false, false, false, true, true,
        true, true, false, false, true, true, false, false,
        true, true, false, false, true, true, false, false,
        false, true, false, true, false, true, true, true,
        true, true, false, true, false, true, false, false,
        true, false, true, true, false, false, false, false,
        true, false, false, false },
      { true, false, true, false, false, false, false, false,
        false, true, true, false, false, true, true, false,
        true, true, false, false, true, false, true, true,
        false, false, true, false, true, true, true, true,
        true, true, true, false, true, true, false, true,
        true, false, true, false, true, true, true, true,
        true, true, false, false, true, false, false, true,
        true, true, true, true, false, true, false, true,
        false, false, true, false, false, true, true, false,
        false, true, true, false, false, true, false, false,
        false, false, false, true, false, false, true, false,
        false, true, true, false },
      { true, false, true, true, true, false, true, true,
        false, false, true, false, false, false, true, true,
        false, true, true, true, false, false, true, false,
        false, true, false, true, true, false, true, false,
        true, false, true, true, true, true, false, false,
        false, true, false, false, false, true, true, true,
        true, true, false, false, true, true, false, false,
        false, true, false, true, true, true, true, true,
        false, true, false, false, true, true, false, false,
        true, true, false, false, false, true, false, false,
        true, true, false, false, true, true, false, true,
        false, false, true, false },
      { true, true, false, true, true, true, true, false,
        true, true, false, true, true, false, false, true,
        true, true, false, true, true, false, true, true,
        true, false, true, false, false, false, true, true,
        true, false, true, true, true, true, true, false,
        true, true, true, false, false, true, false, false,
        false, false, false, false, true, true, false, false,
        false, true, false, true, true, false, false, true,
        true, false, true, true, false, true, false, true,
        false, true, true, false, false, false, false, false,
        true, false, false, true, true, false, true, true,
        false, true, false, false },
      { true, true, false, true, true, false, false, true,
        true, false, true, false, false, true, true, true,
        false, false, false, false, false, false, false, true,
        false, true, true, false, true, false, true, false,
        true, true, false, false, false, true, true, false,
        false, true, false, true, false, false, true, true,
        true, true, true, false, false, true, true, false,
        true, true, false, true, true, true, true, false,
        true, true, false, false, true, true, false, true,
        true, true, false, false, true, false, false, true,
        false, false, false, false, false, false, true, true,
        false, true, true, false },
      { true, false, false, true, true, false, true, false,
        true, true, false, true, false, true, false, false,
        false, true, true, false, true, false, true, false,
        true, true, true, false, true, true, false, true,
        false, true, false, true, true, true, true, true,
        false, true, true, true, false, false, false, false,
        false, true, true, true, true, true, true, true,
        false, false, true, false, true, false, false, false,
        false, false, false, false, true, false, true, false,
        true, false, true, true, false, true, false, true,
        true, true, true, true, true, true, false, false,
        false, true, false, false },
      { true, true, true, false, false, true, false, true,
        true, false, false, true, false, false, true, false,
        false, false, false, true, true, true, false, false,
        false, true, true, true, false, true, true, true,
        true, false, false, false, false, false, true, false,
        false, false, true, false, false, true, false, true,
        true, false, false, false, false, true, true, true,
        false, false, true, true, false, false, false, true,
        false, true, true, false, true, true, false, true,
        false, true, true, true, true, true, false, true,
        false, false, true, true, true, true, false, false,
        false, false, true, false },
      { false, true, false, false, true, true, true, true,
        false, false, false, true, false, true, false, false,
        true, true, false, true, true, false, true, false,
        true, false, false, false, false, false, true, false,
        false, true, false, false, false, false, true, false,
        true, false, true, false, true, false, false, false,
        true, false, true, true, true, false, false, false,
        false, true, true, false, true, true, false, true,
        true, true, false, false, true, false, true, false,
        false, true, true, true, false, false, true, true,
        false, false, true, true, false, true, false, true,
        false, false, true, false },
      { true, false, false, false, true, false, true, true,
        true, false, false, false, true, false, true, true,
        false, true, false, true, false, false, false, false,
        false, true, true, true, true, false, true, false,
        true, true, false, true, false, true, false, false,
        false, true, true, false, false, true, true, true,
        true, true, false, true, false, true, false, false,
        false, true, false, false, false, true, false, false,
        false, false, false, true, true, true, false, true,
        true, true, true, true, false, true, true, true,
        false, true, true, true, false, false, false, false,
        true, true, true, false },
      { false, false, true, false, false, false, true, false,
        true, false, false, false, false, false, true, true,
        false, false, false, true, true, true, false, false,
        true, false, false, true, true, true, false, false,
        true, true, true, true, false, false, false, true,
        false, false, false, true, false, true, true, false,
        true, false, false, true, false, true, false, false,
        false, true, true, false, false, true, true, true,
        true, false, true, false, true, true, false, true,
        false, false, false, false, false, true, false, false,
        true, false, true, true, false, true, true, false,
        true, false, false, false },
      { false, false, true, false, false, false, false, true,
        false, false, true, true, true, false, true, true,
        true, false, false, false, false, false, true, true,
        true, false, false, false, true, true, true, true,
        true, true, true, false, false, false, true, false,
        true, false, true, false, true, true, true, false,
        false, true, false, true, false, true, false, false,
        true, true, false, false, false, false, true, true,
        true, false, false, false, true, true, true, false,
        true, true, true, false, false, true, true, true,
        false, false, false, true, true, false, false, false,
        false, false, false, false },
      { false, true, false, true, true, true, false, true,
        true, false, false, true, false, false, true, false,
        false, true, true, false, true, false, true, true,
        false, true, true, false, true, true, false, true,
        true, true, false, true, false, true, true, true,
        false, false, false, true, true, true, true, true,
        false, false, false, false, true, false, false, false,
        false, true, false, true, false, false, false, true,
        true, false, false, false, false, false, false, true,
        true, false, true, false, false, true, false, false,
        true, true, true, false, false, false, false, true,
        false, false, true, false },
      { false, true, true, false, false, true, true, false,
        true, false, true, false, true, false, true, true,
        false, true, true, true, true, false, false, true,
        true, true, false, true, false, true, false, false,
        true, false, true, true, false, false, true, false,
        true, false, false, true, true, true, true, false,
        true, true, true, false, false, true, true, false,
        true, true, true, false, false, true, true, false,
        true, false, false, true, false, true, false, true,
        false, false, false, false, true, false, false, true,
        true, true, true, false, false, true, false, true,
        false, true, true, false },
      { true, false, false, true, false, true, false, true,
        true, false, false, false, false, false, false, true,
        false, true, false, false, true, false, false, false,
        false, true, true, false, true, false, false, false,
        false, false, true, false, true, true, false, true,
        false, true, true, true, false, true, false, false,
        true, false, false, false, true, false, true, false,
        false, false, true, true, true, false, false, false,
        true, true, false, true, true, true, false, true,
        false, true, true, false, true, false, false, false,
        true, false, true, true, true, false, true, false,
        true, false, true, false },
      { true, false, true, true, true, false, false, false,
        true, true, false, false, true, true, true, false,
        false, false, false, false, false, false, true, false,
        false, false, false, false, true, true, false, false,
        true, true, true, true, false, false, false, false,
        false, true, true, false, true, false, false, true,
        true, true, false, false, false, false, true, true,
        false, false, true, false, true, false, true, false,
        false, true, true, true, false, false, true, false,
        false, false, true, true, true, false, true, false,
        true, false, true, true, false, false, false, true,
        false, true, false, false },
      { true, true, true, true, false, true, false, false,
        false, false, true, true, false, false, true, true,
        false, false, false, true, true, true, false, true,
        false, true, true, false, true, true, false, true,
        false, true, false, false, false, true, true, false,
        false, false, false, true, false, true, true, false,
        false, false, false, false, false, true, true, true,
        true, true, true, false, true, false, false, true,
        false, true, false, true, false, true, true, true,
        false, true, false, true, false, false, true, false,
        false, true, true, true, false, true, false, false,
        false, true, true, false },
      { false, true, true, false, true, true, false, true,
        true, false, true, false, false, false, true, false,
        false, false, true, true, true, false, true, true,
        true, false, true, false, false, true, false, false,
        false, false, true, false, false, true, false, false,
        true, false, true, true, true, false, false, true,
        false, true, false, true, true, false, false, true,
        false, true, true, false, false, false, false, true,
        false, false, true, true, false, false, true, true,
        true, true, false, false, true, true, true, true,
        true, false, false, true, true, true, false, false,
        true, false, false, false },
      { true, false, true, false, false, true, true, false,
        false, false, true, true, false, true, true, false,
        true, false, true, true, true, true, false, false,
        true, false, true, true, true, true, false, false,
        false, true, true, true, true, false, true, true,
        false, false, true, true, false, false, false, false,
        true, true, false, false, false, true, false, true,
        true, true, true, true, true, false, true, true,
        true, true, true, false, true, false, true, false,
        true, true, true, false, false, true, true, false,
        false, true, true, true, true, true, true, true,
        true, true, true, false },
      { false, true, false, true, true, true, false, false,
        true, false, true, true, false, false, false, false,
        true, true, false, true, true, false, false, false,
        false, true, true, false, true, false, true, false,
        false, false, false, false, false, true, true, true,
        true, true, false, true, true, true, true, true,
        false, true, true, false, false, true, false, true,
        false, true, false, false, true, false, true, false,
        true, false, false, true, false, false, false, false,
        true, false, false, false, true, false, false, true,
        true, false, true, false, false, false, true, false,
        false, false, false, false },
      { true, true, true, true, false, false, false, true,
        false, false, false, true, true, true, true, true,
        false, false, false, true, false, false, false, false,
        false, true, true, false, true, false, false, false,
        false, true, false, false, true, false, false, false,
        false, true, true, true, true, false, false, false,
        false, false, false, false, true, true, true, true,
        true, true, false, false, true, false, false, true,
        true, true, true, false, true, true, false, false,
        true, true, false, true, true, true, false, true,
        true, false, false, false, false, false, false, false,
        true, false, true, false },
      { false, false, false, true, true, true, true, true,
        true, false, true, true, true, false, true, true,
        false, true, false, true, false, false, true, true,
        false, true, true, false, false, true, false, false,
        true, true, true, true, true, false, true, true,
        true, false, false, false, true, true, false, true,
        false, false, true, false, true, true, false, false,
        true, false, false, true, true, true, false, true,
        false, true, true, true, false, false, true, true,
        false, false, false, false, true, true, false, true,
        false, true, false, true, true, false, true, true,
        true, false, true, false },
      { true, true, true, true, true, true, false, false,
        true, false, true, true, true, false, false, false,
        false, true, true, false, true, false, true, true,
        true, true, false, false, false, true, true, true,
        false, false, false, false, true, false, true, false,
        false, true, false, true, false, false, false, false,
        true, true, false, false, true, false, false, true,
        true, true, false, true, false, false, false, false,
        false, false, true, false, true, false, true, false,
        false, true, false, true, true, true, false, true,
        false, false, false, false, false, false, true, true,
        false, true, false, false },
      { true, false, true, false, false, true, false, true,
        false, false, true, true, false, true, false, false,
        false, true, false, false, false, false, true, true,
        false, false, true, true, false, false, false, false,
        false, false, true, false, true, false, false, true,
        true, true, true, false, true, false, true, false,
        true, true, false, false, false, false, false, true,
        false, true, false, true, true, true, true, true,
        false, false, true, true, false, false, true, false,
        false, false, true, false, true, true, true, false,
        false, false, true, true, false, true, false, false,
        true, true, false, false },
      { true, true, false, false, true, false, false, true,
        true, false, false, false, true, false, false, true,
        true, true, false, true, true, false, false, true,
        true, true, false, false, false, true, true, true,
        true, true, false, false, false, false, true, true,
        true, true, false, true, false, false, true, true,
        true, false, true, true, true, false, false, false,
        true, true, false, false, false, true, false, true,
        false, true, false, true, true, true, false, true,
        false, true, true, true, false, true, false, true,
        false, false, false, true, false, false, true, true,
        false, false, false, false },
      { false, true, true, true, true, false, true, true,
        true, false, true, true, false, false, true, true,
        true, false, false, false, true, false, true, true,
        false, false, true, false, true, true, true, true,
        false, false, false, false, false, false, false, true,
        true, false, false, false, false, true, true, false,
        true, true, false, true, false, true, false, false,
        false, true, true, false, false, true, true, false,
        false, true, false, false, false, false, true, true,
        true, false, true, false, true, true, true, false,
        true, false, false, true, false, true, true, false,
        false, false, true, false },
      { false, false, true, false, false, true, true, false,
        false, true, false, false, false, true, false, false,
        true, true, true, false, true, false, true, true,
        true, false, true, false, true, true, false, true,
        true, true, true, false, true, false, true, true,
        false, true, false, false, false, true, false, false,
        true, false, true, true, true, false, false, true,
        false, true, false, false, false, true, true, false,
        false, true, true, true, true, true, false, true,
        false, false, false, true, true, true, true, true,
        false, true, false, false, false, false, true, false,
        true, true, false, false },
      { false, true, true, false, false, false, false, false,
        true, false, false, false, true, true, false, false,
        true, true, false, false, true, false, false, false,
        false, true, false, true, false, true, true, true,
        false, true, false, true, true, false, false, true,
        false, true, false, false, true, false, true, true,
        true, true, true, true, true, false, true, true,
        true, false, true, true, false, true, false, true,
        false, true, false, true, true, true, false, true,
        false, true, true, false, true, false, false, true,
        false, true, true, false, false, false, false, false,
        false, false, false, false }
    };
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
  fprintf(stderr, "FT4FT8Fields assignment %p\n", this);
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
        fprintf(stderr, "Type found, we can return a vector, index = %d\n", vectorIndex);
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
    fprintf(stderr, "i=3 is encoded to %d\n", binary);
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

int main() {
  fprintf(stderr, "----------------------------1-----------------------------\n");
  t71 at71;
  at71.print();
  fprintf(stderr, "----------------------------2-----------------------------\n");
  t71 bt71(0x010203);
  bt71.print();
  fprintf(stderr, "----------------------------3-----------------------------\n");
  t71 ct71(0xaaaaaaaaaaaaaaaa);
  ct71.print();
  fprintf(stderr, "----------------------------4-----------------------------\n");
  t71 dt71(ct71.getField());
  dt71.print();
  fprintf(stderr, "----------------------------5-----------------------------\n");
  t71 et71(bt71.getField());
  et71.print();
  fprintf(stderr, "----------------------------6-----------------------------\n");
  FT4FT8Fields cat = bt71 + bt71;
  cat.print();
  fprintf(stderr, "----------------------------7-----------------------------\n");
  char cs[] = "KG5YJE";
  fprintf(stderr, "in main creating a c28 callSign object with nothing in it\n");
  c28 callSign;
  callSign.print();
  fprintf(stderr, "----------------------------8-----------------------------\n");
  fprintf(stderr, "in main, about to call encode and will assign the result to callsign\n");
  c28 x = c28::encode(cs);
  fprintf(stderr, "back in main after encode\n");
  x.print();
  fprintf(stderr, "----------------------------9-----------------------------\n");
  fprintf(stderr, "in main about to assign callSign to copy\n");
  c28 copy = x;
  fprintf(stderr, "in main about to print copy\n");
  copy.print();
  fprintf(stderr, "----------------------------10-----------------------------\n");
  FT4FT8Fields a = copy + copy;
  a.print();
  fprintf(stderr, "----------------------------11-----------------------------\n");
  fprintf(stderr, "Decoded callSign: %s\n", x.decode());
  char g[] = "EM13";
  g15 grid = g15::encode(g);
  fprintf(stderr, "Decoded grid: %s\n", grid.decode());
  FT4FT8Fields b = copy + grid;
  b.print();
  fprintf(stderr, "----------------------------12-----------------------------\n");
  char r[] = "/R";
  r1 r1stuff = r1::encode(r);
  fprintf(stderr, "Decoded r: %s\n", r1stuff.decode());
  FT4FT8Fields c = b + r1stuff;
  c.print();
  fprintf(stderr, "----------------------------13-----------------------------\n");
  // lets make a full type 1 message of c28 r1 c28 r1 R1 g15
  char cq[] = "CQ";
  char empty[] = "";
  FT4FT8Fields f1 = c28::encode(cq);
  FT4FT8Fields f2 = r1::encode(empty);
  FT4FT8Fields f3 = c28::encode(cs);
  FT4FT8Fields f4 = r1::encode(empty);
  FT4FT8Fields f5 = r1::encode(empty);
  FT4FT8Fields f6 = g15::encode(g);
  char one[] = "1";
  FT4FT8Fields f7 = i3::encode(one);
  FT4FT8Fields type1(1);
  type1 = f1 + f2 + f3 + f4 + f5 + f6 + f7;
  type1.print();
  type1.toOctal();
  fprintf(stderr, "----------------------------14-----------------------------\n");
  std::vector<bool> cks = FT4FT8Fields::crc(type1.getField());
  FT4FT8Fields f8 = cs14(cks);
  f8.print();
  FT4FT8Fields type2 = type1 + f8;
  type2.print();
  type2.toOctal();
  fprintf(stderr, "----------------------------15-----------------------------\n");
  FT4FT8Fields fakeF8 = cs14(0x0004);
  FT4FT8Fields type3 = type1 + fakeF8;
  type3.print();
  type3.toOctal();
  fprintf(stderr, "----------------------------16-----------------------------\n");
  type3 = type1 + fakeF8;
  type3.print();
  fprintf(stderr, "----------------------------17-----------------------------\n");
  FT4FT8Fields type4 = type3("c28", 0);
  type4.print();
  fprintf(stderr, "----------------------------18-----------------------------\n");
  type4 = type3("c28", 1);
  type4.print();
  c28 type5 = c28::convertToC28(type4);
  type5.print();
  fprintf(stderr, "decoded c28: %s\n", type5.decode());
  fprintf(stderr, "----------------------------19-----------------------------\n");
  type4 = type3("c28", 2);
  type4.print();
  fprintf(stderr, "----------------------------20-----------------------------\n");
  FT4FT8Fields ldpcPart = ldpc83(FT4FT8Fields::ldpc(type2.getField()));
  FT4FT8Fields fullMessage = type2 + ldpcPart;
  fullMessage.print();
  fullMessage.toOctal();
  return 0;
}
