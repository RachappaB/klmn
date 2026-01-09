#pragma once

#define NUM_FEATURES 7
#define NUM_CLASSES 3
#define SCALE 1000

const int16_t W[NUM_CLASSES][NUM_FEATURES] = {
  { -5, -7, 0, -19, 133, -17, -3 },
  { 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, -3, 0, 0, 0, 0 },
};

const int16_t B[NUM_CLASSES] = { 0, -35472, 184530 };
