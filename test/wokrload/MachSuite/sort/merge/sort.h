#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "support.h"

#define SIZE 2048
#define TYPE uint64_t
#define TYPE_MAX INT64_MAX

void ms_mergesort(TYPE a[SIZE]);

////////////////////////////////////////////////////////////////////////////////
// Test harness interface code.

struct bench_args_t {
  TYPE a[SIZE];
};
