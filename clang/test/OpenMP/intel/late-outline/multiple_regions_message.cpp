// INTEL_COLLAB
//
//RUN: %clang_cc1 -fopenmp -fopenmp-late-outline -fopenmp-version=51 \
//RUN:  -I%S/Inputs -verify -emit-llvm -o - %s

void foo() {
  int i = 0;

#define VALUE 1
#include "multiple_regions_message.inc"

#undef VALUE
#define VALUE 2
#include "multiple_regions_message.inc"

}

#define A()\
_Pragma("omp target")\
{}\
_Pragma("omp target")\
{}

void bar()
{
  A() // expected-error {{multiple target regions at same the location is not supported}}
}

// end INTEL_COLLAB
