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
