/*
 *      convert_aByte_f.c -- DSP Pipe - byte(signed) stream to float
 *
 *      Copyright (C) 2019 
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */
#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
/* ---------------------------------------------------------------------- */

int convert_aByte_f() {
  const int BUFFER_SIZE = 4096;
  signed char c[BUFFER_SIZE];
  float f[BUFFER_SIZE];
  int count;
  float * fptr;
  signed char * cptr;
  fcntl(STDIN_FILENO, F_SETPIPE_SZ, BUFFER_SIZE); 
  fcntl(STDOUT_FILENO, F_SETPIPE_SZ, BUFFER_SIZE); 
  for (;;) {
    count = fread(&c, sizeof(char), BUFFER_SIZE, stdin);
    if(count < BUFFER_SIZE) {
      fprintf(stderr, "Short data stream\n");
      fclose(stdout);
      return 0;
    }
    cptr = c;
    fptr = f;
    for (int i=0; i < BUFFER_SIZE; i++) {
      *fptr++ = *cptr++/128.0 ;
    }
    fwrite(&f, sizeof(float), BUFFER_SIZE, stdout);
  }

  return 0;

}

/* ---------------------------------------------------------------------- */