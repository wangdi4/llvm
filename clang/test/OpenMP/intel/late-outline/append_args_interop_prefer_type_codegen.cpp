// INTEL_COLLAB
//RUN: %clang_cc1 -triple x86_64-pc-linux-gnu -fopenmp -fopenmp-version=51     \
//RUN:   -fopenmp-late-outline -x c++ -std=c++14 -fexceptions -fcxx-exceptions \
//RUN:   -Wno-source-uses-openmp -Wno-openmp-clauses                           \
//RUN:   -emit-llvm -verify -o - %s | FileCheck %s

//RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fopenmp -fopenmp-version=51  \
//RUN:   -fopenmp-late-outline -x c++ -std=c++14 -fexceptions -fcxx-exceptions \
//RUN:   -Wno-source-uses-openmp -Wno-openmp-clauses                           \
//RUN:   -emit-llvm -verify -o - %s | FileCheck %s

//RUN: %clang_cc1 -triple x86_64-pc-linux-gnu -fopenmp -fopenmp-version=51     \
//RUN:   -fopenmp-late-outline -x c++ -std=c++14 -fexceptions -fcxx-exceptions \
//RUN:   -Wno-source-uses-openmp -Wno-openmp-clauses                           \
//RUN:   -emit-pch -o %t.1 %s

//RUN: %clang_cc1 -triple x86_64-pc-linux-gnu -fopenmp -fopenmp-version=51     \
//RUN:   -fopenmp-late-outline -x c++ -std=c++14 -fexceptions -fcxx-exceptions \
//RUN:   -Wno-source-uses-openmp -Wno-openmp-clauses                           \
//RUN:   -emit-llvm -o - -include-pch %t.1 %s | FileCheck %s

// expected-no-diagnostics

#ifndef HEADER
#define HEADER

typedef void *omp_interop_t;

void variant_one(float* F1, float *F2, omp_interop_t);
void variant_two(float* F1, float *F2, omp_interop_t, omp_interop_t);

//CHECK: define{{.*}}barOne{{.*}}#[[BAR_ONE_A:[0-9]+]]
//CHECK: define{{.*}}barTwo{{.*}}#[[BAR_TWO_A:[0-9]+]]
//CHECK: define{{.*}}barThree{{.*}}#[[BAR_THREE_A:[0-9]+]]
//CHECK: define{{.*}}barFour{{.*}}#[[BAR_FOUR_A:[0-9]+]]
//CHECK: define{{.*}}barFive{{.*}}#[[BAR_FIVE_A:[0-9]+]]
//CHECK: define{{.*}}templatebarOne{{.*}}#[[TEMPBAR_ONE_A:[0-9]+]]

//CHECK: define{{.*}}memberbarOne{{.*}}#[[MEMBAR_ONE_A:[0-9]+]]
//CHECK: define{{.*}}memberbarTwo{{.*}}#[[MEMBAR_ONE_A]]
//CHECK: define{{.*}}smemberbarOne{{.*}}#[[SMEMBAR_ONE_A:[0-9]+]]
//CHECK: define{{.*}}tempmembar{{.*}}#[[TEMPMEMBAR_1:[0-9]+]]
//CHECK: define{{.*}}tempmembar{{.*}}#[[TEMPMEMBAR_3:[0-9]+]]
//CHECK: define{{.*}}tempmembar{{.*}}#[[TEMPMEMBAR_4:[0-9]+]]
//CHECK: define{{.*}}tempmembar{{.*}}#[[TEMPMEMBAR_6:[0-9]+]]

// '1' not supported and ignored.
//CHECK: attributes #[[BAR_ONE_A]] = {{.*}}"openmp-variant"{{.*}}variant_one{{.*}};interop:target,targetsync,3"
#pragma omp declare variant(variant_one) match(construct={dispatch}) \
                                    append_args(interop(target,prefer_type(1,3),targetsync))
void barOne(float *FF1, float *FF2) { return; }

// "cuda" not supported, and ignored.
//CHECK: attributes #[[BAR_TWO_A]] = {{.*}}"openmp-variant"{{.*}}variant_one{{.*}};interop:targetsync"
#pragma omp declare variant(variant_one) match(construct={dispatch}) \
                                    append_args(interop(prefer_type("cuda"),targetsync))
void barTwo(float *FF1, float *FF2) { return; }

// '2' ignored, "sycl" is 4.
//CHECK: attributes #[[BAR_THREE_A]] = {{.*}}"openmp-variant"{{.*}}variant_one{{.*}};interop:target,4"
#pragma omp declare variant(variant_one) match(construct={dispatch}) \
                                    append_args(interop(target,prefer_type(2,"sycl")))
