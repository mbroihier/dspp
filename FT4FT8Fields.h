#ifndef FT4FT8FIELDS_H_
#define FT4FT8FIELDS_H_
/*
 *      FT4FT8Fields.h - FT4 FT8 bit fields
 *
 *      Copyright (C) 2024
 *          Mark Broihier
 *
 */

/* ---------------------------------------------------------------------- */
#include <stdint.h>
#include <map>
#include <vector>
#include <cstdio>
#include <cstdlib>
static const std::vector<bool> ldpc_generator[83] =
  { { true, false, false, false, false, false, true, true,
      false, false, true, false, true, false, false, true,
      true, true, false, false, true, true, true, false,
      false, false, false, true, false, false, false, true,
      true, false, true, true, true, true, true, true,
      false, false, true, true, false, false, false, true,
      true, true, true, false, true, false, true, false,
      true, true, true, true, false, true, false, true,
      false, false, false, false, true, false, false, true,
      true, true, true, true, false, false, true, false,
      false, true, true, true, true, true, true, true,
      true, true, false, false },
    { false, true, true, true, false, true, true, false,
      false, false, false, true, true, true, false, false,
      false, false, true, false, false, true, true, false,
      false, true, false, false, true, true, true, false,
      false, false, true, false, false, true, false, true,
      true, true, false, false, false, false, true, false,
      false, true, false, true, true, false, false, true,
      false, false, true, true, false, false, true, true,
      false, true, false, true, false, true, false, false,
      true, false, false, true, false, false, true, true,
      false, false, false, true, false, false, true, true,
      false, false, true, false },
    { true, true, false, true, true, true, false, false,
      false, false, true, false, false, true, true, false,
      false, true, false, true, true, false, false, true,
      false, false, false, false, false, false, true, false,
      true, true, true, true, true, false, true, true,
      false, false, true, false, false, true, true, true,
      false, true, true, true, true, true, false, false,
      false, true, true, false, false, true, false, false,
      false, false, false, true, false, false, false, false,
      true, false, true, false, false, false, false, true,
      true, false, true, true, true, true, false, true,
      true, true, false, false },
    { false, false, false, true, true, false, true, true,
      false, false, true, true, true, true, true, true,
      false, true, false, false, false, false, false, true,
      false, true, true, true, true, false, false, false,
      false, true, false, true, true, false, false, false,
      true, true, false, false, true, true, false, true,
      false, false, true, false, true, true, false, true,
      true, true, false, true, false, false, true, true,
      false, false, true, true, true, true, true, false,
      true, true, false, false, false, true, true, true,
      true, true, true, true, false, true, true, false,
      false, false, true, false },
    { false, false, false, false, true, false, false, true,
      true, true, true, true, true, true, false, true,
      true, false, true, false, false, true, false, false,
      true, true, true, true, true, true, true, false,
      true, true, true, false, false, false, false, false,
      false, true, false, false, false, false, false, true,
      true, false, false, true, false, true, false, true,
      true, true, true, true, true, true, false, true,
      false, false, false, false, false, false, true, true,
      false, true, false, false, false, true, true, true,
      true, false, false, false, false, false, true, true,
      true, false, true, false },
    { false, false, false, false, false, true, true, true,
      false, true, true, true, true, true, false, false,
      true, true, false, false, true, true, false, false,
      true, true, false, false, false, false, false, true,
      false, false, false, true, true, false, true, true,
      true, false, false, false, true, false, false, false,
      false, true, true, true, false, false, true, true,
      true, true, true, false, true, true, false, true,
      false, true, false, true, true, true, false, false,
      false, false, true, true, true, true, false, true,
      false, true, false, false, true, false, false, false,
      true, false, true, false },
    { false, false, true, false, true, false, false, true,
      true, false, true, true, false, true, true, false,
      false, false, true, false, true, false, true, false,
      true, true, true, true, true, true, true, false,
      false, false, true, true, true, true, false, false,
      true, false, true, false, false, false, false, false,
      false, false, true, true, false, true, true, false,
      true, true, true, true, false, true, false, false,
      true, true, true, true, true, true, true, false,
      false, false, false, true, true, false, true, false,
      true, false, false, true, true, true, false, true,
      true, false, true, false },
    { false, true, true, false, false, false, false, false,
      false, true, false, true, false, true, false, false,
      true, true, true, true, true, false, true, false,
      true, true, true, true, false, true, false, true,
      true, true, true, true, false, false, true, true,
      false, true, false, true, true, true, false, true,
      true, false, false, true, false, true, true, false,
      true, true, false, true, false, false, true, true,
      true, false, true, true, false, false, false, false,
      true, true, false, false, true, false, false, false,
      true, true, false, false, false, false, true, true,
      true, true, true, false },
    { true, true, true, false, false, false, true, false,
      false, false, false, false, false, true, true, true,
      true, false, false, true, true, false, false, false,
      true, true, true, false, false, true, false, false,
      false, false, true, true, false, false, false, true,
      false, false, false, false, true, true, true, false,
      true, true, true, false, true, true, false, true,
      false, false, true, false, false, true, true, true,
      true, false, false, false, true, false, false, false,
      false, true, false, false, true, false, true, false,
      true, true, true, false, true, false, false, true,
      false, false, false, false },
    { false, true, true, true, false, true, true, true,
      false, true, false, true, true, true, false, false,
      true, false, false, true, true, true, false, false,
      false, false, false, false, true, false, false, false,
      true, true, true, false, true, false, false, false,
      false, false, false, false, true, true, true, false,
      false, false, true, false, false, true, true, false,
      true, true, false, true, true, true, false, true,
      true, false, true, false, true, true, true, false,
      false, true, false, true, false, true, true, false,
      false, false, true, true, false, false, false, true,
      true, false, false, false },
    { true, false, true, true, false, false, false, false,
      true, false, true, true, true, false, false, false,
      false, false, false, true, false, false, false, true,
      false, false, false, false, false, false, true, false,
      true, false, false, false, true, true, false, false,
      false, false, true, false, true, false, true, true,
      true, true, true, true, true, false, false, true,
      true, false, false, true, false, true, true, true,
      false, false, true, false, false, false, false, true,
      false, false, true, true, false, true, false, false,
      true, false, false, false, false, true, true, true,
      true, true, false, false },
    { false, false, false, true, true, false, false, false,
      true, false, true, false, false, false, false, false,
      true, true, false, false, true, false, false, true,
      false, false, true, false, false, false, true, true,
      false, false, false, true, true, true, true, true,
      true, true, false, false, false, true, true, false,
      false, false, false, false, true, false, true, false,
      true, true, false, true, true, true, true, true,
      false, true, false, true, true, true, false, false,
      false, true, false, true, true, true, true, false,
      true, false, true, false, false, false, true, true,
      false, false, true, false },
    { false, true, true, true, false, true, true, false,
      false, true, false, false, false, true, true, true,
      false, false, false, true, true, true, true, false,
      true, false, false, false, false, false, true, true,
      false, false, false, false, false, false, true, false,
      true, false, true, false, false, false, false, false,
      false, true, true, true, false, false, true, false,
      false, false, false, true, true, true, true, false,
      false, false, false, false, false, false, false, true,
      true, false, true, true, false, false, false, true,
      false, false, true, false, true, false, true, true,
      true, false, false, false },
    { true, true, true, true, true, true, true, true,
      true, false, true, true, true, true, false, false,
      true, true, false, false, true, false, true, true,
      true, false, false, false, false, false, false, false,
      true, true, false, false, true, false, true, false,
      true, false, false, false, false, false, true, true,
      false, true, false, false, false, false, false, true,
      true, true, true, true, true, false, true, false,
      true, true, true, true, true, false, true, true,
      false, true, false, false, false, true, true, true,
      true, false, true, true, false, false, true, false,
      true, true, true, false },
    { false, true, true, false, false, true, true, false,
      true, false, true, false, false, true, true, true,
      false, false, true, false, true, false, true, false,
      false, false, false, true, false, true, false, true,
      true, false, false, false, true, true, true, true,
      true, false, false, true, false, false, true, true,
      false, false, true, false, false, true, false, true,
      true, false, true, false, false, false, true, false,
      true, false, true, true, true, true, true, true,
      false, true, true, false, false, true, true, true,
      false, false, false, true, false, true, true, true,
      false, false, false, false },
    { true, true, false, false, false, true, false, false,
      false, false, true, false, false, true, false, false,
      false, false, true, true, false, true, true, false,
      true, false, false, false, true, false, false, true,
      true, true, true, true, true, true, true, false,
      true, false, false, false, false, true, false, true,
      true, false, true, true, false, false, false, true,
      true, true, false, false, false, true, false, true,
      false, false, false, true, false, false, true, true,
      false, true, true, false, false, false, true, true,
      true, false, true, false, false, false, false, true,
      true, false, false, false },
    { false, false, false, false, true, true, false, true,
      true, true, true, true, true, true, true, true,
      false, true, true, true, false, false, true, true,
      true, false, false, true, false, true, false, false,
      false, false, false, true, false, true, false, false,
      true, true, false, true, false, false, false, true,
      true, false, true, false, false, false, false, true,
      true, false, true, true, false, false, true, true,
      false, true, false, false, true, false, true, true,
      false, false, false, true, true, true, false, false,
      false, false, true, false, false, true, true, true,
      false, false, false, false },
    { false, false, false, true, false, true, false, true,
      true, false, true, true, false, true, false, false,
      true, false, false, false, true, false, false, false,
      false, false, true, true, false, false, false, false,
      false, true, true, false, false, false, true, true,
      false, true, true, false, true, true, false, false,
      true, false, false, false, true, false, true, true,
      true, false, false, true, true, false, false, true,
      true, false, false, false, true, false, false, true,
      false, true, false, false, true, false, false, true,
      false, true, true, true, false, false, true, false,
      true, true, true, false },
    { false, false, true, false, true, false, false, true,
      true, false, true, false, true, false, false, false,
      true, false, false, true, true, true, false, false,
      false, false, false, false, true, true, false, true,
      false, false, true, true, true, true, false, true,
      true, true, true, false, true, false, false, false,
      false, false, false, true, true, true, false, true,
      false, true, true, false, false, true, true, false,
      false, true, false, true, false, true, false, false,
      true, false, false, false, true, false, false, true,
      true, false, true, true, false, false, false, false,
      true, true, true, false },
    { false, true, false, false, true, true, true, true,
      false, false, false, true, false, false, true, false,
      false, true, true, false, true, true, true, true,
      false, false, true, true, false, true, true, true,
      true, true, true, true, true, false, true, false,
      false, true, false, true, false, false, false, true,
      true, true, false, false, true, false, true, true,
      true, true, true, false, false, true, true, false,
      false, false, false, true, true, false, true, true,
      true, true, false, true, false, true, true, false,
      true, false, true, true, true, false, false, true,
      false, true, false, false },
    { true, false, false, true, true, false, false, true,
      true, true, false, false, false, true, false, false,
      false, true, true, true, false, false, true, false,
      false, false, true, true, true, false, false, true,
      true, true, false, true, false, false, false, false,
      true, true, false, true, true, false, false, true,
      false, true, true, true, true, true, false, true,
      false, false, true, true, true, true, false, false,
      true, false, false, false, false, true, false, false,
      true, true, true, false, false, false, false, false,
      true, false, false, true, false, true, false, false,
      false, false, false, false },
    { false, false, false, true, true, false, false, true,
      false, false, false, true, true, false, false, true,
      true, false, true, true, false, true, true, true,
      false, true, false, true, false, false, false, true,
      false, false, false, true, true, false, false, true,
      false, true, true, true, false, true, true, false,
      false, true, false, true, false, true, true, false,
      false, false, true, false, false, false, false, true,
      true, false, true, true, true, false, true, true,
      false, true, false, false, true, true, true, true,
      false, false, false, true, true, true, true, false,
      true, false, false, false },
    { false, false, false, false, true, false, false, true,
      true, true, false, true, true, false, true, true,
      false, false, false, true, false, false, true, false,
      true, true, false, true, false, true, true, true,
      false, false, true, true, false, false, false, true,
      true, true, true, true, true, false, true, false,
      true, true, true, false, true, true, true, false,
      false, false, false, false, true, false, true, true,
      true, false, false, false, false, true, true, false,
      true, true, false, true, true, true, true, true,
      false, true, true, false, true, false, true, true,
      true, false, false, false },
    { false, true, false, false, true, false, false, false,
      true, false, false, false, true, true, true, true,
      true, true, false, false, false, false, true, true,
      false, false, true, true, true, true, false, true,
      true, true, true, true, false, true, false, false,
      false, false, true, true, true, true, true, true,
      true, false, true, true, true, true, false, true,
      true, true, true, false, true, true, true, false,
      true, false, true, false, false, true, false, false,
      true, true, true, false, true, false, true, false,
      true, true, true, true, true, false, true, true,
      false, true, false, false },
    { true, false, false, false, false, false, true, false,
      false, true, true, true, false, true, false, false,
      false, false, true, false, false, false, true, true,
      true, true, true, false, true, true, true, false,
      false, true, false, false, false, false, false, false,
      true, false, true, true, false, true, true, false,
      false, true, true, true, false, true, false, true,
      true, true, true, true, false, true, true, true,
      false, true, false, true, false, true, true, false,
      true, true, true, false, true, false, true, true,
      false, true, false, true, true, true, true, true,
      true, true, true, false },
    { true, false, true, false, true, false, true, true,
      true, true, true, false, false, false, false, true,
      true, false, false, true, false, true, true, true,
      true, true, false, false, false, true, false, false,
      true, false, false, false, false, true, false, false,
      true, true, false, false, true, false, true, true,
      false, true, true, true, false, true, false, false,
      false, true, true, true, false, true, false, true,
      false, true, true, true, false, false, false, true,
      false, true, false, false, false, true, false, false,
      true, false, true, false, true, false, false, true,
      true, false, true, false },
    { false, false, true, false, true, false, true, true,
      false, true, false, true, false, false, false, false,
      false, false, false, false, true, true, true, false,
      false, true, false, false, true, false, true, true,
      true, true, false, false, false, false, false, false,
      true, true, true, false, true, true, false, false,
      false, true, false, true, true, false, true, false,
      false, true, true, false, true, true, false, true,
      false, false, true, false, true, false, true, true,
      true, true, false, true, true, false, true, true,
      true, true, false, true, true, true, false, true,
      false, false, false, false },
    { true, true, false, false, false, true, false, false,
      false, true, true, true, false, true, false, false,
      true, false, true, false, true, false, true, false,
      false, true, false, true, false, false, true, true,
      true, true, false, true, false, true, true, true,
      false, false, false, false, false, false, true, false,
      false, false, false, true, true, false, false, false,
      false, true, true, true, false, true, true, false,
      false, false, false, true, false, true, true, false,
      false, true, true, false, true, false, false, true,
      false, false, true, true, false, true, true, false,
      false, false, false, false },
    { true, false, false, false, true, true, true, false,
      true, false, true, true, true, false, true, false,
      false, false, false, true, true, false, true, false,
      false, false, false, true, false, false, true, true,
      true, true, false, true, true, false, true, true,
      false, false, true, true, false, false, true, true,
      true, false, false, true, false, false, false, false,
      true, false, true, true, true, true, false, true,
      false, true, true, false, false, true, true, true,
      false, false, false, true, true, false, false, false,
      true, true, false, false, true, true, true, false,
      true, true, false, false },
    { false, true, true, true, false, true, false, true,
      false, false, true, true, true, false, false, false,
      false, true, false, false, false, true, false, false,
      false, true, true, false, false, true, true, true,
      false, false, true, true, true, false, true, false,
      false, false, true, false, false, true, true, true,
      false, true, true, true, true, false, false, false,
      false, false, true, false, true, true, false, false,
      true, true, false, false, false, true, false, false,
      false, false, true, false, false, false, false, false,
      false, false, false, true, false, false, true, false,
      true, true, true, false },
    { false, false, false, false, false, true, true, false,
      true, true, true, true, true, true, true, true,
      true, false, false, false, false, false, true, true,
      true, false, true, false, false, false, false, true,
      false, true, false, false, false, true, false, true,
      true, true, false, false, false, false, true, true,
      false, true, true, true, false, false, false, false,
      false, false, true, true, false, true, false, true,
      true, false, true, false, false, true, false, true,
      true, true, false, false, false, false, false, true,
      false, false, true, false, false, true, true, false,
      true, false, false, false },
    { false, false, true, true, true, false, true, true,
      false, false, true, true, false, true, true, true,
      false, true, false, false, false, false, false, true,
      false, true, true, true, true, false, false, false,
      false, true, false, true, true, false, false, false,
      true, true, false, false, true, true, false, false,
      false, false, true, false, true, true, false, true,
      true, true, false, true, false, false, true, true,
      false, false, true, true, true, true, true, false,
      true, true, false, false, false, false, true, true,
      true, true, true, true, false, true, true, false,
      false, false, true, false },
    { true, false, false, true, true, false, true, false,
      false, true, false, false, true, false, true, false,
      false, true, false, true, true, false, true, false,
      false, false, true, false, true, false, false, false,
      true, true, true, false, true, true, true, false,
      false, false, false, true, false, true, true, true,
      true, true, false, false, true, false, true, false,
      true, false, false, true, true, true, false, false,
      false, false, true, true, false, false, true, false,
      false, true, false, false, true, false, false, false,
      false, true, false, false, false, false, true, false,
      true, true, false, false },
    { true, false, true, true, true, true, false, false,
      false, false, true, false, true, false, false, true,
      true, true, true, true, false, true, false, false,
      false, true, true, false, false, true, false, true,
      false, false, true, true, false, false, false, false,
      true, false, false, true, true, true, false, false,
      true, false, false, true, false, true, true, true,
      false, true, true, true, true, true, true, false,
      true, false, false, false, true, false, false, true,
      false, true, true, false, false, false, false, true,
      false, false, false, false, true, false, true, false,
      false, true, false, false },
    { false, false, true, false, false, true, true, false,
      false, true, true, false, false, false, true, true,
      true, false, true, false, true, true, true, false,
      false, true, true, false, true, true, false, true,
      true, true, false, true, true, true, true, true,
      true, false, false, false, true, false, true, true,
      false, true, false, true, true, true, false, false,
      true, true, true, false, false, false, true, false,
      true, false, true, true, true, false, true, true,
      false, false, true, false, true, false, false, true,
      false, true, false, false, true, false, false, false,
      true, false, false, false },
    { false, true, false, false, false, true, true, false,
      true, true, true, true, false, false, true, false,
      false, false, true, true, false, false, false, true,
      true, true, true, false, true, true, true, true,
      true, true, true, false, false, true, false, false,
      false, true, false, true, false, true, true, true,
      false, false, false, false, false, false, true, true,
      false, true, false, false, true, true, false, false,
      false, false, false, true, true, false, false, false,
      false, false, false, true, false, true, false, false,
      false, true, false, false, false, false, false, true,
      true, false, false, false },
    { false, false, true, true, true, true, true, true,
      true, false, true, true, false, false, true, false,
      true, true, false, false, true, true, true, false,
      true, false, false, false, false, true, false, true,
      true, false, true, false, true, false, true, true,
      true, true, true, false, true, false, false, true,
      true, false, true, true, false, false, false, false,
      true, true, false, false, false, true, true, true,
      false, false, true, false, true, true, true, false,
      false, false, false, false, false, true, true, false,
      true, true, true, true, true, false, true, true,
      true, true, true, false },
    { true, true, false, true, true, true, true, false,
      true, false, false, false, false, true, true, true,
      false, true, false, false, true, false, false, false,
      false, false, false, true, true, true, true, true,
      false, false, true, false, true, false, false, false,
      false, false, true, false, true, true, false, false,
      false, false, false, true, false, true, false, true,
      false, false, true, true, true, false, false, true,
      false, true, true, true, false, false, false, true,
      true, false, true, false, false, false, false, false,
      true, false, true, false, false, false, true, false,
      true, true, true, false },
    { true, true, true, true, true, true, false, false,
      true, true, false, true, false, true, true, true,
      true, true, false, false, true, true, false, false,
      true, true, true, true, false, false, true, false,
      false, false, true, true, true, true, false, false,
      false, true, true, false, true, false, false, true,
      true, true, true, true, true, false, true, false,
      true, false, false, true, true, false, false, true,
      true, false, true, true, true, false, true, true,
      true, false, true, false, false, false, false, true,
      false, true, false, false, false, false, false, true,
      false, false, true, false },
    { true, true, true, true, false, false, false, false,
      false, false, true, false, false, true, true, false,
      false, false, false, true, false, true, false, false,
      false, true, false, false, false, true, true, true,
      true, true, true, false, true, false, false, true,
      false, true, false, false, true, false, false, true,
      false, false, false, false, true, true, false, false,
      true, false, true, false, true, false, false, false,
      true, true, true, false, false, true, false, false,
      false, true, true, true, false, true, false, false,
      true, true, false, false, true, true, true, false,
      true, true, false, false },
    { false, true, false, false, false, true, false, false,
      false, false, false, true, false, false, false, false,
      false, false, false, true, false, false, false, true,
      false, true, false, true, true, false, false, false,
      false, false, false, true, true, false, false, false,
      false, false, false, true, true, false, false, true,
      false, true, true, false, true, true, true, true,
      true, false, false, true, false, true, false, true,
      true, true, false, false, true, true, false, true,
      true, true, false, true, false, true, true, true,
      false, false, false, false, false, false, false, true,
      false, false, true, false },
    { false, false, false, false, true, false, false, false,
      true, false, false, false, true, true, true, true,
      true, true, false, false, false, false, true, true,
      false, false, false, true, true, true, false, true,
      true, true, true, true, false, true, false, false,
      true, false, true, true, true, true, true, true,
      true, false, true, true, true, true, false, true,
      true, true, true, false, false, false, true, false,
      true, false, true, false, false, true, false, false,
      true, true, true, false, true, false, true, false,
      true, true, true, true, true, false, true, true,
      false, true, false, false },
    { true, false, true, true, true, false, false, false,
      true, true, true, true, true, true, true, false,
      true, true, true, true, false, false, false, true,
      true, false, true, true, false, true, true, false,
      false, false, true, true, false, false, false, false,
      false, true, true, true, false, true, true, true,
      false, false, true, false, true, false, false, true,
      true, true, true, true, true, false, true, true,
      false, false, false, false, true, false, true, false,
      false, false, false, false, false, true, true, true,
      true, false, false, false, true, true, false, false,
      false, false, false, false },
    { false, true, false, true, true, false, true, false,
      true, true, true, true, true, true, true, false,
      true, false, true, false, false, true, true, true,
      true, false, true, false, true, true, false, false,
      true, true, false, false, true, true, false, false,
      true, false, true, true, false, true, true, true,
      false, true, true, true, true, false, true, true,
      true, false, true, true, true, true, false, false,
      true, false, false, true, true, true, false, true,
      true, false, false, true, true, false, false, true,
      true, false, true, false, true, false, false, true,
      false, false, false, false },
    { false, true, false, false, true, false, false, true,
      true, false, true, false, false, true, true, true,
      false, false, false, false, false, false, false, true,
      false, true, true, false, true, false, true, false,
      true, true, false, false, false, true, true, false,
      false, true, false, true, false, false, true, true,
      true, true, true, true, false, true, true, false,
      false, true, false, true, true, true, true, false,
      true, true, false, false, true, true, false, true,
      true, true, false, false, true, false, false, true,
      false, false, false, false, false, true, true, true,
      false, true, true, false },
    { false, false, false, true, true, false, false, true,
      false, true, false, false, false, true, false, false,
      true, true, false, true, false, false, false, false,
      true, false, false, false, false, true, false, true,
      true, false, true, true, true, true, true, false,
      false, true, false, false, true, true, true, false,
      false, true, true, true, true, true, false, true,
      true, false, true, false, true, false, false, false,
      true, true, false, true, false, true, true, false,
      true, true, false, false, true, true, false, false,
      false, true, true, true, true, true, false, true,
      false, false, false, false },
    { false, false, true, false, false, true, false, true,
      false, false, false, true, true, true, true, true,
      false, true, true, false, false, false, true, false,
      true, false, true, false, true, true, false, true,
      true, true, false, false, false, true, false, false,
      false, false, false, false, false, false, true, true,
      false, false, true, false, true, true, true, true,
      false, false, false, false, true, true, true, false,
      true, true, true, false, false, true, true, true,
      false, false, false, true, false, true, false, false,
      false, false, false, false, false, false, false, false,
      false, false, true, false },
    { false, true, false, true, false, true, true, false,
      false, true, false, false, false, true, true, true,
      false, false, false, true, true, true, true, true,
      true, false, false, false, false, true, true, true,
      false, false, false, false, false, false, true, false,
      true, false, true, false, false, false, false, false,
      false, true, true, true, false, false, true, false,
      false, false, false, true, true, true, true, false,
      false, false, false, false, false, false, false, false,
      true, false, true, true, false, false, false, true,
      false, false, true, false, true, false, true, true,
      true, false, false, false },
    { false, false, true, false, true, false, true, true,
      true, false, false, false, true, true, true, false,
      false, true, false, false, true, false, false, true,
      false, false, true, false, false, false, true, true,
      true, true, true, true, false, false, true, false,
      true, true, false, true, true, true, false, true,
      false, true, false, true, false, false, false, true,
      true, true, true, false, false, false, true, false,
      true, true, false, true, false, true, false, true,
      false, false, true, true, false, true, true, true,
      true, true, true, true, true, false, true, false,
      false, false, false, false },
    { false, true, true, false, true, false, true, true,
      false, true, false, true, false, true, false, true,
      false, false, false, false, true, false, true, false,
      false, true, false, false, false, false, false, false,
      true, false, true, false, false, true, true, false,
      false, true, true, false, true, true, true, true,
      false, true, false, false, false, true, true, true,
      false, true, false, true, false, true, false, true,
      true, true, false, true, true, true, true, false,
      true, false, false, true, false, true, false, true,
      true, true, false, false, false, false, true, false,
      false, true, true, false },
    { true, false, true, false, false, false, false, true,
      true, false, false, false, true, false, true, false,
      true, true, false, true, false, false, true, false,
      true, false, false, false, true, true, false, true,
      false, true, false, false, true, true, true, false,
      false, false, true, false, false, true, true, true,
      true, true, true, true, true, true, true, false,
      true, false, false, true, false, false, true, false,
      true, false, true, false, false, true, false, false,
      true, true, true, true, false, true, true, false,
      true, true, false, false, true, false, false, false,
      false, true, false, false },
    { false, false, false, true, false, false, false, false,
      true, true, false, false, false, false, true, false,
      true, true, true, false, false, true, false, true,
      true, false, false, false, false, true, true, false,
      false, false, true, true, true, false, false, false,
      true, false, false, false, true, true, false, false,
      true, false, true, true, true, false, false, false,
      false, false, true, false, true, false, true, false,
      false, false, true, true, true, true, false, true,
      true, false, false, false, false, false, false, false,
      false, true, true, true, false, true, false, true,
      true, false, false, false },
    { true, true, true, false, true, true, true, true,
      false, false, true, true, false, true, false, false,
      true, false, true, false, false, true, false, false,
      false, false, false, true, true, false, false, false,
      false, false, false, true, false, true, true, true,
      true, true, true, false, true, true, true, false,
      false, false, false, false, false, false, true, false,
      false, false, false, true, false, false, true, true,
      false, false, true, true, true, true, false, true,
      true, false, true, true, false, false, true, false,
      true, true, true, false, true, false, true, true,
      false, false, false, false },
    { false, true, true, true, true, true, true, false,
      true, false, false, true, true, true, false, false,
      false, false, false, false, true, true, false, false,
      false, true, false, true, false, true, false, false,
      false, false, true, true, false, false, true, false,
      false, true, false, true, true, false, true, false,
      true, false, false, true, true, true, false, false,
      false, false, false, true, false, true, false, true,
      true, false, false, false, false, false, true, true,
      false, true, true, false, true, true, true, false,
      false, false, false, false, false, false, false, false,
      false, false, false, false },
    { false, false, true, true, false, true, true, false,
      true, false, false, true, false, false, true, true,
      true, true, true, false, false, true, false, true,
      false, true, true, true, false, false, true, false,
      true, true, false, true, false, false, false, true,
      true, true, true, true, true, true, false, true,
      true, true, true, false, false, true, false, false,
      true, true, false, false, true, true, false, true,
      true, true, true, true, false, false, false, false,
      false, true, true, true, true, false, false, true,
      true, true, true, false, true, false, false, false,
      false, true, true, false },
    { true, false, true, true, true, true, true, true,
      true, false, true, true, false, false, true, false,
      true, true, false, false, true, true, true, false,
      true, true, false, false, false, true, false, true,
      true, false, true, false, true, false, true, true,
      true, true, true, false, false, false, false, true,
      true, false, true, true, false, false, false, false,
      true, true, false, false, false, true, true, true,
      false, false, true, false, true, true, true, false,
      false, false, false, false, false, true, true, true,
      true, true, true, true, true, false, true, true,
      true, true, true, false },
    { false, true, true, true, true, true, true, false,
      true, true, true, false, false, false, false, true,
      true, false, false, false, false, false, true, false,
      false, false, true, true, false, false, false, false,
      true, true, false, false, false, true, false, true,
      true, false, false, false, false, false, true, true,
      true, true, false, false, true, true, false, false,
      true, true, false, false, true, true, false, false,
      false, true, false, true, false, true, true, true,
      true, true, false, true, false, true, false, false,
      true, false, true, true, false, false, false, false,
      true, false, false, false },
    { true, false, true, false, false, false, false, false,
      false, true, true, false, false, true, true, false,
      true, true, false, false, true, false, true, true,
      false, false, true, false, true, true, true, true,
      true, true, true, false, true, true, false, true,
      true, false, true, false, true, true, true, true,
      true, true, false, false, true, false, false, true,
      true, true, true, true, false, true, false, true,
      false, false, true, false, false, true, true, false,
      false, true, true, false, false, true, false, false,
      false, false, false, true, false, false, true, false,
      false, true, true, false },
    { true, false, true, true, true, false, true, true,
      false, false, true, false, false, false, true, true,
      false, true, true, true, false, false, true, false,
      false, true, false, true, true, false, true, false,
      true, false, true, true, true, true, false, false,
      false, true, false, false, false, true, true, true,
      true, true, false, false, true, true, false, false,
      false, true, false, true, true, true, true, true,
      false, true, false, false, true, true, false, false,
      true, true, false, false, false, true, false, false,
      true, true, false, false, true, true, false, true,
      false, false, true, false },
    { true, true, false, true, true, true, true, false,
      true, true, false, true, true, false, false, true,
      true, true, false, true, true, false, true, true,
      true, false, true, false, false, false, true, true,
      true, false, true, true, true, true, true, false,
      true, true, true, false, false, true, false, false,
      false, false, false, false, true, true, false, false,
      false, true, false, true, true, false, false, true,
      true, false, true, true, false, true, false, true,
      false, true, true, false, false, false, false, false,
      true, false, false, true, true, false, true, true,
      false, true, false, false },
    { true, true, false, true, true, false, false, true,
      true, false, true, false, false, true, true, true,
      false, false, false, false, false, false, false, true,
      false, true, true, false, true, false, true, false,
      true, true, false, false, false, true, true, false,
      false, true, false, true, false, false, true, true,
      true, true, true, false, false, true, true, false,
      true, true, false, true, true, true, true, false,
      true, true, false, false, true, true, false, true,
      true, true, false, false, true, false, false, true,
      false, false, false, false, false, false, true, true,
      false, true, true, false },
    { true, false, false, true, true, false, true, false,
      true, true, false, true, false, true, false, false,
      false, true, true, false, true, false, true, false,
      true, true, true, false, true, true, false, true,
      false, true, false, true, true, true, true, true,
      false, true, true, true, false, false, false, false,
      false, true, true, true, true, true, true, true,
      false, false, true, false, true, false, false, false,
      false, false, false, false, true, false, true, false,
      true, false, true, true, false, true, false, true,
      true, true, true, true, true, true, false, false,
      false, true, false, false },
    { true, true, true, false, false, true, false, true,
      true, false, false, true, false, false, true, false,
      false, false, false, true, true, true, false, false,
      false, true, true, true, false, true, true, true,
      true, false, false, false, false, false, true, false,
      false, false, true, false, false, true, false, true,
      true, false, false, false, false, true, true, true,
      false, false, true, true, false, false, false, true,
      false, true, true, false, true, true, false, true,
      false, true, true, true, true, true, false, true,
      false, false, true, true, true, true, false, false,
      false, false, true, false },
    { false, true, false, false, true, true, true, true,
      false, false, false, true, false, true, false, false,
      true, true, false, true, true, false, true, false,
      true, false, false, false, false, false, true, false,
      false, true, false, false, false, false, true, false,
      true, false, true, false, true, false, false, false,
      true, false, true, true, true, false, false, false,
      false, true, true, false, true, true, false, true,
      true, true, false, false, true, false, true, false,
      false, true, true, true, false, false, true, true,
      false, false, true, true, false, true, false, true,
      false, false, true, false },
    { true, false, false, false, true, false, true, true,
      true, false, false, false, true, false, true, true,
      false, true, false, true, false, false, false, false,
      false, true, true, true, true, false, true, false,
      true, true, false, true, false, true, false, false,
      false, true, true, false, false, true, true, true,
      true, true, false, true, false, true, false, false,
      false, true, false, false, false, true, false, false,
      false, false, false, true, true, true, false, true,
      true, true, true, true, false, true, true, true,
      false, true, true, true, false, false, false, false,
      true, true, true, false },
    { false, false, true, false, false, false, true, false,
      true, false, false, false, false, false, true, true,
      false, false, false, true, true, true, false, false,
      true, false, false, true, true, true, false, false,
      true, true, true, true, false, false, false, true,
      false, false, false, true, false, true, true, false,
      true, false, false, true, false, true, false, false,
      false, true, true, false, false, true, true, true,
      true, false, true, false, true, true, false, true,
      false, false, false, false, false, true, false, false,
      true, false, true, true, false, true, true, false,
      true, false, false, false },
    { false, false, true, false, false, false, false, true,
      false, false, true, true, true, false, true, true,
      true, false, false, false, false, false, true, true,
      true, false, false, false, true, true, true, true,
      true, true, true, false, false, false, true, false,
      true, false, true, false, true, true, true, false,
      false, true, false, true, false, true, false, false,
      true, true, false, false, false, false, true, true,
      true, false, false, false, true, true, true, false,
      true, true, true, false, false, true, true, true,
      false, false, false, true, true, false, false, false,
      false, false, false, false },
    { false, true, false, true, true, true, false, true,
      true, false, false, true, false, false, true, false,
      false, true, true, false, true, false, true, true,
      false, true, true, false, true, true, false, true,
      true, true, false, true, false, true, true, true,
      false, false, false, true, true, true, true, true,
      false, false, false, false, true, false, false, false,
      false, true, false, true, false, false, false, true,
      true, false, false, false, false, false, false, true,
      true, false, true, false, false, true, false, false,
      true, true, true, false, false, false, false, true,
      false, false, true, false },
    { false, true, true, false, false, true, true, false,
      true, false, true, false, true, false, true, true,
      false, true, true, true, true, false, false, true,
      true, true, false, true, false, true, false, false,
      true, false, true, true, false, false, true, false,
      true, false, false, true, true, true, true, false,
      true, true, true, false, false, true, true, false,
      true, true, true, false, false, true, true, false,
      true, false, false, true, false, true, false, true,
      false, false, false, false, true, false, false, true,
      true, true, true, false, false, true, false, true,
      false, true, true, false },
    { true, false, false, true, false, true, false, true,
      true, false, false, false, false, false, false, true,
      false, true, false, false, true, false, false, false,
      false, true, true, false, true, false, false, false,
      false, false, true, false, true, true, false, true,
      false, true, true, true, false, true, false, false,
      true, false, false, false, true, false, true, false,
      false, false, true, true, true, false, false, false,
      true, true, false, true, true, true, false, true,
      false, true, true, false, true, false, false, false,
      true, false, true, true, true, false, true, false,
      true, false, true, false },
    { true, false, true, true, true, false, false, false,
      true, true, false, false, true, true, true, false,
      false, false, false, false, false, false, true, false,
      false, false, false, false, true, true, false, false,
      true, true, true, true, false, false, false, false,
      false, true, true, false, true, false, false, true,
      true, true, false, false, false, false, true, true,
      false, false, true, false, true, false, true, false,
      false, true, true, true, false, false, true, false,
      false, false, true, true, true, false, true, false,
      true, false, true, true, false, false, false, true,
      false, true, false, false },
    { true, true, true, true, false, true, false, false,
      false, false, true, true, false, false, true, true,
      false, false, false, true, true, true, false, true,
      false, true, true, false, true, true, false, true,
      false, true, false, false, false, true, true, false,
      false, false, false, true, false, true, true, false,
      false, false, false, false, false, true, true, true,
      true, true, true, false, true, false, false, true,
      false, true, false, true, false, true, true, true,
      false, true, false, true, false, false, true, false,
      false, true, true, true, false, true, false, false,
      false, true, true, false },
    { false, true, true, false, true, true, false, true,
      true, false, true, false, false, false, true, false,
      false, false, true, true, true, false, true, true,
      true, false, true, false, false, true, false, false,
      false, false, true, false, false, true, false, false,
      true, false, true, true, true, false, false, true,
      false, true, false, true, true, false, false, true,
      false, true, true, false, false, false, false, true,
      false, false, true, true, false, false, true, true,
      true, true, false, false, true, true, true, true,
      true, false, false, true, true, true, false, false,
      true, false, false, false },
    { true, false, true, false, false, true, true, false,
      false, false, true, true, false, true, true, false,
      true, false, true, true, true, true, false, false,
      true, false, true, true, true, true, false, false,
      false, true, true, true, true, false, true, true,
      false, false, true, true, false, false, false, false,
      true, true, false, false, false, true, false, true,
      true, true, true, true, true, false, true, true,
      true, true, true, false, true, false, true, false,
      true, true, true, false, false, true, true, false,
      false, true, true, true, true, true, true, true,
      true, true, true, false },
    { false, true, false, true, true, true, false, false,
      true, false, true, true, false, false, false, false,
      true, true, false, true, true, false, false, false,
      false, true, true, false, true, false, true, false,
      false, false, false, false, false, true, true, true,
      true, true, false, true, true, true, true, true,
      false, true, true, false, false, true, false, true,
      false, true, false, false, true, false, true, false,
      true, false, false, true, false, false, false, false,
      true, false, false, false, true, false, false, true,
      true, false, true, false, false, false, true, false,
      false, false, false, false },
    { true, true, true, true, false, false, false, true,
      false, false, false, true, true, true, true, true,
      false, false, false, true, false, false, false, false,
      false, true, true, false, true, false, false, false,
      false, true, false, false, true, false, false, false,
      false, true, true, true, true, false, false, false,
      false, false, false, false, true, true, true, true,
      true, true, false, false, true, false, false, true,
      true, true, true, false, true, true, false, false,
      true, true, false, true, true, true, false, true,
      true, false, false, false, false, false, false, false,
      true, false, true, false },
    { false, false, false, true, true, true, true, true,
      true, false, true, true, true, false, true, true,
      false, true, false, true, false, false, true, true,
      false, true, true, false, false, true, false, false,
      true, true, true, true, true, false, true, true,
      true, false, false, false, true, true, false, true,
      false, false, true, false, true, true, false, false,
      true, false, false, true, true, true, false, true,
      false, true, true, true, false, false, true, true,
      false, false, false, false, true, true, false, true,
      false, true, false, true, true, false, true, true,
      true, false, true, false },
    { true, true, true, true, true, true, false, false,
      true, false, true, true, true, false, false, false,
      false, true, true, false, true, false, true, true,
      true, true, false, false, false, true, true, true,
      false, false, false, false, true, false, true, false,
      false, true, false, true, false, false, false, false,
      true, true, false, false, true, false, false, true,
      true, true, false, true, false, false, false, false,
      false, false, true, false, true, false, true, false,
      false, true, false, true, true, true, false, true,
      false, false, false, false, false, false, true, true,
      false, true, false, false },
    { true, false, true, false, false, true, false, true,
      false, false, true, true, false, true, false, false,
      false, true, false, false, false, false, true, true,
      false, false, true, true, false, false, false, false,
      false, false, true, false, true, false, false, true,
      true, true, true, false, true, false, true, false,
      true, true, false, false, false, false, false, true,
      false, true, false, true, true, true, true, true,
      false, false, true, true, false, false, true, false,
      false, false, true, false, true, true, true, false,
      false, false, true, true, false, true, false, false,
      true, true, false, false },
    { true, true, false, false, true, false, false, true,
      true, false, false, false, true, false, false, true,
      true, true, false, true, true, false, false, true,
      true, true, false, false, false, true, true, true,
      true, true, false, false, false, false, true, true,
      true, true, false, true, false, false, true, true,
      true, false, true, true, true, false, false, false,
      true, true, false, false, false, true, false, true,
      false, true, false, true, true, true, false, true,
      false, true, true, true, false, true, false, true,
      false, false, false, true, false, false, true, true,
      false, false, false, false },
    { false, true, true, true, true, false, true, true,
      true, false, true, true, false, false, true, true,
      true, false, false, false, true, false, true, true,
      false, false, true, false, true, true, true, true,
      false, false, false, false, false, false, false, true,
      true, false, false, false, false, true, true, false,
      true, true, false, true, false, true, false, false,
      false, true, true, false, false, true, true, false,
      false, true, false, false, false, false, true, true,
      true, false, true, false, true, true, true, false,
      true, false, false, true, false, true, true, false,
      false, false, true, false },
    { false, false, true, false, false, true, true, false,
      false, true, false, false, false, true, false, false,
      true, true, true, false, true, false, true, true,
      true, false, true, false, true, true, false, true,
      true, true, true, false, true, false, true, true,
      false, true, false, false, false, true, false, false,
      true, false, true, true, true, false, false, true,
      false, true, false, false, false, true, true, false,
      false, true, true, true, true, true, false, true,
      false, false, false, true, true, true, true, true,
      false, true, false, false, false, false, true, false,
      true, true, false, false },
    { false, true, true, false, false, false, false, false,
      true, false, false, false, true, true, false, false,
      true, true, false, false, true, false, false, false,
      false, true, false, true, false, true, true, true,
      false, true, false, true, true, false, false, true,
      false, true, false, false, true, false, true, true,
      true, true, true, true, true, false, true, true,
      true, false, true, true, false, true, false, true,
      false, true, false, true, true, true, false, true,
      false, true, true, false, true, false, false, true,
      false, true, true, false, false, false, false, false,
      false, false, false, false }
  };
