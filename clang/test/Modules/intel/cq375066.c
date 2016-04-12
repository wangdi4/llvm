// CQ#375066
// RUN: %clang_cc1 -fintel-compatibility -I%S/Inputs/cq375066/1 -I%S/Inputs/cq375066/2 -verify %s
// RUN: %clang_cc1 -I%S/Inputs/cq375066/1 -I%S/Inputs/cq375066/2 -verify %s
// expected-no-diagnostics 

#include "cq375066.h"

#ifndef CQ375066_1_H
#error CQ375066_1_H not defined
#endif

#ifndef CQ375066_2_H
#error CQ375066_2_H not defined
#endif
