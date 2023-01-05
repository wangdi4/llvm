// INTEL_COLLAB
// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu           \
// RUN:  -isystem %S/Inputs -emit-llvm -disable-llvm-passes   \
// RUN:  -fopenmp -fopenmp-version=51 -fopenmp-targets=x86_64 \
// RUN:  -fopenmp-late-outline -o - %s | FileCheck %s

// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu           \
// RUN:  -isystem %S/Inputs -emit-llvm -disable-llvm-passes   \
// RUN:  -fopenmp -fopenmp-version=51 -fopenmp-targets=x86_64 \
// RUN:  -fopenmp-late-outline -x c -o - %s | FileCheck %s -check-prefix CHECKC

#include <omp.h>

#ifdef __cplusplus

template <typename T>
struct SmallStruct {
  T real;
  T imag;
};


// CHECK: define{{.*}}foo_v1{{.*}}
void foo_v1(SmallStruct<double> &alpha, const SmallStruct<double> *&AAA,
    SmallStruct<double> beta, const SmallStruct<double> *&BBB,
    SmallStruct<double> gamma, const SmallStruct<double> *&CCC,
    SmallStruct<double> delta, ...) {return;}

// CHECK: define{{.*}}mfoo1{{.*}}({{.*}}%alpha{{.*}}, {{.*}}%AAA{{.*}}, {{.*}}%beta{{.*}}, {{.*}}%beta{{.*}}, {{.*}}%BBB{{.*}}, {{.*}}%gamma{{.*}}, {{.*}}%gamma{{.*}}, {{.*}}%CCC{{.*}}, {{.*}}%delta{{.*}}, {{.*}}%delta{{.*}}, ...)
#pragma omp declare variant (foo_v1) match(construct={dispatch}) 
void mfoo1(SmallStruct<double> &alpha, const SmallStruct<double> *&AAA,
    SmallStruct<double> beta, const SmallStruct<double> *&BBB,
    SmallStruct<double> gamma, const SmallStruct<double> *&CCC,
    SmallStruct<double> delta, ...) {return;}

// CHECK: define{{.*}}mfoo2{{.*}}({{.*}}%alpha{{.*}}, {{.*}}%AAA{{.*}}, {{.*}}%beta{{.*}}, {{.*}}%beta{{.*}}, {{.*}}%BBB{{.*}}, {{.*}}%gamma{{.*}}, {{.*}}%gamma{{.*}}, {{.*}}%CCC{{.*}}, {{.*}}%delta{{.*}}, {{.*}}%delta{{.*}}, ...)
#pragma omp declare variant (foo_v1) match(construct={target variant dispatch}) 
void mfoo2(SmallStruct<double> &alpha, const SmallStruct<double> *&AAA,
    SmallStruct<double> beta, const SmallStruct<double> *&BBB,
    SmallStruct<double> gamma, const SmallStruct<double> *&CCC,
    SmallStruct<double> delta, ...) {return;}

void boo1(SmallStruct<double> &alpha, const SmallStruct<double> *&AAA,
    SmallStruct<double> beta, const SmallStruct<double> *&BBB,
    SmallStruct<double> gamma, const SmallStruct<double> *&CCC,
    SmallStruct<double> delta) {
  #pragma omp target
  {
    // CHECK: "DIR.OMP.DISPATCH"()
    // CHECK-SAME: "QUAL.OMP.NEED_DEVICE_PTR:PTR_TO_PTR"(i32 2, i32 5, i32 8)
    #pragma omp dispatch need_device_ptr(2) need_device_ptr(4) need_device_ptr(6)
    mfoo1(alpha, AAA, beta, BBB, gamma, CCC, delta);
    
    // CHECK: "DIR.OMP.DISPATCH"()
    // CHECK-SAME: "QUAL.OMP.NEED_DEVICE_PTR"(i32 11)
    #pragma omp dispatch need_device_ptr(8)
    mfoo1(alpha, AAA, beta, BBB, gamma, CCC, delta, AAA);
    
    // CHECK: "DIR.OMP.DISPATCH"()
    // CHECK-SAME: "QUAL.OMP.NEED_DEVICE_PTR:PTR_TO_PTR"(i32 2, i32 5, i32 8)
    // CHECK-SAME: "QUAL.OMP.NEED_DEVICE_PTR"(i32 14)
    #pragma omp dispatch need_device_ptr(2, 4, 6, 10)
    mfoo1(alpha, AAA, beta, BBB, gamma, CCC, delta, beta, gamma, AAA);
    
    // CHECK: "DIR.OMP.DISPATCH"()
    // CHECK-SAME: "QUAL.OMP.NEED_DEVICE_PTR:PTR_TO_PTR"(i32 2, i32 5, i32 8)
    // CHECK-SAME: "QUAL.OMP.NEED_DEVICE_PTR"(i32 13, i32 15)
    #pragma omp dispatch need_device_ptr(2, 4, 6, 9, 11)
    mfoo1(alpha, AAA, beta, BBB, gamma, CCC, delta, beta, AAA, gamma, AAA);
  }

 // CHECK: "DIR.OMP.TARGET.VARIANT.DISPATCH"()
 // CHECK-SAME: "QUAL.OMP.NEED_DEVICE_PTR:PTR_TO_PTR"(i32 2, i32 5, i32 8)
 // CHECK-SAME: "QUAL.OMP.NEED_DEVICE_PTR"(i32 13)
  #pragma omp target variant dispatch need_device_ptr(2, 4, 6, 9)
  mfoo2(alpha, AAA, beta, BBB, gamma, CCC, delta, beta, &alpha);
}