/* ---------------------------------------------------------------------- */
class FT4FT8Fields {
 protected:
  uint32_t bits;
  uint32_t bytes;
  uint8_t * fieldBytes;
  std::vector<bool> fieldBits;
  std::vector<const char *> fieldTypes;
  std::vector<uint32_t> fieldSizes;
  std::vector<uint32_t> fieldIndices;
  void setType(const char * t) { fieldTypes.push_back(t); }
  FT4FT8Fields(void) { bits = 0; bytes = 0; fieldBytes = 0; }
  FT4FT8Fields(uint32_t bits, uint64_t data, const char * fieldType);
  FT4FT8Fields(uint32_t bits, std::vector<bool> data, std::vector<const char *> fields,
               std::vector<uint32_t> fieldIndices, std::vector<uint32_t> fieldSizes);

 public:
  explicit FT4FT8Fields(uint32_t bits);
  FT4FT8Fields(uint32_t bits, uint64_t data);
  FT4FT8Fields(uint32_t bits, std::vector<bool> data);
  uint32_t getBits() const { return bits; }
  uint32_t getBytes() const { return bytes; }
  uint8_t * getFieldBytes() const { return fieldBytes; }
  std::vector<bool> getFieldBits() const { return fieldBits; }
  std::vector<const char *> getFieldTypes() const { return fieldTypes; }
  std::vector<uint32_t> getFieldSizes() const { return fieldSizes; }
  std::vector<uint32_t> getFieldIndices() const { return fieldIndices; }
  std::vector<bool> getField(void) const { return fieldBits; }
  void print(void) const;
  void toOctal(void) const;
  uint32_t static toGray(uint32_t v) { const uint32_t map[] = {0, 1, 3, 2, 5, 6, 4, 7 }; return map[v]; }
  uint32_t static isIn(char b, const char * a);
  std::vector<bool> static crc(std::vector<bool> message);
  std::vector<bool> static ldpc(std::vector<bool> message);
  bool static checkLdpc(std::vector<bool> payload, uint32_t index, std::map<uint32_t,
                        std::vector<uint32_t>> * possibleBits);
  bool static fastCheckLdpc(std::vector<bool> payload, uint32_t index);
  uint32_t static scoreLdpc(std::vector<bool> payload, std::map<uint32_t, std::vector<uint32_t>> * possibleBits);
  uint32_t static fastScoreLdpc(std::vector<bool> payload);
  FT4FT8Fields operator+(const FT4FT8Fields & rhs);
  FT4FT8Fields& operator=(const FT4FT8Fields & rhs);
  FT4FT8Fields operator ()(const char * index, uint32_t instance);

