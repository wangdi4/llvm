// INTEL_COLLAB
// RUN: %clang_cc1 -no-opaque-pointers -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN: -triple x86_64-unknown-linux-gnu %s | FileCheck \
// RUN: --check-prefixes=CHECK,INSTR %s
//
// RUN: %clang_cc1 -no-opaque-pointers -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN: -triple x86_64-unknown-linux-gnu -DCOMBINED %s | \
// RUN: FileCheck --check-prefixes=CHECK,INSTR %s
//
// RUN: %clang_cc1 -no-opaque-pointers -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN: -fintel-openmp-region-atomic -triple x86_64-unknown-linux-gnu %s | \
// RUN: FileCheck --check-prefixes=CHECK,REGION %s
//
// RUN: %clang_cc1 -no-opaque-pointers -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN: -fintel-openmp-region-atomic -triple x86_64-unknown-linux-gnu \
// RUN: -DCOMBINED %s | FileCheck --check-prefixes=CHECK,REGION %s

// Verify omp atomic can closely nest within omp teams when using default
// OpenMP late outlining mode.

int main(int argc, char* argv[])
{
  int data[1] = {0};
//CHECK: "DIR.OMP.TARGET"
//CHECK-SAME: "QUAL.OMP.OFFLOAD.ENTRY.IDX"
//CHECK-SAME: QUAL.OMP.MAP.TOFROM"
//CHECK: "DIR.OMP.TEAMS"
//CHECK-SAME: QUAL.OMP.NUM_TEAMS"
//INSTR: atomicrmw
//REGION: "DIR.OMP.ATOMIC"
//REGION-SAME: "QUAL.OMP.UPDATE"
//REGION: fence acquire
//REGION: %[[ARRY:[a-z]+]] = getelementptr inbounds [1 x i32], [1 x i32]* %data, i64 0, i64 0
//REGION: %[[AVAL:[0-9]+]] = load i32, i32* %[[ARRY]], align 4
//REGION: %[[ADD:[0-9,a-z]+]] = add nsw i32 %[[AVAL]], 1
//REGION: store i32 %[[ADD]], i32* %[[ARRY]], align 4
//REGION: fence release
//REGION: "DIR.OMP.END.ATOMIC"
//CHECK: "DIR.OMP.END.TEAMS"
//CHECK: "DIR.OMP.END.TARGET"
#ifdef COMBINED
#pragma omp target teams map(tofrom:data) num_teams(16)
#else
#pragma omp target map(tofrom:data)
#pragma omp teams num_teams(16)
#endif
  {
#pragma omp atomic
    data[0] += 1;
  }
  if (data[0] == 16)
    return 0;
  else
    return 1;
}
// end INTEL_COLLAB
