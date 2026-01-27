/*
 *      base band WSPR signal - generate WSPR baseband signal via real to
 *                              quadrature
 *
 *      Copyright (C) 2026
 *          Mark Broihier
 *
 */
/* ---------------------------------------------------------------------- */
#include <math.h>
#include <stdio.h>
/* ---------------------------------------------------------------------- */
int main(int argc, char *argv[]) {

  // the vector below should result in a call sign of KG5YJE, a location of EM13 and power of 10
  int testInput[] = {3, 3, 2, 2, 2, 2, 2, 2, 3, 0, 2, 0, 3, 1, 1, 0, 0, 0, 1, 2, 2, 1, 2, 1, 1, 1, 3, 0, 0, 2,
                     2, 2, 2, 2, 1, 0, 2, 3, 0, 1, 0, 0, 0, 0, 0, 2, 1, 0, 3, 3, 2, 0, 1, 3, 2, 3, 2, 2, 2, 3,
                     3, 0, 3, 0, 0, 2, 2, 3, 1, 0, 1, 2, 3, 0, 3, 2, 3, 2, 0, 1, 0, 0, 1, 2, 1, 1, 2, 0, 0, 3,
                     3, 0, 1, 2, 1, 2, 2, 2, 1, 0, 0, 0, 0, 2, 3, 0, 0, 3, 0, 0, 1, 3, 1, 2, 3, 3, 0, 2, 1, 3,
                     0, 1, 0, 0, 2, 3, 1, 1, 2, 2, 2, 0, 0, 3, 2, 1, 0, 0, 1, 3, 2, 0, 2, 2, 0, 0, 0, 1, 1, 2,
                     3, 0, 3, 1, 2, 0, 0, 3, 3, 2, 0, 2};
  // the vector below should result in a call sign of KG5YJE, a location of EM14 and power of 10
  int testInput2[] = {3, 1, 2, 0, 2, 0, 2, 0, 3, 0, 2, 2, 3, 3, 1, 0, 0, 0, 1, 2, 2, 3, 2, 3, 1, 3, 3, 2, 0, 0,
                      2, 2, 2, 0, 1, 0, 2, 1, 0, 1, 0, 2, 0, 2, 0, 0, 1, 0, 3, 1, 2, 0, 1, 1, 2, 3, 2, 2, 2, 1,
                      3, 2, 3, 0, 0, 2, 2, 3, 1, 2, 1, 2, 3, 2, 3, 0, 3, 2, 0, 1, 0, 2, 1, 0, 1, 1, 2, 0, 0, 3,
                      3, 2, 1, 2, 1, 2, 2, 2, 1, 0, 0, 2, 0, 0, 3, 0, 0, 1, 0, 0, 1, 3, 1, 2, 3, 3, 0, 0, 1, 3,
                      0, 3, 0, 0, 2, 3, 3, 1, 2, 0, 2, 2, 0, 3, 2, 3, 0, 2, 1, 1, 2, 2, 2, 2, 0, 2, 0, 1, 1, 2,
                      3, 0, 3, 1, 2, 0, 0, 3, 3, 2, 0, 0};

  const int SAMPLING_FREQUENCY = 1500;  // Hz
  double t = 0.0;  // seconds
  const double deltaT = 1.0 / SAMPLING_FREQUENCY;  // seconds
  const int DURATION = 150;
  float signal;
  const float DEVIATION = 1.4648; // Hz
  const int SYMBOL_DURATION = int (SAMPLING_FREQUENCY / DEVIATION);
  float offset = 50.0;
  float toneGain = 0.5;
  float driftRate = 0.0;  // Hz/min
  float driftDelta = 0.0;
  float drift = 0.0;
  float noiseGain = 0.1;
  float startSignalAtF = 0.5;
  if (argc == 5) {
    sscanf(argv[1], "%f", &offset);
    sscanf(argv[2], "%f", &driftRate);
    sscanf(argv[3], "%f", &toneGain);
    sscanf(argv[4], "%f", &startSignalAtF);
    driftDelta = driftRate / 60.0 / SAMPLING_FREQUENCY; // Hz/sample
  }
  int decodedSignalAtT[DURATION * SAMPLING_FREQUENCY] = {0};
  int decodedSignalAtT2[DURATION * SAMPLING_FREQUENCY] = {0};
  int startSignalAt = startSignalAtF * SAMPLING_FREQUENCY;
  int index = 0;
  for (auto token : testInput) {
    for (int sym = 0; sym < SYMBOL_DURATION; sym++) {
      decodedSignalAtT[startSignalAt + index] = token;
      index++;
    }
  }
  index = 0;
  for (auto token : testInput2) {
    for (int sym = 0; sym < SYMBOL_DURATION; sym++) {
      decodedSignalAtT2[startSignalAt + index] = token;
      index++;
    }
  }
  double noiseFrequency = -DEVIATION * 100.0;
  double noiseFrequencyDelta = DEVIATION;
  const double NOISE_FREQUENCY_LIMIT = 100.0 * DEVIATION; 
  float noise = 2 * M_PI * noiseFrequency;
  float omega0 = 2 * M_PI * (-DEVIATION*3/2 + offset);
  float omega1 = 2 * M_PI * (-DEVIATION/2 + offset);
  float omega2 = 2 * M_PI * (DEVIATION/2 + offset);
  float omega3 = 2 * M_PI * (DEVIATION*3/2 + offset);
  float omega4 = 2 * M_PI * (-DEVIATION*3/2 + 4 * offset);
  float omega5 = 2 * M_PI * (-DEVIATION/2 + 4 * offset);
  float omega6 = 2 * M_PI * (DEVIATION/2 + 4 * offset);
  float omega7 = 2 * M_PI * (DEVIATION*3/2 + 4 * offset);
  fprintf(stderr, "signal offset: %f, drift rate: %2.1f, tone gain %5.2f, start signal at: %f\n",
          offset, driftRate, toneGain, startSignalAtF);
  index = 0;
  while (t < DURATION) {
    noise = 2 * M_PI * noiseFrequency;
    omega0 = 2 * M_PI * (-DEVIATION*3/2 + offset + drift);
    omega1 = 2 * M_PI * (-DEVIATION/2 + offset + drift);
    omega2 = 2 * M_PI * (DEVIATION/2 + offset + drift);
    omega3 = 2 * M_PI * (DEVIATION*3/2 + offset + drift);
    omega4 = 2 * M_PI * (-DEVIATION*3/2 + 4 * offset + drift);
    omega5 = 2 * M_PI * (-DEVIATION/2 + 4 * offset + drift);
    omega6 = 2 * M_PI * (DEVIATION/2 + 4 * offset + drift);
    omega7 = 2 * M_PI * (DEVIATION*3/2 + 4 * offset + drift);
    signal = toneGain * (((decodedSignalAtT[index] == 0) * 1.0) * sin(omega0 * t)*2.0 +
                         ((decodedSignalAtT[index] == 1) * 1.0) * sin(omega1 * t)*2.0 +
                         ((decodedSignalAtT[index] == 2) * 1.0) * sin(omega2 * t)*2.0 +
                         ((decodedSignalAtT[index] == 3) * 1.0) * sin(omega3 * t)*2.0 +
                         ((decodedSignalAtT2[index] == 0) * 1.0) * sin(omega4 * t)*4.0 +
                         ((decodedSignalAtT2[index] == 1) * 1.0) * sin(omega5 * t)*4.0 +
                         ((decodedSignalAtT2[index] == 2) * 1.0) * sin(omega6 * t)*4.0 +
                         ((decodedSignalAtT2[index] == 3) * 1.0) * sin(omega7 * t))*4.0 +
      noiseGain * sin(noise * t); 

    fwrite(&signal, sizeof(float), 1, stdout);
    t += deltaT;
    drift += driftDelta;
    noiseFrequency += noiseFrequencyDelta;
    if (noiseFrequency > NOISE_FREQUENCY_LIMIT) {
      noiseFrequency = -DEVIATION * 100.0;
    }
    index++;
  }
  return 0;

}

/* ---------------------------------------------------------------------- */
