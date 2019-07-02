/*
 *      FMMod.h - FM modulation class
 *
 *      Copyright (C) 2019 
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */

/* ---------------------------------------------------------------------- */
class FMMod {

  private:
  float centerFrequency;  // center frequency
  float fd;               // frequency deviation
  float sampleRate;       // samples per second
  float * inputBuffer;    // raw input
  float * outputBuffer;   // output buffer 
  float deltaT;           // time delta of each sample
  float TWO_PI;

  int readSignalPipe();
  int writeSignalPipe();
  
  public:

  FMMod(float sampleRate);

  int modulate();

  ~FMMod(void);
    
};