void Foo_Var(float *AAA, float *BBB, omp_interop_t I1, omp_interop_t I2) {
  return;
}

#pragma omp declare variant(Foo_Var) \
   match(construct={dispatch}, device={arch(XeHP)}) \
   adjust_args(need_device_ptr:AAA) adjust_args(nothing:BBB) \
   append_args(interop(target,targetsync), interop(targetsync,target))
template<typename T>
void Foo(T *AAA, T *BBB) {return;}


// Check static and non-static variant member functions.
struct MyClass {
  static void foo_mv1(float *AAA, float *&BBB, int *I, ...) {return;}
  void foo_mv2(float *&AAA, float *BBB, int *I, ...) {return;}
  void foo_mv3(float *&AAA, float *&BBB, int *I, ...) {
    return;
  }
  #pragma omp declare variant(foo_mv1)                  \
     match(construct={dispatch}, device={arch(gen)}) 
  static void mfoo1(float *AAA, float *&BBB, int *I, ...) {return;}

  #pragma omp declare variant(foo_mv2)                       \
     match(construct={dispatch}, device={arch(gen9)})
  void mfoo2(float *&AAA, float *BBB, int *I, ...) {return;}

  #pragma omp declare variant(foo_mv3)                           \
     match(construct={target variant dispatch}, device={arch(XeLP,XeHP)})
  void mfoo3(float *&AAA, float *&BBB, int *I, ...) {return;}
};


void boo2(float *A, float *B, int *I)
{

  MyClass mc;
  int temp;
  #pragma omp target
  {
    // CHECK: "DIR.OMP.DISPATCH"()
    // CHECK-SAME: "QUAL.OMP.NEED_DEVICE_PTR"(i32 1, i32 2)
    #pragma omp dispatch need_device_ptr(1, 2)
    Foo(A, B);
    // CHECK: "DIR.OMP.DISPATCH"()
    // CHECK-SAME: "QUAL.OMP.NEED_DEVICE_PTR:PTR_TO_PTR"(i32 2)
    // CHECK-SAME: "QUAL.OMP.NEED_DEVICE_PTR"(i32 1, i32 3, i32 5)
    #pragma omp dispatch need_device_ptr(1, 2, 3, 5)
    mc.mfoo1(A, B, I, temp, I);
    // CHECK: "DIR.OMP.DISPATCH"()
    // CHECK-SAME: "QUAL.OMP.NEED_DEVICE_PTR:PTR_TO_PTR"(i32 2)
    // CHECK-SAME: "QUAL.OMP.NEED_DEVICE_PTR"(i32 3, i32 4)
    #pragma omp dispatch need_device_ptr(1, 2, 3)
    mc.mfoo2(A, B, I); 
  }
  // CHECK: "DIR.OMP.TARGET.VARIANT.DISPATCH"()
  // CHECK-SAME: "QUAL.OMP.NEED_DEVICE_PTR:PTR_TO_PTR"(i32 2, i32 3)
  // CHECK-SAME: "QUAL.OMP.NEED_DEVICE_PTR"(i32 4, i32 5, i32 7)
  #pragma omp target variant dispatch need_device_ptr(1, 2, 3, 4, 6)
  mc.mfoo3(A, B, I, I, temp, I);
}

// CHECK: define{{.*}}Foo{{.*}}({{.*}}AAA{{.*}}, {{.*}}BBB{{.*}}){{.*}}
// CHECK: define{{.*}}mfoo1{{.*}}({{.*}}AAA{{.*}}, {{.*}}BBB{{.*}}, {{.*}}I{{.*}}, ...){{.*}}
// CHECK: define{{.*}}mfoo2{{.*}}({{.*}}this{{.*}}, {{.*}}AAA{{.*}}, {{.*}}BBB{{.*}}, {{.*}}I{{.*}}, ...){{.*}}
// CHECK: define{{.*}}mfoo3{{.*}}({{.*}}this{{.*}}, {{.*}}AAA{{.*}}, {{.*}}BBB{{.*}}, {{.*}}I{{.*}}, ...){{.*}}


#else

void cfoo_v0(float *AAA, float *BBB, ...) {return;}

// CHECKC: define{{.*}}cfoo1({{.*}}AAA{{.*}}, {{.*}}BBB, ...){{.*}}
#pragma omp declare variant(cfoo_v0)                  \
   match(construct={dispatch}, device={arch(gen)})    \
   adjust_args(need_device_ptr:AAA,BBB)
void cfoo1(float *AAA, float *BBB, ...) {return;}

void boo3(float *A, float *B) {
  #pragma omp target
  {
    // CHECKC: "DIR.OMP.DISPATCH"()
    // CHECKC-SAME: "QUAL.OMP.NEED_DEVICE_PTR"(i32 1, i32 2, i32 3)
    #pragma omp dispatch need_device_ptr(1, 2, 3)
    cfoo1(A, B, A);
  }
}
#endif
// end INTEL_COLLAB
