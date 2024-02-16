#ifndef FT4FT8FIELDS_H_
#define FT4FT8FIELDS_H_
/*
 *      FT4FT8Fields.h - FT4 FT8 bit fields
 *
 *      Copyright (C) 2024
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */
#include <cstdio>
#include <cstdlib>
#include <stdint.h>
#include <vector>
/* ---------------------------------------------------------------------- */
class FT4FT8Fields {
 protected:
  uint32_t bits;
  uint32_t bytes;
  uint8_t * fieldBytes;
  std::vector<bool> fieldBits;
  std::vector<const char *> fieldTypes;
  void setType(const char * t) { fieldTypes.push_back(t); };
  FT4FT8Fields(void){ bits = 0; bytes = 0; fieldBytes = 0; };
  FT4FT8Fields(uint32_t bits, uint64_t data, const char * fieldType);
  FT4FT8Fields(uint32_t bits, std::vector<bool> data, std::vector<const char *> fields);
 public:
  FT4FT8Fields(uint32_t bits);
  FT4FT8Fields(uint32_t bits, uint64_t data);
  FT4FT8Fields(uint32_t bits, std::vector<bool> data);
  std::vector<bool> getField(void) { return fieldBits; };
  void print(void) const;
  void toOctal(void) const;
  uint32_t static toGray(uint32_t v) { const uint32_t map[] = {0, 1, 3, 2, 5, 6, 4, 7 }; return map[v]; };
  uint32_t static isIn(char b, const char * a);
  std::vector<bool> static crc(std::vector<bool> message);
  FT4FT8Fields operator+(FT4FT8Fields & rhs);
  FT4FT8Fields& operator=(FT4FT8Fields & rhs);

  FT4FT8Fields(FT4FT8Fields& orig);
  FT4FT8Fields(const FT4FT8Fields& orig);
  FT4FT8Fields(volatile FT4FT8Fields& orig);
  FT4FT8Fields(volatile const FT4FT8Fields& orig);
  ~FT4FT8Fields(void);
};
/* ---------------------------------------------------------------------- */
class i3 : public FT4FT8Fields {
 private:
  char i3Char[2];
 public:
  i3(void): FT4FT8Fields(3){ setType("i3"); };
  i3(uint64_t data): FT4FT8Fields(3, data){ setType("i3"); };
  i3(std::vector<bool> data): FT4FT8Fields(3, data){ setType("i3"); };
  static i3 encode(char * displayFormat);
  char * decode();

  ~i3(void){};
};
/* ---------------------------------------------------------------------- */
class n3 : public FT4FT8Fields {
 private:
  char n3Char[2];
 public:
  n3(void): FT4FT8Fields(3){ setType("n3"); };
  n3(uint64_t data): FT4FT8Fields(3, data){ setType("n3"); };
  n3(std::vector<bool> data): FT4FT8Fields(3, data){ setType("n3"); };
  static n3 encode(char * displayFormat);
  char * decode();

  ~n3(void){};
};
/* ---------------------------------------------------------------------- */
class c1 : public FT4FT8Fields {
 private:
 public:
  c1(void): FT4FT8Fields(1){ setType("c1"); };
  c1(uint64_t data): FT4FT8Fields(1, data){ setType("c1"); };
  c1(std::vector<bool> data): FT4FT8Fields(1, data){ setType("c1"); };

  ~c1(void){};
};
/* ---------------------------------------------------------------------- */
class c28 : public FT4FT8Fields {
 private:
  char callSign[10];
 public:
  c28(void): FT4FT8Fields(28){ setType("c28"); };
  c28(uint64_t data): FT4FT8Fields(28, data){ setType("c28"); fprintf(stderr, "c28 constructor with data %llu %p\n",
                                                                      data, this);};
  c28(std::vector<bool> data): FT4FT8Fields(28, data){ setType("c28"); };
  static c28 encode(char * displayFormat);
  char * decode();
  c28(c28& orig);
  c28(const c28& orig);
  c28(volatile c28& orig);
  c28(volatile const c28& orig);
  ~c28(void){fprintf(stderr, "in c28 destructor: %p\n", this);};
};
/* ---------------------------------------------------------------------- */
class c58 : public FT4FT8Fields {
 private:
 public:
  c58(void): FT4FT8Fields(58){ setType("c58"); };
  c58(uint64_t data): FT4FT8Fields(58, data){ setType("c58"); };
  c58(std::vector<bool> data): FT4FT8Fields(58, data){ setType("c58"); };