  FT4FT8Fields(uint32_t bits, std::vector<bool> data, std::vector<const char *> fields);
  FT4FT8Fields(const FT4FT8Fields& orig);
  ~FT4FT8Fields(void);
};
/* ---------------------------------------------------------------------- */
class i3 : public FT4FT8Fields {
 private:
  char i3Char[2];
 public:
  i3(void): FT4FT8Fields(3) { setType("i3"); }
  explicit i3(uint64_t data): FT4FT8Fields(3, data) { setType("i3"); }
  explicit i3(std::vector<bool> data): FT4FT8Fields(3, data) { setType("i3"); }
  static i3 encode(char * displayFormat);
  char * decode();

  ~i3(void) {}
};
/* ---------------------------------------------------------------------- */
class n3 : public FT4FT8Fields {
 private:
  char n3Char[2];
 public:
  n3(void): FT4FT8Fields(3) { setType("n3"); }
  explicit n3(uint64_t data): FT4FT8Fields(3, data) { setType("n3"); }
  explicit n3(std::vector<bool> data): FT4FT8Fields(3, data) { setType("n3"); }
  static n3 encode(char * displayFormat);
  char * decode();

  ~n3(void) {}
};
/* ---------------------------------------------------------------------- */
class c1 : public FT4FT8Fields {
 private:
 public:
  c1(void): FT4FT8Fields(1) { setType("c1"); }
  explicit c1(uint64_t data): FT4FT8Fields(1, data) { setType("c1"); }
  explicit c1(std::vector<bool> data): FT4FT8Fields(1, data) { setType("c1"); }

