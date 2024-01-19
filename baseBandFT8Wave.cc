/*
 *      base band FR8 signal - generate FT8 signal to convert to a wave file
 *
 *      Copyright (C) 2024
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */
#include <complex.h>
#include <stdio.h>
/* ---------------------------------------------------------------------- */
int main(int argc, char *argv[]) {

  // the vector below should result in a call sign of KG5YJE, a location of EM13 and message of CQ
  // ft8_gen can be used to create the tone array (testInput)
  
  int testInput[] = {3,1,4,0,6,5,2,0,0,0,0,0,0,0,0,1,1,1,2,2,7,4,1,5,3,2,0,5,0,4,7,3,3,0,0,0,
                     3,1,4,0,6,5,2,3,3,3,6,2,1,2,6,0,2,4,4,7,2,7,4,5,1,6,1,2,1,6,6,5,4,3,1,0,
                     3,1,4,0,6,5,2};
  const int IDEAL_SAMPLING_FREQUENCY = 3200;  // Hz
  const int SAMPLING_FREQUENCY = 8000;  // Hz
  double t = 0.0;  // seconds
  const double deltaT = 1.0 / SAMPLING_FREQUENCY;  // seconds
  const int DURATION = 15;
  std::complex<double> signal;
  const float DEVIATION = 6.25; // Hz
  const float RM = 1.0 / float(RAND_MAX);
  srandom(59);

  float offset = 200.0;
  float toneGain = 0.5;
  float driftRate = 0.0;  // Hz/min
  float driftDelta = 0.0;
  float drift = 0.0;
  float noiseGain = 0.5;
  float startSignalAtF = 0.5;
  if (argc == 5) {
    sscanf(argv[1], "%f", &offset);
    sscanf(argv[2], "%f", &driftRate);
    sscanf(argv[3], "%f", &toneGain);
    sscanf(argv[4], "%f", &startSignalAtF);
    driftDelta = driftRate / 60.0 / SAMPLING_FREQUENCY; // Hz/sample
  }
  int decodedSignalAtT[DURATION * SAMPLING_FREQUENCY] = {8};
  int startSignalAt = startSignalAtF * SAMPLING_FREQUENCY;
  int index = 0;
  int symLimit = 512 * SAMPLING_FREQUENCY/IDEAL_SAMPLING_FREQUENCY;
  for (auto token : testInput) {
    for (int sym = 0; sym < symLimit; sym++) {
      decodedSignalAtT[startSignalAt + index] = token;
      index++;
    }
  }
  fprintf(stderr, "last index written to was %d out of %d\n", index+startSignalAt, DURATION*SAMPLING_FREQUENCY);
  double noiseFrequency = -DEVIATION * 100.0;
  std::complex<double> noise = 2 * M_PI * noiseFrequency * I;
  std::complex<double> omega0 = 2 * M_PI * (offset + drift) * I;
  std::complex<double> omega1 = 2 * M_PI * (DEVIATION + offset + drift) * I;
  std::complex<double> omega2 = 2 * M_PI * (DEVIATION*2 + offset + drift) * I;
  std::complex<double> omega3 = 2 * M_PI * (DEVIATION*3 + offset + drift) * I;
  std::complex<double> omega4 = 2 * M_PI * (DEVIATION*4 + offset + drift) * I;
  std::complex<double> omega5 = 2 * M_PI * (DEVIATION*5 + offset + drift) * I;
  std::complex<double> omega6 = 2 * M_PI * (DEVIATION*6 + offset + drift) * I;
  std::complex<double> omega7 = 2 * M_PI * (DEVIATION*7 + offset + drift) * I;
  fprintf(stderr, "signal offset: %f, drift rate: %2.1f, tone gain %5.2f, start signal at: %f\n",
          offset, driftRate, toneGain, startSignalAtF);
  index = 0;
  float realPart;
  float imagPart;
  std::complex<double> toneGainC = std::complex<double>(toneGain + 0.0 * I);
  std::complex<double> noiseGainC = std::complex<double>(noiseGain + 0.0 * I);
  while (t < DURATION) {
    noise = 2 * M_PI * noiseFrequency * I;
    omega0 = 2 * M_PI * (offset + drift) * I;
    omega1 = 2 * M_PI * (DEVIATION + offset + drift) * I;
    omega2 = 2 * M_PI * (DEVIATION*2 + offset + drift) * I;
    omega3 = 2 * M_PI * (DEVIATION*3 + offset + drift) * I;
    omega4 = 2 * M_PI * (DEVIATION*4 + offset + drift) * I;
    omega5 = 2 * M_PI * (DEVIATION*5 + offset + drift) * I;
    omega6 = 2 * M_PI * (DEVIATION*6 + offset + drift) * I;
    omega7 = 2 * M_PI * (DEVIATION*7 + offset + drift) * I;
    
    
    signal = toneGainC * (((decodedSignalAtT[index] == 0) * 1.0) * exp(omega0 * t) +
                          ((decodedSignalAtT[index] == 1) * 1.0) * exp(omega1 * t) +
                          ((decodedSignalAtT[index] == 2) * 1.0) * exp(omega2 * t) +
                          ((decodedSignalAtT[index] == 3) * 1.0) * exp(omega3 * t) +
                          ((decodedSignalAtT[index] == 4) * 1.0) * exp(omega4 * t) +
                          ((decodedSignalAtT[index] == 5) * 1.0) * exp(omega5 * t) +
                          ((decodedSignalAtT[index] == 6) * 1.0) * exp(omega6 * t) +
                          ((decodedSignalAtT[index] == 7) * 1.0) * exp(omega7 * t)) +
      noiseGainC * exp(noise * t); 

    realPart = real(signal);
    imagPart = imag(signal);
    fwrite(&realPart, sizeof(float), 1, stdout);
    fwrite(&imagPart, sizeof(float), 1, stdout);
    t += deltaT;
    drift += driftDelta;
    noiseFrequency = (float(random()) * RM - 0.5) * 1500.0;
    index++;
  }
  return 0;

}

/* ---------------------------------------------------------------------- */