  ~c58(void){};
};
/* ---------------------------------------------------------------------- */
class f71 : public FT4FT8Fields {
 private:
 public:
  f71(void): FT4FT8Fields(71){ setType("f71"); };
  f71(uint64_t data): FT4FT8Fields(71, data){ setType("f71"); };
  f71(std::vector<bool> data): FT4FT8Fields(71, data){ setType("f71"); };

  ~f71(void){};
};
/* ---------------------------------------------------------------------- */
class g15 : public FT4FT8Fields {
 private:
  char grid[5];
 public:
  g15(void): FT4FT8Fields(15){ setType("g15"); };
  g15(uint64_t data): FT4FT8Fields(15, data){ setType("g15"); };
  g15(std::vector<bool> data): FT4FT8Fields(15, data){ setType("g15"); };
  static g15 encode(char * displayFormat);
  char * decode();

  ~g15(void){};
};
/* ---------------------------------------------------------------------- */
class g25 : public FT4FT8Fields {
 private:
 public:
  g25(void): FT4FT8Fields(25){ setType("g25"); };
  g25(uint64_t data): FT4FT8Fields(25, data){ setType("g25"); };
  g25(std::vector<bool> data): FT4FT8Fields(25, data){ setType("g25"); };

  ~g25(void){};
};
/* ---------------------------------------------------------------------- */
class h1 : public FT4FT8Fields {
 private:
 public:
  h1(void): FT4FT8Fields(1){ setType("h1"); };
  h1(uint64_t data): FT4FT8Fields(1, data){ setType("h1"); };
  h1(std::vector<bool> data): FT4FT8Fields(1, data){ setType("h1"); };

  ~h1(void){};
};
/* ---------------------------------------------------------------------- */
class h10 : public FT4FT8Fields {
 private:
 public:
  h10(void): FT4FT8Fields(10){ setType("h10"); };
  h10(uint64_t data): FT4FT8Fields(10, data){ setType("h10"); };
  h10(std::vector<bool> data): FT4FT8Fields(10, data){ setType("h10"); };

  ~h10(void){};
};
/* ---------------------------------------------------------------------- */
class h12 : public FT4FT8Fields {
 private:
 public:
  h12(void): FT4FT8Fields(12){ setType("h12"); };
  h12(uint64_t data): FT4FT8Fields(12, data){ setType("h12"); };
  h12(std::vector<bool> data): FT4FT8Fields(12, data){ setType("h12"); };

  ~h12(void){};
};
/* ---------------------------------------------------------------------- */
class h22 : public FT4FT8Fields {
 private:
 public:
  h22(void): FT4FT8Fields(22){ setType("h22"); };
  h22(uint64_t data): FT4FT8Fields(22, data){ setType("h22"); };
  h22(std::vector<bool> data): FT4FT8Fields(22, data){ setType("h22"); };

  ~h22(void){};
};
/* ---------------------------------------------------------------------- */
class k3 : public FT4FT8Fields {
 private:
 public:
  k3(void): FT4FT8Fields(3){ setType("k3"); };
  k3(uint64_t data): FT4FT8Fields(3, data){ setType("k3"); };
  k3(std::vector<bool> data): FT4FT8Fields(3, data){ setType("k3"); };

  ~k3(void){};
};
/* ---------------------------------------------------------------------- */
class n4 : public FT4FT8Fields {
 private:
 public:
  n4(void): FT4FT8Fields(4){ setType("n4"); };
  n4(uint64_t data): FT4FT8Fields(4, data){ setType("n4"); };
  n4(std::vector<bool> data): FT4FT8Fields(4, data){ setType("n4"); };

  ~n4(void){};
};
/* ---------------------------------------------------------------------- */
class p1 : public FT4FT8Fields {
 private:
 public:
  p1(void): FT4FT8Fields(1){ setType("p1"); };
  p1(uint64_t data): FT4FT8Fields(1, data){ setType("p1"); };
  p1(std::vector<bool> data): FT4FT8Fields(1, data){ setType("p1"); };