  ~c1(void) {}
};
/* ---------------------------------------------------------------------- */
class c28 : public FT4FT8Fields {
 private:
  char callSign[10];
 public:
  c28(void): FT4FT8Fields(28) { setType("c28"); }
  explicit c28(uint64_t data): FT4FT8Fields(28, data) { setType("c28");
    /* fprintf(stderr, "c28 constructor with data %llu %p\n", data, this); */ }
  explicit c28(std::vector<bool> data): FT4FT8Fields(28, data) { setType("c28"); }
  static c28 encode(char * displayFormat);
  char * decode();
  c28 static convertToC28(const FT4FT8Fields& orig);
  c28(const c28& orig);
  ~c28(void) { /* fprintf(stderr, "in c28 destructor: %p\n", this); */ }
};
/* ---------------------------------------------------------------------- */
class c58 : public FT4FT8Fields {
 private:
 public:
  c58(void): FT4FT8Fields(58) { setType("c58"); }
  explicit c58(uint64_t data): FT4FT8Fields(58, data) { setType("c58"); }
  explicit c58(std::vector<bool> data): FT4FT8Fields(58, data) { setType("c58"); }

  ~c58(void) {}
};
/* ---------------------------------------------------------------------- */
class f71 : public FT4FT8Fields {
 private:
 public:
  f71(void): FT4FT8Fields(71) { setType("f71"); }
  explicit f71(uint64_t data): FT4FT8Fields(71, data) { setType("f71"); }
  explicit f71(std::vector<bool> data): FT4FT8Fields(71, data) { setType("f71"); }

