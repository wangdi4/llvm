// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-version=50 -fopenmp-targets=spir64 \
// RUN:  -emit-llvm %s -o - | FileCheck %s

// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-version=50 -fopenmp-targets=spir64 \
// RUN:  -emit-llvm-bc %s -o %t-host.bc
//
// RUN: %clang_cc1 -triple spir64 -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-version=50 -fopenmp-targets=spir64 -fopenmp-is-device \
// RUN:  -fopenmp-host-ir-file-path %t-host.bc %s -emit-llvm -o - | \
// RUN:  FileCheck %s


template <class T>
struct A {
  T value;
  A(T& v) : value(&v) {}
  A(const T& v) : value(v) {}
};

template <class T>
struct Wrapper;

template <class T>
struct Wrapper< A<T> > {
  typedef T value_type;
  static void mycombinerfunc(value_type& dest, const value_type& src) { }
  static void myinitfunc(value_type& val) { }
};

template <class RTy, class VTy>
struct Reducer {

  #pragma omp declare reduction(custom:VTy            \
    : Wrapper <RTy>::mycombinerfunc(omp_out, omp_in)) \
    initializer(Wrapper <RTy>::myinitfunc(omp_priv))

  static void reduce() {
    VTy Res = VTy();
    Wrapper<RTy>::myinitfunc(Res);

    #pragma omp target teams distribute parallel for \
                               map(tofrom:Res) reduction(custom:Res)
    for (int i = 0; i < 10; i++)
       Res += 1;
  }
};

void foo() {
  Reducer<A<int>, int> var;
  var.reduce();
}

// mycombinerfunc and myinitfunc are called only from the combiner and
// initializer of the UDR and must be emitted on target.
//CHECK-DAG: define{{.*}}mycombinerfunc
//CHECK-DAG: define{{.*}}myinitfunc
