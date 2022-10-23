/*
 *      AGC.cc - Automatic Gain Control - develop a gain that keeps the signal
 *               at a target level.
 *
 *      Copyright (C) 2022
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "AGC.h"

/* ---------------------------------------------------------------------- */
void AGC::init(float target) {
  this->target = target;
  // Build correction table
  for (int i = 0; i < SIZE_OF_CORRECTION_TABLE; i++) {
    if (i) {
      correctionTable[i].ratio = i * 0.1;
      correctionTable[i].dBCorrection = logf(correctionTable[i].ratio) / logf(10.0) * 10.0;
    } else {
      correctionTable[i].ratio = 0.0;
      correctionTable[i].dBCorrection = -20.0;
    }
  }
  // tweak values where ratio is near 1.0
  correctionTable[8].dBCorrection = -0.45;
  correctionTable[9].dBCorrection = -0.05;
  correctionTable[10].dBCorrection = 0.0;
  correctionTable[11].dBCorrection = 0.05;
  correctionTable[12].dBCorrection = 0.35;
  // Build dB to gain table
  gainDB = GAIN_MIN_DB;
  for (int i = 0; i < SIZE_OF_GAIN_TABLE; i++) {
    gainTable[i].gainDB = gainDB;
    if (i == (SIZE_OF_GAIN_TABLE - 1)) {
      gainTable[i].baseGain = powf(10.0, gainDB/10.0);
      gainTable[i].deltaGain = 0.0;
      gainTable[i-1].deltaGain = (gainTable[i].baseGain - gainTable[i-1].baseGain)/3.0;
    } else {
      gainTable[i].baseGain = powf(10.0, gainDB/10.0);
      if (i) gainTable[i-1].deltaGain = (gainTable[i].baseGain - gainTable[i-1].baseGain)/3.0;
    }
    gainDB += GAIN_DELTA;
  }
  // Write out tables
  fprintf(stderr, "AGC Correction Table\n");
  for (int i = 0; i < SIZE_OF_CORRECTION_TABLE; i++) {
    fprintf(stderr, "index: %d, correction ratio: %f, correction in dB: %f\n", i, correctionTable[i].ratio,
            correctionTable[i].dBCorrection);
  }
  fprintf(stderr, "dB to Gain Table\n");
  for (int i = 0; i < SIZE_OF_GAIN_TABLE; i++) {
    fprintf(stderr, "index: %d, Gain in dB: %f, base gain: %f, gain delta %f\n", i, gainTable[i].gainDB,
            gainTable[i].baseGain, gainTable[i].deltaGain);
  }
  // set initial gains for main loop
  gain = 100.0;
  gainDB = 20.0;
}

AGC::AGC(void) {
  init(1.0);
}

AGC::AGC(float target) {
  init(target);
}

void AGC::doWork() {
  float signal[BUFFER_SIZE];
  float output[BUFFER_SIZE];
  float * signalPtr = 0;
  float * outputPtr = 0;
  fprintf(stderr, "AGC with target value of %f\n", target);
  bool done = false;
  size_t count = 0;
  float observedTarget = 0.0;
  while (!done) {
    // fprintf(stderr, "top of loop, sizeof correctionTable: %d, sizeof gainTable: %d,
    // sizeof signal: %d, sizeof output: %d\n", sizeof(correctionTable), sizeof(gainTable),
    // sizeof(signal), sizeof(output));
    count = fread(&signal, sizeof(float), BUFFER_SIZE, stdin);
    // fprintf(stderr, "read done\n");
    if (count < BUFFER_SIZE) {
      done = true;
      continue;
    }
    signalPtr = signal;
    outputPtr = output;
    for (size_t i = 0; i < BUFFER_SIZE; i++) {  // apply gain
      *outputPtr++ = *signalPtr++ * gain;
    }
    // fprintf(stderr, "doing write\n");
    fwrite(output, sizeof(float), count, stdout);
    // find target within input
    // fprintf(stderr, "finding target\n");
    observedTarget = findTarget(output, BUFFER_SIZE);
    // calculate new gain for next pass
    // fprintf(stderr, "calculating new gain\n");
    gain = adjustGain(observedTarget);
    fprintf(stderr, "gain: %f, gainDB: %f, target: %f\n", gain, gainDB, observedTarget);
  }
}

float AGC::findTarget(float * buffer, size_t size) {
  float absoluteSum = 0.0;
  for (size_t i = 0; i < size; i++) {
    absoluteSum += fabsf(*buffer++);
  }
  return absoluteSum * scale;
}

float AGC::adjustGain(float observedTarget) {
  float perfectCorrection = target/observedTarget;
  // fprintf(stderr, "perfectCorrection: %f, target: %f, observedTarget: %f\n",
  // perfectCorrection, target, observedTarget);
  float correctionDB = 0.0;
  float rawGain = 0.0;
  int i = 0;
  if (perfectCorrection > MAX_CORRECTION_ALLOWED) {
    perfectCorrection = MAX_CORRECTION_ALLOWED;
  }
  while (correctionTable[i].ratio < perfectCorrection && i < SIZE_OF_CORRECTION_TABLE) i++;
  correctionDB = correctionTable[i].dBCorrection;
  gainDB += correctionDB;
  // fprintf(stderr, "gainDB: %f, correctionDB: %f, observedTarget: %f\n", gainDB, correctionDB, observedTarget);
  // convert gain in dB to gain
  if (gainDB < GAIN_MIN_DB) {
    gainDB = GAIN_MIN_DB;
  } else  if (gainDB > GAIN_MAX_DB) {
    gainDB = GAIN_MAX_DB;
  }
  i = 0;
  while (gainTable[i].gainDB < gainDB && i < SIZE_OF_GAIN_TABLE) i++;
  rawGain = gainTable[i].baseGain + (gainDB - gainTable[i].gainDB)*gainTable[i].deltaGain;
  return (ALPHA * rawGain + BETA * gain);
}
AGC::~AGC(void) {
};