  ~f71(void){}
};
/* ---------------------------------------------------------------------- */
class g15 : public FT4FT8Fields {
 private:
  char grid[5];
 public:
  g15(void): FT4FT8Fields(15) { setType("g15"); }
  explicit g15(uint64_t data): FT4FT8Fields(15, data) { setType("g15"); }
  explicit g15(std::vector<bool> data): FT4FT8Fields(15, data) { setType("g15"); }
  static g15 encode(char * displayFormat);
  char * decode();

  ~g15(void) {}
};
/* ---------------------------------------------------------------------- */
class g25 : public FT4FT8Fields {
 private:
 public:
  g25(void): FT4FT8Fields(25) { setType("g25"); }
  explicit g25(uint64_t data): FT4FT8Fields(25, data) { setType("g25"); }
  explicit g25(std::vector<bool> data): FT4FT8Fields(25, data) { setType("g25"); }

  ~g25(void) {}
};
/* ---------------------------------------------------------------------- */
class h1 : public FT4FT8Fields {
 private:
 public:
  h1(void): FT4FT8Fields(1) { setType("h1"); }
  explicit h1(uint64_t data): FT4FT8Fields(1, data) { setType("h1"); }
  explicit h1(std::vector<bool> data): FT4FT8Fields(1, data) { setType("h1"); }

