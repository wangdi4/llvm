// INTEL_COLLAB
// RUN: %clang_cc1 -verify -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN: -fopenmp-targets=spir64 -emit-llvm-bc %s -o %t-host.bc
//
// RUN: %clang_cc1 -verify -triple spir64 -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-targets=spir64 -fopenmp-is-device \
// RUN:  -fopenmp-host-ir-file-path %t-host.bc %s -emit-llvm -o - \
// RUN:  | FileCheck %s
//
// expected-no-diagnostics

// Checks that temps created for complex return appear in private clauses.
extern double cabs (double _Complex __z) __attribute__ ((__nothrow__ ));

void foo()
{
  //CHECK:[[XT:%xTmp]] = alloca [[TP:{ double, double }]], align 8
  //CHECK:[[XTA:%xTmp.ascast]] =
  //CHECK-SAME: addrspacecast [[TP]]* [[XT]] to [[TP]] [[AS:addrspace\(4\)]]*

  //CHECK:[[TH:%thresh]] = alloca double, align 8
  //CHECK:[[THA:%thresh.ascast]] =
  //CHECK-SAME: addrspacecast double* [[TH]] to double [[AS]]*

  //CHECK:[[IAT:%indirect-arg-temp[0-9]*]] = alloca [[TP]], align 8
  //CHECK:[[IATA:%indirect-arg-temp[0-9]*.ascast]] =
  //CHECK-SAME: addrspacecast [[TP]]* [[IAT]] to [[TP]] [[AS:addrspace\(4\)]]*

  //CHECK: "DIR.OMP.TARGET"()
  //CHECK-SAME: "QUAL.OMP.PRIVATE"([[TP]] [[AS]]* [[XTA]])
  //CHECK-SAME: "QUAL.OMP.PRIVATE"(double [[AS]]* [[THA]])
  //CHECK-SAME: "QUAL.OMP.PRIVATE"([[TP]] [[AS]]* [[IATA]])
  //CHECK: "DIR.OMP.PARALLEL"()
  //CHECK-SAME: "QUAL.OMP.PRIVATE"([[TP]] [[AS]]* [[XTA]])
  //CHECK-SAME: "QUAL.OMP.PRIVATE"(double [[AS]]* [[THA]])
  //CHECK-SAME: "QUAL.OMP.PRIVATE"([[TP]] [[AS]]* [[IATA]])
  #pragma omp target parallel
  {
    //CHECK: [[IATA]].realp =
    //CHECK: [[IATA]].imagp =
    double _Complex xTmp = 0;
    double thresh = 1.0;
    if (cabs(xTmp) > thresh)
    { }
  }
}
// end INTEL_COLLAB
