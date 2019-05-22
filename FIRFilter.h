/*
 *      fir_filter.h - FIR filter class
 *
 *      Copyright (C) 2019 
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */

/* ---------------------------------------------------------------------- */
class FIRFilter {

  private:
  float * coefficients;
  float cutoffFrequency;  // relative to original sampling frequency
  int M;                  // number of coefficients desired - this sets the
                          // transition bandwidth and must be less than the
                          // decimation value
  int midPoint;
  int decimation;         // keep 1 out of decimation input samples

  int N;                  // number of samples per read to process
  int extra;              // number of extra samples in the delay buffer
  int floatSamplesPerBufferToRead;
  int INPUT_BUFFER_SIZE;  // number of bytes to read from the pipe
  int SIGNAL_BUFFER_SIZE; // total size of input buffer including delay portion
  int OUTPUT_BUFFER_SIZE; // total size of output buffer
  float * signalBuffer;   // buffer containing I/Q values
  float * inputBuffer;    // buffer read directly from stream within signalBuffer
  float * outputBuffer;   // filtered I/Q values
  float * inputToDelay;   // input to copy to delayed area
  
  int readSignalPipe();
  int writeSignalPipe();
  public:

  enum WindowType {HAMMING, BLACKMAN};

  FIRFilter(float cutoffFrequency, int M, int decimation, int N, WindowType windowType);

  void filterSignal();

  ~FIRFilter(void);
    
};

