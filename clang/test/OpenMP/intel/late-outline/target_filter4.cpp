// INTEL_COLLAB

// RUN: %clang_cc1 -verify -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -mconstructor-aliases \
// RUN:  -fopenmp-targets=x86_64-pc-linux-gnu -emit-llvm-bc %s -o %t-host.bc
//
// RUN: %clang_cc1 -verify -triple x86_64-pc-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -mconstructor-aliases \
// RUN:  -fopenmp-targets=x86_64-pc-linux-gnu -fopenmp-is-device \
// RUN:  -fopenmp-host-ir-file-path %t-host.bc %s -emit-llvm -o - \
// RUN:  | FileCheck %s
//
// expected-no-diagnostics

// Verifies that routines on the deferred list with ctor/dtor aliases
// are marked with the correct attriubtes.

template<class type>
class CC {
  type x;
public:
  explicit CC(const double& a) { x = a; }
  ~CC() { x = -1.0; }

  template<class T>
  friend inline CC<T> CC_minus(const T* a);
};


template<class T>
inline CC<T> CC_minus(const T* a) {
  CC<T> result(*a);
  return result;
}

void bar() {

    #pragma omp target
    {
       double *dp;
       CC<double> wdiff = CC_minus(dp);
    }
}

//CHECK: define {{.*}}bar{{.*}}#[[CONTAINS_TARGET:[0-9]+]]
//CHECK: define {{.*}}minus{{.*}}#[[TARGET_DECLARE:[0-9]+]]
//CHECK: define {{.*}}_ZN2CCIdED2Ev{{.*}}#[[TARGET_DECLARE]]
//CHECK: define {{.*}}_ZN2CCIdEC2ERKd{{.*}}#[[TARGET_DECLARE]]

//CHECK: attributes #[[CONTAINS_TARGET]] = {{.*}}"contains-openmp-target"="true"
//CHECK: attributes #[[TARGET_DECLARE]] = {{.*}}"openmp-target-declare"="true"

// end INTEL_COLLAB