  ~h1(void) {}
};
/* ---------------------------------------------------------------------- */
class h10 : public FT4FT8Fields {
 private:
 public:
  h10(void): FT4FT8Fields(10) { setType("h10"); }
  explicit h10(uint64_t data): FT4FT8Fields(10, data) { setType("h10"); }
  explicit h10(std::vector<bool> data): FT4FT8Fields(10, data) { setType("h10"); }

  ~h10(void) {}
};
/* ---------------------------------------------------------------------- */
class h12 : public FT4FT8Fields {
 private:
 public:
  h12(void): FT4FT8Fields(12) { setType("h12"); }
  explicit h12(uint64_t data): FT4FT8Fields(12, data) { setType("h12"); }
  explicit h12(std::vector<bool> data): FT4FT8Fields(12, data) { setType("h12"); }

  ~h12(void) {}
};
/* ---------------------------------------------------------------------- */
class h22 : public FT4FT8Fields {
 private:
 public:
  h22(void): FT4FT8Fields(22) { setType("h22"); }
  explicit h22(uint64_t data): FT4FT8Fields(22, data) { setType("h22"); }
  explicit h22(std::vector<bool> data): FT4FT8Fields(22, data) { setType("h22"); }

  ~h22(void) {}
};
/* ---------------------------------------------------------------------- */
class k3 : public FT4FT8Fields {
 private:
 public:
  k3(void): FT4FT8Fields(3) { setType("k3"); }
  explicit k3(uint64_t data): FT4FT8Fields(3, data) { setType("k3"); }
  explicit k3(std::vector<bool> data): FT4FT8Fields(3, data) { setType("k3"); }

