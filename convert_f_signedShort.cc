/*
 *      convert_f_signedShort.cc -- DSP Pipe - float (-1.0 to 1.0)
 *                                    to -32767 to 32767
 *
 *      Copyright (C) 2019 
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
/* ---------------------------------------------------------------------- */

int convert_f_signedShort() {
  const int BUFFER_SIZE = 4096;
  short i[BUFFER_SIZE];
  float f[BUFFER_SIZE];
  int count;
  float * fptr;
  short * iptr;
  //fcntl(STDIN_FILENO, F_SETPIPE_SZ, sizeof(f)); 
  //fcntl(STDOUT_FILENO, F_SETPIPE_SZ, sizeof(i)); 
  for (;;) {
    count = fread(&f, sizeof(float), BUFFER_SIZE, stdin);
    if(count < BUFFER_SIZE) {
      fprintf(stderr, "Short data stream\n");
      fclose(stdout);
      return 0;
    }
    iptr = i;
    fptr = f;
    for (int i=0; i < BUFFER_SIZE; i++) {
      *iptr++ = *fptr++ * 32767.0;
    }
    fwrite(&i, sizeof(short), BUFFER_SIZE, stdout);
  }

  return 0;

}

/* ---------------------------------------------------------------------- */