  ~p1(void){};
};
/* ---------------------------------------------------------------------- */
class r1 : public FT4FT8Fields {
 private:
  char r1char[3];
 public:
  r1(void): FT4FT8Fields(1){ setType("r1"); };
  r1(uint64_t data): FT4FT8Fields(1, data){ setType("r1"); };
  r1(std::vector<bool> data): FT4FT8Fields(1, data){ setType("r1"); };
  static r1 encode(char * displayFormat);
  char * decode();

  ~r1(void){};
};
/* ---------------------------------------------------------------------- */
class r2 : public FT4FT8Fields {
 private:
 public:
  r2(void): FT4FT8Fields(2){ setType("r2"); };
  r2(uint64_t data): FT4FT8Fields(2, data){ setType("r2"); };
  r2(std::vector<bool> data): FT4FT8Fields(2, data){ setType("r2"); };

  ~r2(void){};
};
/* ---------------------------------------------------------------------- */
class r3 : public FT4FT8Fields {
 private:
 public:
  r3(void): FT4FT8Fields(3){ setType("r3"); };
  r3(uint64_t data): FT4FT8Fields(3, data){ setType("r3"); };
  r3(std::vector<bool> data): FT4FT8Fields(3, data){ setType("r3"); };

  ~r3(void){};
};
/* ---------------------------------------------------------------------- */
class R1 : public FT4FT8Fields {
 private:
  char R1char[2];
 public:
  R1(void): FT4FT8Fields(1){ setType("R1"); };
  R1(uint64_t data): FT4FT8Fields(1, data){ setType("R1"); };
  R1(std::vector<bool> data): FT4FT8Fields(1, data){ setType("R1"); };
  static R1 encode(char * displayFormat);
  char * decode();

  ~R1(void){};
};
/* ---------------------------------------------------------------------- */
class r5 : public FT4FT8Fields {
 private:
 public:
  r5(void): FT4FT8Fields(5){ setType("r5"); };
  r5(uint64_t data): FT4FT8Fields(5, data){ setType("r5"); };
  r5(std::vector<bool> data): FT4FT8Fields(5, data){ setType("r5"); };

  ~r5(void){};
};
/* ---------------------------------------------------------------------- */
class s11 : public FT4FT8Fields {
 private:
 public:
  s11(void): FT4FT8Fields(11){ setType("s11"); };
  s11(uint64_t data): FT4FT8Fields(11, data){ setType("s11"); };
  s11(std::vector<bool> data): FT4FT8Fields(11, data){ setType("s11"); };

  ~s11(void){};
};
/* ---------------------------------------------------------------------- */
class s13 : public FT4FT8Fields {
 private:
 public:
  s13(void): FT4FT8Fields(13){ setType("s13"); };
  s13(uint64_t data): FT4FT8Fields(13, data){ setType("s13"); };
  s13(std::vector<bool> data): FT4FT8Fields(13, data){ setType("s13"); };

  ~s13(void){};
};
/* ---------------------------------------------------------------------- */
class S7 : public FT4FT8Fields {
 private:
 public:
  S7(void): FT4FT8Fields(7){ setType("S7"); };
  S7(uint64_t data): FT4FT8Fields(7, data){ setType("S7"); };
  S7(std::vector<bool> data): FT4FT8Fields(7, data){ setType("S7"); };

  ~S7(void){};
};
/* ---------------------------------------------------------------------- */
class t1 : public FT4FT8Fields {
 private:
 public:
  t1(void): FT4FT8Fields(1){ setType("t1"); };
  t1(uint64_t data): FT4FT8Fields(1, data){ setType("t1"); };
  t1(std::vector<bool> data): FT4FT8Fields(1, data){ setType("t1"); };

  ~t1(void){};
};
/* ---------------------------------------------------------------------- */
class t71 : public  FT4FT8Fields {
 private:
 public:
  t71(void): FT4FT8Fields(71){ setType("t71"); };
  t71(uint64_t data): FT4FT8Fields(71, data){ setType("t71"); };
  t71(std::vector<bool> data): FT4FT8Fields(71, data){ setType("t71"); };

  ~t71(void){};
};
/* ---------------------------------------------------------------------- */
class cs14 : public  FT4FT8Fields {
 private:
 public:
  cs14(void): FT4FT8Fields(14){ setType("cs14"); };
  cs14(uint64_t data): FT4FT8Fields(14, data){ setType("cs14"); };
  cs14(std::vector<bool> data): FT4FT8Fields(14, data){ setType("cs14"); };

  ~cs14(void){};
};
#endif  //  FT4FT8FIELDS_H_
