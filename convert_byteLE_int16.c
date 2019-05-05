/*
 *      convert_byteLE_int16.c -- DSP Pipe - byte stream to short int stream
 *
 *      Copyright (C) 2019 
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */

#include <stdio.h>

/* ---------------------------------------------------------------------- */
#ifdef LE_MACHINE
int convert_byteLE_int16() {

  union encode {
    char bytes[2];
    short integer;
  } piece;

  const int byteBufferSize = sizeof(piece.bytes);
  for (;;) {
    fread(&piece.bytes, sizeof(char), byteBufferSize, stdin);
    fwrite(&piece.integer, sizeof(short), 1, stdout);
  }

  return 0;

}

/* ---------------------------------------------------------------------- */
#else

int convert_byteLE_int16() {

  union encode {
    char bytes[2];
    short integer;
  } piece;

  for (;;) {
    fread(&piece.bytes[1], sizeof(char), 1, stdin);
    fread(&piece.bytes[0], sizeof(char), 1, stdin);
    fwrite(&piece.integer, sizeof(short), 1, stdout);
  }

  return 0;

}
#endif
