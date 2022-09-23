#ifndef CFILTER_H_
#define CFILTER_H_
/*
 *      CFilter.h - Comb filter class
 *
 *      Copyright (C) 2022
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */

/* ---------------------------------------------------------------------- */
class CFilter {
 private:
  static const bool debug = false;
  int BUFFER_SIZE;   // number of bytes to read from the pipe
  signed char * inputBuffer;  // buffer read directly from stream
  float * outputBuffer;    // filtered I/Q values
  int decimation;

  int readSignalPipe();
  int writeSignalPipe();
  void doWork(int decimation);

 public:
  explicit CFilter(int decimation);

  void filterSignal();

  ~CFilter(void);
};
#endif  // CFILTER_H_
