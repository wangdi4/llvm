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
  //CHECK-SAME: "QUAL.OMP.MAP.TOFROM:CHAIN"(i32* [[A1:%a[0-9]+]]
  //CHECK-SAME: "QUAL.OMP.MAP.TOFROM:CHAIN"(i32** [[PTR1:%ptr[0-9]+]]
  //CHECK-SAME: "QUAL.OMP.MAP.TOFROM:CHAIN"(i32* [[LREF1:%[0-9]+]]
  //CHECK-SAME: "QUAL.OMP.MAP.TOFROM:CHAIN"(i32** [[AAPTR1:%aaptr[0-9]+]]
  //CHECK-SAME: "QUAL.OMP.MAP.TOFROM:CHAIN"([4 x i32]* [[ARR1:%arr[0-9]+]]
  //CHECK-SAME: "QUAL.OMP.USE_DEVICE_ADDR{{.*}}[[A]])
  //CHECK-SAME: "QUAL.OMP.USE_DEVICE_ADDR{{.*}}[[PTR]])
  //CHECK-SAME: "QUAL.OMP.USE_DEVICE_ADDR{{.*}}[[LREF]])
  //CHECK-SAME: "QUAL.OMP.USE_DEVICE_ADDR{{.*}}[[AAPTR]])
  //CHECK-SAME: "QUAL.OMP.USE_DEVICE_ADDR{{.*}}[[ARR]])
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
  //CHECK-DAG: [[KMAPTMP:%k.map.ptr.tmp]] = alloca i32*
  //CHECK-DAG: [[L0:%[0-9]+]] = load i32*, i32** [[J]]
  //CHECK-DAG: [[L1:%[0-9]+]] = load i32*, i32** [[J]]
  //CHECK-DAG: [[L2:%[0-9]+]] = load i32*, i32** [[K]]
  //CHECK-DAG: [[L3:%[0-9]+]] = load i32*, i32** [[J]]

  //CHECK: "DIR.OMP.TARGET.DATA"()
  //CHECK-DAG: "QUAL.OMP.MAP.TOFROM"(i32* [[I]]
  //CHECK-DAG: "QUAL.OMP.MAP.TOFROM"(i32* [[L1]]
  //CHECK-DAG: "QUAL.OMP.MAP.TOFROM"(i32* [[L2]]
  //CHECK-DAG: QUAL.OMP.USE_DEVICE_ADDR{{.*}}[[I]])
  //CHECK-DAG: QUAL.OMP.USE_DEVICE_ADDR{{.*}}[[L2]])
  //CHECK-DAG: "QUAL.OMP.LIVEIN"(i32** [[KMAPTMP]])
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
  //CHECK: [[PTRMPTMP:%ptr.map.ptr.tmp]] = alloca float*
  //CHECK: [[ARRMPTMP:%arr.map.ptr.tmp]] = alloca float*
  //CHECK: [[VLAMPTMP:%vla.map.ptr.tmp]] = alloca float*
  //CHECK: [[VLA:%vla.*]] = alloca float, i64

  //CHECK: [[LPTR:%[0-9]+]] = load float*, float** [[PTR]]
  //CHECK: [[LREF:%[0-9]+]] = load float*, float** [[REF]]
  //CHECK: [[ARDEC:%arraydecay]] = getelementptr inbounds

  S s;
  s.foo();

  //CHECK: "DIR.OMP.TARGET.DATA"()
  //CHECK: "QUAL.OMP.MAP.TOFROM"(float* [[A]]
  //CHECK: "QUAL.OMP.MAP.TOFROM"(float* [[LPTR]]
  //CHECK: "QUAL.OMP.MAP.TOFROM"(float* [[LREF]]
  //CHECK: "QUAL.OMP.MAP.TOFROM"(float* [[ARDEC]]
  //CHECK: "QUAL.OMP.MAP.TOFROM"(float* [[VLA]]
  //CHECK: QUAL.OMP.USE_DEVICE_ADDR{{.*}}[[A]])
  //CHECK: QUAL.OMP.USE_DEVICE_ADDR{{.*}}[[LPTR]])
  //CHECK: "QUAL.OMP.USE_DEVICE_ADDR:BYREF"{{.*}}[[REF]])
  //CHECK: QUAL.OMP.USE_DEVICE_ADDR{{.*}}[[ARDEC]])
  //CHECK: QUAL.OMP.USE_DEVICE_ADDR{{.*}}[[VLA]])
  //CHECK: "QUAL.OMP.LIVEIN"(float** [[PTRMPTMP]]
  //CHECK: "QUAL.OMP.LIVEIN"(float** [[ARRMPTMP]]
  //CHECK: "QUAL.OMP.LIVEIN"(float** [[VLAMPTMP]]
  #pragma omp target data \
               use_device_addr(a, ptr [3:4], ref, ptr[0], arr[:(int)a], vla[0])
  ++a, ++*ptr, ++ref, ++arr[0], ++vla[0];
  //CHECK: "DIR.OMP.END.TARGET.DATA"()
  return a;
}

// end INTEL_COLLAB
