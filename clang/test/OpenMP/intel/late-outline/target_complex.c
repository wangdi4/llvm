// INTEL_COLLAB
// RUN: %clang_cc1 -opaque-pointers -emit-llvm -o -  -triple x86_64-pc-windows-msvc19.16.27041 \
// RUN: -fopenmp -fintel-compatibility -fopenmp-late-outline -fopenmp-typed-clauses \
// RUN: -fopenmp-targets=spir64 -DWINDOW %s | FileCheck %s \
// RUN: --check-prefix CHECK-WIN
//
// RUN: %clang_cc1 -opaque-pointers -verify -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline -fopenmp-typed-clauses \
// RUN: -fopenmp-targets=spir64 -emit-llvm-bc %s -o %t-host.bc
//
// RUN: %clang_cc1 -opaque-pointers -verify -triple spir64 -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline -fopenmp-typed-clauses \
// RUN:  -fopenmp-targets=spir64 -fopenmp-is-device \
// RUN:  -fopenmp-host-ir-file-path %t-host.bc %s -emit-llvm -o - \
// RUN:  | FileCheck %s
//
// expected-no-diagnostics

// Checks that temps created for complex return appear in private clauses.
extern double cabs (double _Complex __z) __attribute__ ((__nothrow__ ));

#if defined(WINDOW)
void foo()
{
  //CHECK-WIN:[[XT:%xTmp]] = alloca [[TP:{ double, double }]], align 8

  //CHECK-WIN:[[TH:%thresh]] = alloca double, align 8

  //CHECK-WIN:[[IAT:%indirect-arg-temp[0-9]*]] = alloca [[TP]], align 8

  //CHECK-WIN: "DIR.OMP.TARGET"()
  //CHECK-WIN-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[XT]]
  //CHECK-WIN-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[TH]]
  //CHECK-WIN-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[IAT]]
  //CHECK-WIN: "DIR.OMP.PARALLEL"()
  //CHECK-WIN-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[XT]]
  //CHECK-WIN-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[TH]]
  //CHECK-WIN-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[IAT]]
  #pragma omp target parallel
  {
    double _Complex xTmp = 0;
    double thresh = 1.0;
    if (cabs(xTmp) > thresh)
    { }
  }
}

#else

void foo()
{
  //CHECK:[[XT:%xTmp]] = alloca [[TP:{ double, double }]], align 8
  //CHECK:[[TH:%thresh]] = alloca double, align 8
  //CHECK:[[IAT:%indirect-arg-temp[0-9]*]] = alloca [[TP]], align 8

  //CHECK:[[XTA:%xTmp.ascast]] =
  //CHECK-SAME: addrspacecast ptr [[XT]] to ptr [[AS:addrspace\(4\)]]
  //CHECK:[[THA:%thresh.ascast]] =
  //CHECK-SAME: addrspacecast ptr [[TH]] to ptr [[AS]]
  //CHECK:[[IATA:%indirect-arg-temp[0-9]*.ascast]] =
  //CHECK-SAME: addrspacecast ptr [[IAT]] to ptr [[AS]]

  //CHECK: "DIR.OMP.TARGET"()
  //CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[AS]] [[XTA]]
  //CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[AS]] [[THA]]
  //CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[AS]] [[IATA]]
  //CHECK: "DIR.OMP.PARALLEL"()
  //CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[AS]] [[XTA]]
  //CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[AS]] [[THA]]
  //CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[AS]] [[IATA]]
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

void zoo(int i);
int bar() {
  //CHECK: "DIR.OMP.TARGET"()
  //CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast
  //CHECK-NOT: "QUAL.OMP.PRIVATE:TYPED"(ptr %i
  #pragma omp target
  {
    int i = 0;
    zoo(i);
  }
  return 0;
}
#endif
// end INTEL_COLLAB