  ~k3(void) {}
};
/* ---------------------------------------------------------------------- */
class n4 : public FT4FT8Fields {
 private:
 public:
  n4(void): FT4FT8Fields(4) { setType("n4"); }
  explicit n4(uint64_t data): FT4FT8Fields(4, data) { setType("n4"); }
  explicit n4(std::vector<bool> data): FT4FT8Fields(4, data) { setType("n4"); }

  ~n4(void) {}
};
/* ---------------------------------------------------------------------- */
class p1 : public FT4FT8Fields {
 private:
 public:
  p1(void): FT4FT8Fields(1) { setType("p1"); }
  explicit p1(uint64_t data): FT4FT8Fields(1, data) { setType("p1"); }
  explicit p1(std::vector<bool> data): FT4FT8Fields(1, data) { setType("p1"); }

  ~p1(void) {}
};
/* ---------------------------------------------------------------------- */
class r1 : public FT4FT8Fields {
 private:
  char r1char[3];
 public:
  r1(void): FT4FT8Fields(1) { setType("r1"); }
  explicit r1(uint64_t data): FT4FT8Fields(1, data) { setType("r1"); }
  explicit r1(std::vector<bool> data): FT4FT8Fields(1, data) { setType("r1"); }
  static r1 encode(char * displayFormat);
  char * decode();

