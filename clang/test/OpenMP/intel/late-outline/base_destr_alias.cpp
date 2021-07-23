// INTEL_COLLAB
//RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -fopenmp  \
//RUN:  -fopenmp-late-outline -fopenmp-targets=spir64,x86_64 \
//RUN:  -O2 -disable-llvm-passes                             \
//RUN:  -mconstructor-aliases %s -emit-llvm-bc -o %t-host.bc

//RUN: %clang_cc1 -triple spir64 -aux-triple x86_64-unknown-linux-gnu \
//RUN:  -fopenmp -fopenmp-late-outline -fopenmp-targets=spir64,x86_64 \
//RUN:  -fopenmp-is-device -fopenmp-host-ir-file-path %t-host.bc      \
//RUN:  -O2 -disable-llvm-passes                                      \
//RUN:  -mconstructor-aliases %s -emit-llvm -o - | FileCheck %s

//RUN: %clang_cc1 -triple x86_64 -aux-triple x86_64-unknown-linux-gnu \
//RUN:  -fopenmp -fopenmp-late-outline -fopenmp-targets=spir64,x86_64 \
//RUN:  -fopenmp-is-device -fopenmp-host-ir-file-path %t-host.bc      \
//RUN:  -O2 -disable-llvm-passes                                      \
//RUN:  -mconstructor-aliases %s -emit-llvm -o - | FileCheck %s

// Verifies a linkable result when the base destructor alias optimization would
// be possible.

class Few {
public:
  ~Few() {};
};

class foo : public Few {
};

int main() {
  #pragma omp target
  foo a;
  return 0;
}

//CHECK: define {{.*}}main{{.*}}#[[ATTR:[0-9]]] {
//CHECK: "DIR.OMP.TARGET"()
//CHECK: call {{.*}}@_ZN3fooD2Ev
//CHECK: "DIR.OMP.END.TARGET"()

//CHECK: define {{.*}}@_ZN3fooD2Ev{{.*}}#[[ATTR2:[0-9]]]
//CHECK: call {{.*}}@_ZN3FewD2Ev

//CHECK: define {{.*}}@_ZN3FewD2Ev{{.*}}#[[ATTR3:[0-9]]]

//CHECK: attributes #[[ATTR]] = {{.*}}"contains-openmp-target"="true"
//CHECK: attributes #[[ATTR2]] = {{.*}}"openmp-target-declare"="true"
//CHECK: attributes #[[ATTR3]] = {{.*}}"openmp-target-declare"="true"
// end INTEL_COLLAB