void barThree(float *FF1, float *FF2) { return; }

// Two interops with prefer_types.
//CHECK: attributes #[[BAR_FOUR_A]] = {{.*}}"openmp-variant"{{.*}}variant_two{{.*}};interop:target,6;interop:targetsync,4"
#pragma omp declare variant(variant_two) match(construct={dispatch}) \
                       append_args(interop(target,prefer_type("level_zero")), interop(prefer_type(4,1),targetsync))
void barFour(float *FF1, float *FF2) { return; }

// Check order is preserved.
//CHECK: attributes #[[BAR_FIVE_A]] = {{.*}}"openmp-variant"{{.*}}variant_two{{.*}};interop:target,6,4,3;interop:targetsync,3,4,6"
#pragma omp declare variant(variant_two) match(construct={dispatch}) \
                       append_args(interop(target,prefer_type("level_zero","sycl","opencl")), interop(prefer_type("opencl","sycl","level_zero"),targetsync))
void barFive(float *FF1, float *FF2) { return; }

template <typename T> void templatefoo_v1(const T& t, omp_interop_t I);
template <typename T> void templatebar(const T& t) {}

//CHECK: attributes #[[TEMPBAR_ONE_A]] = {{.*}}"openmp-variant"{{.*}}templatefoo_v1{{.*}};interop:target,targetsync,4,3"
#pragma omp declare variant(templatefoo_v1<int>) match(construct={dispatch}) \
  append_args(interop(prefer_type("sycl",3),targetsync,target))
void templatebarOne(const int &t) {}

class A {
public:
  void memberfoo_v1(float *A, float *B, int *I, omp_interop_t IOp);

  // Next two have the same metadata.
  //CHECK: attributes #[[MEMBAR_ONE_A]] = {{.*}}"openmp-variant"{{.*}}memberfoo_v1{{.*}};interop:target,6,4,3"
  #pragma omp declare variant(memberfoo_v1) match(construct={dispatch}) \
    append_args(interop(target,prefer_type(6,5,4,3,2,1)))
  void memberbarOne(float *A, float *B, int *I) { return; }

  #pragma omp declare variant(memberfoo_v1) match(construct={dispatch}) \
    append_args(interop(target,prefer_type("level_zero","hip","sycl","opencl","cuda_driver","cuda")))
  void memberbarTwo(float *A, float *B, int *I) { return; }

  static void smemberfoo_v1(float *A, float *B, int *I, omp_interop_t IOp);

  //CHECK: attributes #[[SMEMBAR_ONE_A]] = {{.*}}"openmp-variant"{{.*}}smemberfoo_v1{{.*}};interop:target,3,4,6"
  #pragma omp declare variant(smemberfoo_v1) match(construct={dispatch}) \
    append_args(interop(prefer_type("cuda","cuda_driver","opencl","sycl","hip","level_zero"),target))
  static void smemberbarOne(float *A, float *B, int *I) { return; }
};

template <signed d>
class X {
public:
  void tfoo_v1(float *A, float *B, int *I, omp_interop_t IOp);

  //CHECK: attributes #[[TEMPMEMBAR_1]] = {{.*}}"openmp-variant"{{.*}}tfoo_v1{{.*}};interop:target"
  //CHECK: attributes #[[TEMPMEMBAR_3]] = {{.*}}"openmp-variant"{{.*}}tfoo_v1{{.*}};interop:target,3"
  //CHECK: attributes #[[TEMPMEMBAR_4]] = {{.*}}"openmp-variant"{{.*}}tfoo_v1{{.*}};interop:target,4"
  //CHECK: attributes #[[TEMPMEMBAR_6]] = {{.*}}"openmp-variant"{{.*}}tfoo_v1{{.*}};interop:target,6"
  #pragma omp declare variant(tfoo_v1) match(construct={dispatch}) \
    append_args(interop(target, prefer_type(1,d)))
  void tempmembar(float *A, float *B, int *I) { return; }
};

int main()
{
  float f1 = 0.0;
  int I = 0;
  A a;
  a.memberbarOne(&f1, &f1, &I);
  a.memberbarTwo(&f1, &f1, &I);
  A::smemberbarOne(&f1, &f1, &I);
  templatebarOne(4);

  X<1> x1;
  X<3> x3;
  X<4> x4;
  X<6> x6;
  x1.tempmembar(&f1, &f1, &I);
  x3.tempmembar(&f1, &f1, &I);
  x4.tempmembar(&f1, &f1, &I);
  x6.tempmembar(&f1, &f1, &I);

  return 0;
}
#endif // HEADER
// end INTEL_COLLAB
