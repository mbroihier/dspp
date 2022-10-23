#ifndef AGC_H_
#define AGC_H_
/*
 *      AGC.h - Automatic Gain Control - produce a gain that keeps a signal 
 *              at a target level.
 *
 *      Copyright (C) 2022 
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */
#include <math.h>
#include <sys/types.h>
/* ---------------------------------------------------------------------- */
class AGC {
 private:
  const size_t BUFFER_SIZE = 4096;
  const float scale = 1.0 / BUFFER_SIZE;
  struct CorrectionRecord { float ratio; float dBCorrection; };
  struct GainRecord { float baseGain; float deltaGain; float gainDB; };
  static const int SIZE_OF_CORRECTION_TABLE = 30;
  static const int SIZE_OF_GAIN_TABLE = 17;
  const float MAX_CORRECTION_ALLOWED = 2.9;
  // these constants are in dB
  const float GAIN_MIN_DB = -19.0;
  const float GAIN_MAX_DB = 29.0;
  const float GAIN_DELTA = 3.0;
  const float ALPHA = 0.2;
  const float BETA = 1.0 - ALPHA;
  struct CorrectionRecord correctionTable[SIZE_OF_CORRECTION_TABLE];
  struct GainRecord gainTable[SIZE_OF_GAIN_TABLE];
  float target;
  float gain;
  float gainDB;
  void init(float target);
  float findTarget(float * buffer, size_t size);
  float adjustGain(float observedTarget);

 public:
  void doWork();
  AGC(void);
  explicit AGC(float target);
  ~AGC(void);
};
#endif  // AGC_H_
