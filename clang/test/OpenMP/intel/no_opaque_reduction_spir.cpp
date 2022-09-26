// INTEL_COLLAB
//RUN: %clang_cc1 -no-opaque-pointers -triple x86_64-unknown-linux-gnu \
//RUN:  -emit-llvm -disable-llvm-passes \
//RUN:  -fopenmp -fopenmp-targets=spir64 \
//RUN:  -fopenmp-late-outline -fintel-compatibility \
//RUN:  -Werror -Wsource-uses-openmp -o - %s

//RUN: %clang_cc1 -no-opaque-pointers -triple i386-unknown-linux-gnu \
//RUN:  -emit-llvm -disable-llvm-passes \
//RUN:  -fopenmp -fopenmp-targets=spir \
//RUN:  -fopenmp-late-outline -fintel-compatibility \
//RUN:  -Werror -Wsource-uses-openmp -o - %s

//RUN: %clang_cc1 -no-opaque-pointers -triple x86_64-unknown-linux-gnu \
//RUN:  -emit-llvm-bc -disable-llvm-passes \
//RUN:  -fopenmp -fopenmp-targets=spir64 \
//RUN:  -fopenmp-late-outline -fintel-compatibility \
//RUN:  -Werror -Wsource-uses-openmp -o %t_host.bc %s

//RUN: %clang_cc1 -no-opaque-pointers -triple spir64 \
//RUN:  -emit-llvm -disable-llvm-passes \
//RUN:  -fopenmp -fopenmp-targets=spir64 \
//RUN:  -fopenmp-late-outline -fintel-compatibility \
//RUN:  -fopenmp-is-device -fopenmp-host-ir-file-path %t_host.bc \
//RUN:  -verify -Werror -Wsource-uses-openmp -o - %s \
//RUN:  | FileCheck %s

//RUN: %clang_cc1 -no-opaque-pointers -triple i386-unknown-linux-gnu \
//RUN:  -emit-llvm-bc -disable-llvm-passes \
//RUN:  -fopenmp -fopenmp-targets=spir \
//RUN:  -fopenmp-late-outline -fintel-compatibility \
//RUN:  -Werror -Wsource-uses-openmp -o %t_host.bc %s

//RUN: %clang_cc1 -no-opaque-pointers -triple spir \
//RUN:  -emit-llvm -disable-llvm-passes \
//RUN:  -fopenmp -fopenmp-targets=spir \
//RUN:  -fopenmp-late-outline -fintel-compatibility \
//RUN:  -fopenmp-is-device -fopenmp-host-ir-file-path %t_host.bc \
//RUN:  -verify -Werror -Wsource-uses-openmp -o - %s \
//RUN:  | FileCheck %s

//expected-no-diagnostics
//

#pragma omp begin declare target
volatile double g, g_orig;
#pragma omp end declare target
volatile double &g1 = g_orig;
template <class T> struct S {
  T f;
  S(T a) : f(a + g) {}
  S() : f(g) {}
  operator T() { return T(); }
  S &operator&(const S &) { return *this; }
  ~S() {}
};


template <typename T, int length>
T tmain()
{
  T t;
  S<T> test;
  T t_var = T(), t_var1;
  T vec[] = {1, 2} ;
  S<T> s_arr[] = {1, 2};
  S<T> &var = test;
  S<T> var1;
  S<T> arr[length];
   #pragma omp declare reduction (&: S<int> : omp_out = omp_in) \
                                 initializer(omp_priv = S<int>(10))
  // CHECK: call token @llvm.directive.region.entry()
  // CHECK-SAME: "DIR.OMP.TARGET"
  // CHECK: call token @llvm.directive.region.entry()
  // CHECK-SAME: "DIR.OMP.TEAMS"
  // CHECK-SAME: "QUAL.OMP.REDUCTION.UDR"([42 x %struct.S] addrspace(4)* %arr.ascast,
  // CHECK-SAME: void (%struct.S addrspace(4)*, %struct.S addrspace(4)*)* @.omp_combiner.
  // CHECK-SAME: void (%struct.S addrspace(4)*, %struct.S addrspace(4)*)* @.omp_initializer.)
  // CHECK-SAME: "QUAL.OMP.REDUCTION.UDR:BYREF"(%struct.S addrspace(4)* addrspace(4)* %var.map.ptr.tmp.ascast,
  // CHECK-SAME: void (%struct.S addrspace(4)*, %struct.S addrspace(4)*)* @.omp_combiner.,
  // CHECK-SAME:  void (%struct.S addrspace(4)*, %struct.S addrspace(4)*)* @.omp_initializer.)
  // CHECK-SAME: "QUAL.OMP.REDUCTION.UDR"(%struct.S addrspace(4)* %var1.ascast,
  // CHECK-SAME:  void (%struct.S addrspace(4)*, %struct.S addrspace(4)*)* @.omp_combiner.,
  // CHECK-SAME:  void (%struct.S addrspace(4)*, %struct.S addrspace(4)*)* @.omp_initializer.)
#pragma omp target teams reduction(&:arr, var, var1)
  {
      var = var1;
      arr[1]=var;
  }
  // CHECK: [ "DIR.OMP.END.TEAMS"() ]
  // CHECK: [ "DIR.OMP.END.TARGET"() ]
  return T();
}

int main()
{
  return tmain<int, 42>();
}