  ~r1(void) {}
};
/* ---------------------------------------------------------------------- */
class r2 : public FT4FT8Fields {
 private:
 public:
  r2(void): FT4FT8Fields(2) { setType("r2"); }
  explicit r2(uint64_t data): FT4FT8Fields(2, data) { setType("r2"); }
  explicit r2(std::vector<bool> data): FT4FT8Fields(2, data) { setType("r2"); }

  ~r2(void) {}
};
/* ---------------------------------------------------------------------- */
class r3 : public FT4FT8Fields {
 private:
 public:
  r3(void): FT4FT8Fields(3) { setType("r3"); }
  explicit r3(uint64_t data): FT4FT8Fields(3, data) { setType("r3"); }
  explicit r3(std::vector<bool> data): FT4FT8Fields(3, data) { setType("r3"); }

  ~r3(void) {}
};
/* ---------------------------------------------------------------------- */
class R1 : public FT4FT8Fields {
 private:
  char R1char[2];
 public:
  R1(void): FT4FT8Fields(1) { setType("R1"); }
  explicit R1(uint64_t data): FT4FT8Fields(1, data) { setType("R1"); }
  explicit R1(std::vector<bool> data): FT4FT8Fields(1, data) { setType("R1"); }
  static R1 encode(char * displayFormat);
  char * decode();

  ~R1(void) {}
};
/* ---------------------------------------------------------------------- */
class r5 : public FT4FT8Fields {
 private:
 public:
  r5(void): FT4FT8Fields(5) { setType("r5"); }
  explicit r5(uint64_t data): FT4FT8Fields(5, data) { setType("r5"); }
  explicit r5(std::vector<bool> data): FT4FT8Fields(5, data) { setType("r5"); }

  ~r5(void) {}
};
/* ---------------------------------------------------------------------- */
class s11 : public FT4FT8Fields {
 private:
 public:
  s11(void): FT4FT8Fields(11) { setType("s11"); }
  explicit s11(uint64_t data): FT4FT8Fields(11, data) { setType("s11"); }
  explicit s11(std::vector<bool> data): FT4FT8Fields(11, data) { setType("s11"); }

  ~s11(void) {}
};
/* ---------------------------------------------------------------------- */
class s13 : public FT4FT8Fields {
 private:
 public:
  s13(void): FT4FT8Fields(13) { setType("s13"); }
  explicit s13(uint64_t data): FT4FT8Fields(13, data) { setType("s13"); }
  explicit s13(std::vector<bool> data): FT4FT8Fields(13, data) { setType("s13"); }

  ~s13(void) {}
};
/* ---------------------------------------------------------------------- */
class S7 : public FT4FT8Fields {
 private:
 public:
  S7(void): FT4FT8Fields(7) { setType("S7"); }
  explicit S7(uint64_t data): FT4FT8Fields(7, data) { setType("S7"); }
  explicit S7(std::vector<bool> data): FT4FT8Fields(7, data) { setType("S7"); }

  ~S7(void) {}
};
/* ---------------------------------------------------------------------- */
class t1 : public FT4FT8Fields {
 private:
 public:
  t1(void): FT4FT8Fields(1) { setType("t1"); }
  explicit t1(uint64_t data): FT4FT8Fields(1, data) { setType("t1"); }
  explicit t1(std::vector<bool> data): FT4FT8Fields(1, data) { setType("t1"); }

  ~t1(void) {}
};
/* ---------------------------------------------------------------------- */
class t71 : public  FT4FT8Fields {
 private:
 public:
  t71(void): FT4FT8Fields(71) { setType("t71"); }
  explicit t71(uint64_t data): FT4FT8Fields(71, data) { setType("t71"); }
  explicit t71(std::vector<bool> data): FT4FT8Fields(71, data) { setType("t71"); }

  ~t71(void) {}
};
/* ---------------------------------------------------------------------- */
class cs14 : public  FT4FT8Fields {
 private:
 public:
  cs14(void): FT4FT8Fields(14) { setType("cs14"); }
  explicit cs14(uint64_t data): FT4FT8Fields(14, data) { setType("cs14"); }
  explicit cs14(std::vector<bool> data): FT4FT8Fields(14, data) { setType("cs14"); }

  ~cs14(void) {}
};
/* ---------------------------------------------------------------------- */
class ldpc83 : public  FT4FT8Fields {
 private:
 public:
  ldpc83(void): FT4FT8Fields(83) { setType("ldpc83"); }
  explicit ldpc83(uint64_t data): FT4FT8Fields(83, data) { setType("ldpc83"); }
  explicit ldpc83(std::vector<bool> data): FT4FT8Fields(83, data) { setType("ldpc83"); }

  ~ldpc83(void) {}
};
class costas21 : public  FT4FT8Fields {
 private:
 public:
  costas21(void);
  ~costas21(void) {}
};
class payload174 : public  FT4FT8Fields {
 private:
 public:
  explicit payload174(const FT4FT8Fields & parts);
  explicit payload174(const std::vector<bool> & data);
  ~payload174(void) {}
};
class FT8Message237 : public  FT4FT8Fields {
 private:
  std::vector<bool> mappedPayloadBits;
  std::vector<bool> unmappedPayloadBits;
 public:
  explicit FT8Message237(const FT4FT8Fields & payload);
  std::vector<bool> getUnmappedPayloadBits() { return unmappedPayloadBits; }
  ~FT8Message237(void) {}
};
#endif  //  FT4FT8FIELDS_H_
