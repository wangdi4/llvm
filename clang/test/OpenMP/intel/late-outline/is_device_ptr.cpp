//INTEL_COLLAB
//RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fopenmp-version=50 \
//RUN:  -fopenmp-late-outline \
//RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s

struct S {
  int a = 0;
  int *ptr = &a;
  int &ref = a;
  int *aaptr = &a;
  int arr[4];
  S() {}
  void foo(float *&refptr);
};

//CHECK-LABEL: S3foo
void S::foo(float *&refptr) {
  //CHECK: [[REFPTR:%refptr.*]] = alloca float**, align 8
  //CHECK: [[LREFPTR:%[0-9]+]] = load float**, float*** [[REFPTR]]

  //CHECK: "DIR.OMP.TARGET"()
  //CHECK: QUAL.OMP.IS_DEVICE_PTR:PTR_TO_PTR"(i32** %ptr, float** [[LREFPTR]], i32** %aaptr, [4 x i32]* %arr)
  #pragma omp target is_device_ptr(ptr, refptr, aaptr, arr)
  ++a, ++*ptr, ++ref, ++arr[0];
  //CHECK: "DIR.OMP.END.TARGET"()
}


//CHECK-LABEL: foo
void foo() {
  int i;
  int &j = i;
  int *k = &j;
  //CHECK-DAG: [[K:%k.*]] = alloca i32*,

  //CHECK: "DIR.OMP.TARGET"()
  //CHECK: QUAL.OMP.IS_DEVICE_PTR:PTR_TO_PTR"(i32** %k)
  #pragma omp target map(tofrom: i) is_device_ptr(k)
  {
    i++; j++; k++;
  }
  //CHECK: "DIR.OMP.END.TARGET"()
}

//CHECK-LABEL: main
int main() {
  float a = 0;
  float *ptr = &a;
  float &ref = a;
  float arr[4];
  float vla[(int)a];

  //CHECK: [[A:%a.*]] = alloca float,
  //CHECK: [[PTR:%ptr.*]] = alloca float*,
  //CHECK: [[REF:%ref.*]] = alloca float*,
  //CHECK: [[ARR:%arr.*]] = alloca [4 x float],
  //CHECK: [[VLA:%vla.*]] = alloca float, i64

  S s;
  s.foo(ptr);

  //CHECK: "DIR.OMP.TARGET"()
  //CHECK: QUAL.OMP.IS_DEVICE_PTR:PTR_TO_PTR"(float** %ptr, [4 x float]* %arr, float* %vla)
  #pragma omp target \
               is_device_ptr(ptr, arr, vla)
  ++a, ++*ptr, ++ref, ++arr[0], ++vla[0];
  //CHECK: "DIR.OMP.END.TARGET"()
  return a;
}

// end INTEL_COLLAB
