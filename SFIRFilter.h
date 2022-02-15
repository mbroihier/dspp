#ifndef SFIRFILTER_H_
#define SFIRFILTER_H_
/*
 *      SFIRFilter.h - smooth FIR filter class
 *
 *      Copyright (C) 2022
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */

/* ---------------------------------------------------------------------- */
class SFIRFilter {
 private:
  static const bool debug = false;
  bool complexFilter;      // defines if the object is a complex or real filter
  float * coefficients;
  int INPUT_BUFFER_SIZE;   // number of bytes to read from the pipe
  int SIGNAL_BUFFER_SIZE;  // total size of input buffer including delay portion
  int OUTPUT_BUFFER_SIZE;  // total size of output buffer
  float * signalBuffer;    // buffer containing I/Q values
  float * inputBuffer;     // buffer read directly from stream within signalBuffer
  float * outputBuffer;    // filtered I/Q values
  float * inputToDelay;    // input to copy to delayed area
  int M;
  int N;
  int decimation;

  int readSignalPipe();
  int writeSignalPipe();
  void doWork(float cuttoff, int decimation, bool highPass, bool complexFilter);

 public:
  explicit SFIRFilter(float cutoff);
  SFIRFilter(float cutoff, int decimation);
  SFIRFilter(float cutoff, int decimation, bool highPass);
  SFIRFilter(float cutoff, int decimation, bool highPass, bool complexFilter);

  void filterSignal();

  ~SFIRFilter(void);
};
#endif  // SFIRFILTER_H_
