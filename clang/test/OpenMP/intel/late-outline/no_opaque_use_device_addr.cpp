//INTEL_COLLAB
//RUN: %clang_cc1 -no-opaque-pointers -emit-llvm -o - -fopenmp -fopenmp-version=50 \
//RUN:  -fopenmp-late-outline \
//RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s

struct S {
  int a = 0;
  int *ptr = &a;
  int &ref = a;
  int *aaptr = &a;
  int arr[4];
  S() {}
  void foo();
};

//CHECK-LABEL: S3foo
void S::foo() {
  //CHECK: [[A:%a.*]] = getelementptr {{.*}}, i32 0, i32 0{{$}}
  //CHECK: [[PTR:%ptr.*]] = getelementptr {{.*}}, i32 0, i32 1{{$}}
  //CHECK: [[REF:%ref.*]] = getelementptr {{.*}}, i32 0, i32 2{{$}}
  //CHECK: [[LREF:%[0-9]+]] = load i32*, i32** [[REF]], align 8
  //CHECK: [[AAPTR:%aaptr.*]] = getelementptr {{.*}}, i32 0, i32 3{{$}}
  //CHECK: [[ARR:%arr.*]] = getelementptr {{.*}}, i32 0, i32 4{{$}}

  //CHECK: "DIR.OMP.TARGET.DATA"()
  //CHECK: QUAL.OMP.USE_DEVICE_ADDR{{.*}}[[A]])
  //CHECK: QUAL.OMP.USE_DEVICE_ADDR{{.*}}[[PTR]])
  //CHECK: QUAL.OMP.USE_DEVICE_ADDR{{.*}}[[LREF]])
  //CHECK: QUAL.OMP.USE_DEVICE_ADDR{{.*}}[[AAPTR]])
  //CHECK: QUAL.OMP.USE_DEVICE_ADDR{{.*}}[[ARR]])
  #pragma omp target data use_device_addr(a, ptr [3:4], ref, aaptr[0], arr[:a])
  ++a, ++*ptr, ++ref, ++arr[0];
  //CHECK: "DIR.OMP.END.TARGET.DATA"()
}


//CHECK-LABEL: foo
void foo() {
  int i;
  int &j = i;
  int *k = &j;
  //CHECK-DAG: [[I:%i.*]] = alloca i32,
  //CHECK-DAG: [[J:%j.*]] = alloca i32*,
  //CHECK-DAG: [[K:%k.*]] = alloca i32*,

  //CHECK: "DIR.OMP.TARGET.DATA"()
  //CHECK: QUAL.OMP.USE_DEVICE_ADDR{{.*}}[[I]])
  //CHECK: QUAL.OMP.USE_DEVICE_ADDR{{.*}}[[J]])
  //CHECK: QUAL.OMP.USE_DEVICE_ADDR{{.*}}[[K]],
  #pragma omp target data map(tofrom: i) use_device_addr(i, j, k[:i])
  {
    i++; j++; k++;
  }
  //CHECK: "DIR.OMP.END.TARGET.DATA"()
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
  s.foo();

  //CHECK: "DIR.OMP.TARGET.DATA"()
  //CHECK: QUAL.OMP.USE_DEVICE_ADDR{{.*}}[[A]])
  //CHECK: QUAL.OMP.USE_DEVICE_ADDR:ARRSECT{{.*}}[[PTR]], i64 1, i64 3,
  //CHECK: QUAL.OMP.USE_DEVICE_ADDR{{.*}}[[REF]])
  //CHECK: QUAL.OMP.USE_DEVICE_ADDR:ARRSECT{{.*}}[[PTR]], i64 1, i64 0,
  //CHECK: QUAL.OMP.USE_DEVICE_ADDR:ARRSECT{{.*}}[[ARR]],
  //CHECK: QUAL.OMP.USE_DEVICE_ADDR:ARRSECT{{.*}}[[VLA]],
  #pragma omp target data \
               use_device_addr(a, ptr [3:4], ref, ptr[0], arr[:(int)a], vla[0])
  ++a, ++*ptr, ++ref, ++arr[0], ++vla[0];
  //CHECK: "DIR.OMP.END.TARGET.DATA"()
  return a;
}

// end INTEL_COLLAB
