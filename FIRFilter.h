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
  float cutoffFrequency; // relative to original sampling frequency
  int M;                 // number of coefficients desired - this sets the
                         // transition bandwidth and must be less than the
                         // decimation value
  int midPoint;
  int decimation;        // keep 1 out of decimation input samples

  float * oldInput;      // input from previous buffer
  int     oldestQ;       // index for oldest Q in the old input buffer
  

  public:

  enum WindowType {HAMMING, BLACKMAN};

  FIRFilter(float cutoffFrequency, int M, int decimation, WindowType windowType);

  void filterSignal(float * input, float * output, int samples);

  ~FIRFilter(void);
    
};

