// INTEL_COLLAB
// RUN: %clang_cc1 -no-opaque-pointers -isystem %S/Inputs -emit-llvm -o - -fopenmp \
// RUN: -fopenmp-late-outline -fopenmp-version=50 \
// RUN: -triple x86_64-unknown-linux-gnu %s | FileCheck %s

#include <omp.h>

int fails;
//CHECK-LABEL: normal_alignment_is_device_ptr
void normal_alignment_is_device_ptr()
{
  //CHECK: [[I:%i[0-9]*]] = alloca i32, align 4
  //CHECK: [[PTR:%ptr[0-9]*]] = alloca [[TYPE:\[10 x i32\]\*]], align 8

  int i;
  int (*ptr)[10] = omp_target_alloc(100,0);

  // Normal device_ptr is passed by value.
  // Flags are OMP_MAP_TARGET_PARAM|OMP_MAP_LITERAL (currently 0x120 or 288)

  //CHECK: [[L:%[0-9]+]] = load [[TYPE]], [[TYPE]]* [[PTR]], align 8
  //CHECK: "DIR.OMP.TARGET"
  //CHECK-SAME: "QUAL.OMP.IS_DEVICE_PTR:PTR_TO_PTR"([10 x i32]** %ptr)
  //CHECK-SAME: MAP.FROM"(i32* [[I]], i32* [[I]], i64 4,
  //CHECK: "DIR.OMP.END.TARGET"
  #pragma omp target device(0) map(from:i) is_device_ptr(ptr)
  {
    ptr[1][1] = 41;
    i = ptr[1][1] + 1;
  }
  if (i != 42) fails++;
}

//CHECK-LABEL: specific_alignment_is_device_ptr
void specific_alignment_is_device_ptr()
{
  //CHECK: [[I:%i[0-9]*]] = alloca i32, align 4
  //CHECK: [[PTR:%ptr[0-9]*]] = alloca [[TP:\[10 x i32\]\*]], align 64
  int i;
  __attribute__((aligned(64))) int (*ptr)[10] = omp_target_alloc(100,0);

  // Overaligned device_ptr is passed by reference.
  // Flags are OMP_MAP_TARGET_PARAM|OMP_MAP_TO (currently 0x21 or 33)

  //CHECK: "DIR.OMP.TARGET"
  //CHECK-SAME: "QUAL.OMP.IS_DEVICE_PTR:PTR_TO_PTR"([10 x i32]** %ptr)
  //CHECK-SAME: MAP.FROM"(i32* [[I]], i32* [[I]], i64 4,
  //CHECK: "DIR.OMP.END.TARGET"
  #pragma omp target device(0) map(from:i) is_device_ptr(ptr)
  {
    ptr[1][1] = 41;
    i = ptr[1][1] + 1;
  }
  if (i != 42) fails++;
}

// end INTEL_COLLAB
