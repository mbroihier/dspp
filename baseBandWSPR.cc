/*
 *      base band WSPR signal - generate WSPR signal to test WSPRWindow vs
 *                            rtlsdr_wsprd
 *
 *      Copyright (C) 2023
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */
#include <complex.h>
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
  const int SAMPLING_FREQUENCY = 375;  // Hz
  double t = 0.0;  // seconds
  const double deltaT = 1.0 / SAMPLING_FREQUENCY;  // seconds
  const int DURATION = 120;
  std::complex<double> signal;
  const float DEVIATION = 1.4648; // Hz

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
  int startSignalAt = startSignalAtF * SAMPLING_FREQUENCY;
  int index = 0;
  for (auto token : testInput) {
    for (int sym = 0; sym < 256; sym++) {
      decodedSignalAtT[startSignalAt + index] = token;
      index++;
    }
  }
  double noiseFrequency = -DEVIATION * 100.0;
  double noiseFrequencyDelta = DEVIATION;
  const double NOISE_FREQUENCY_LIMIT = 100.0 * DEVIATION; 
  std::complex<double> noise = 2 * M_PI * noiseFrequency * I;
  std::complex<double> omega0 = 2 * M_PI * (-DEVIATION*3/2 + offset) * I;
  std::complex<double> omega1 = 2 * M_PI * (-DEVIATION/2 + offset) * I;
  std::complex<double> omega2 = 2 * M_PI * (DEVIATION/2 + offset) * I;
  std::complex<double> omega3 = 2 * M_PI * (DEVIATION*3/2 + offset) * I;
  fprintf(stderr, "signal offset: %f, drift rate: %2.1f, tone gain %5.2f, start signal at: %f\n",
          offset, driftRate, toneGain, startSignalAtF);
  index = 0;
  float realPart;
  float imagPart;
  std::complex<double> toneGainC = std::complex<double>(toneGain + 0.0 * I);
  std::complex<double> noiseGainC = std::complex<double>(noiseGain + 0.0 * I);
  while (t < DURATION) {
    noise = 2 * M_PI * noiseFrequency * I;
    omega0 = 2 * M_PI * (-DEVIATION*3/2 + offset + drift) * I;
    omega1 = 2 * M_PI * (-DEVIATION/2 + offset + drift) * I;
    omega2 = 2 * M_PI * (DEVIATION/2 + offset + drift) * I;
    omega3 = 2 * M_PI * (DEVIATION*3/2 + offset + drift) * I;
    signal = toneGainC * (((decodedSignalAtT[index] == 0) * 1.0) * exp(omega0 * t) +
                          ((decodedSignalAtT[index] == 1) * 1.0) * exp(omega1 * t) +
                          ((decodedSignalAtT[index] == 2) * 1.0) * exp(omega2 * t) +
                          ((decodedSignalAtT[index] == 3) * 1.0) * exp(omega3 * t)) +
      noiseGainC * exp(noise * t); 

    realPart = real(signal);
    imagPart = imag(signal);
    fwrite(&realPart, sizeof(float), 1, stdout);
    fwrite(&imagPart, sizeof(float), 1, stdout);
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
