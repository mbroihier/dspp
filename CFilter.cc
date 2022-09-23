/*
 *      CFilter.cc - Comb filter class - derived from Hamming's book "Digital Filters" chapter 7
 *
 *      Copyright (C) 2022
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */

#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "CFilter.h"

/* ---------------------------------------------------------------------- */
CFilter::CFilter(int decimation) {
  doWork(decimation);
}
/* ---------------------------------------------------------------------- */
void CFilter::doWork(int decimation) {
  this->decimation = decimation;
  BUFFER_SIZE = decimation * 2;
  inputBuffer = reinterpret_cast<signed char *>(malloc(decimation * 2 * sizeof(signed char)));
  outputBuffer = reinterpret_cast<float *>(malloc(2 * sizeof(float)));
}

/* ---------------------------------------------------------------------- */
int CFilter::readSignalPipe() {
  int count = fread(inputBuffer, sizeof(signed char), BUFFER_SIZE, stdin);
  return count;
}

/* ---------------------------------------------------------------------- */
int CFilter::writeSignalPipe() {
  int count = fwrite(outputBuffer, sizeof(float), 2, stdout);
  return count;
}

/* ---------------------------------------------------------------------- */
//  Taken from rtlsdr_wsprd.c
void CFilter::filterSignal() {
  int IIntegrator1= 0.0;
  int QIntegrator1 = 0.0;
  int IIntegrator2= 0.0;
  int QIntegrator2 = 0.0;
  int Iy1 = 0.0;
  int It1z = 0.0;
  int It1y = 0.0;
  int Qy1 = 0.0;
  int Qt1z = 0.0;
  int Qt1y = 0.0;
  int Iy2 = 0.0;
  int It2z = 0.0;
  int It2y = 0.0;
  int Qy2 = 0.0;
  int Qt2z = 0.0;
  int Qt2y = 0.0;


  for (;;) {
    if (readSignalPipe() != BUFFER_SIZE) {
      fprintf(stderr, "Short read....\n");
      exit(-1);
    }

    signed char * sample = inputBuffer;
    for (int i = 0; i < decimation; i++) {
      IIntegrator1 += *sample++;
      QIntegrator1 += *sample++;
      IIntegrator2 += IIntegrator1;
      QIntegrator2 += QIntegrator1;
    }
    Iy1 = IIntegrator2 - It1z;
    It1z = It1y;
    It1y = IIntegrator2;
    Qy1 = QIntegrator2 - Qt1z;
    Qt1z = Qt1y;
    Qt1y = QIntegrator2;

    Iy2 = Iy1 - It2z;
    It2z = It2y;
    It2y = Iy1;
    Qy2 = Qy1 - Qt2z;
    Qt2z = Qt2y;
    Qt2y = Qy1;
    outputBuffer[0] = Iy2;
    outputBuffer[1] = Qy2;
    if (debug) {
      fprintf(stderr, "II1: %d, II2: %d, QI1: %d, QI2: %d\n", IIntegrator1, IIntegrator2, QIntegrator1, QIntegrator2);
    }
    writeSignalPipe();
  }
}

/* ---------------------------------------------------------------------- */
CFilter::~CFilter(void) {
  if (inputBuffer) free(inputBuffer);
  if (outputBuffer) free(outputBuffer);
}

