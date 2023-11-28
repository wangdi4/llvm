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
  void foo();
};

//CHECK-LABEL: S3foo
void S::foo() {
  //CHECK: [[A:%a]] = getelementptr {{.*}}, i32 0, i32 0{{$}}
  //CHECK: [[PTR:%ptr]] = getelementptr {{.*}}, i32 0, i32 1{{$}}
  //CHECK: [[REF:%ref]] = getelementptr {{.*}}, i32 0, i32 2{{$}}
  //CHECK: [[LREF:%[0-9]+]] = load ptr, ptr [[REF]], align 8
  //CHECK: [[AAPTR:%aaptr]] = getelementptr {{.*}}, i32 0, i32 3{{$}}
  //CHECK: [[ARR:%arr]] = getelementptr {{.*}}, i32 0, i32 4{{$}}

  //CHECK: "DIR.OMP.TARGET.DATA"()
  //CHECK-SAME: "QUAL.OMP.MAP.TOFROM:CHAIN"(ptr [[A1:%a[0-9]+]]
  //CHECK-SAME: "QUAL.OMP.MAP.TOFROM:CHAIN"(ptr [[PTR1:%ptr[0-9]+]]
  //CHECK-SAME: "QUAL.OMP.MAP.TOFROM:CHAIN"(ptr [[LREF1:%[0-9]+]]
  //CHECK-SAME: "QUAL.OMP.MAP.TOFROM:CHAIN"(ptr [[AAPTR1:%aaptr[0-9]+]]
  //CHECK-SAME: "QUAL.OMP.MAP.TOFROM:CHAIN"(ptr [[ARR1:%arr[0-9]+]]
  //CHECK-SAME: QUAL.OMP.USE_DEVICE_ADDR{{.*}}[[A]])
  //CHECK-SAME: QUAL.OMP.USE_DEVICE_ADDR{{.*}}[[PTR]])
  //CHECK-SAME: QUAL.OMP.USE_DEVICE_ADDR{{.*}}[[LREF]])
  //CHECK-SAME: QUAL.OMP.USE_DEVICE_ADDR{{.*}}[[AAPTR]])
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
  //CHECK-DAG: [[J:%j.*]] = alloca ptr,
  //CHECK-DAG: [[K:%k.*]] = alloca ptr,
  //CHECK-DAG: [[KMAPTMP:%k.map.ptr.tmp]] = alloca ptr
  //CHECK-DAG: [[L0:%[0-9]+]] = load ptr, ptr [[J]]
  //CHECK-DAG: [[L1:%[0-9]+]] = load ptr, ptr [[J]]
  //CHECK-DAG: [[L2:%[0-9]+]] = load ptr, ptr [[K]]
  //CHECK-DAG: [[L3:%[0-9]+]] = load ptr, ptr [[J]]

  //CHECK: "DIR.OMP.TARGET.DATA"()
  //CHECK-DAG: "QUAL.OMP.MAP.TOFROM"(ptr [[I]]
  //CHECK-DAG: "QUAL.OMP.MAP.TOFROM"(ptr [[L1]]
  //CHECK-DAG: "QUAL.OMP.MAP.TOFROM"(ptr [[L2]]
  //CHECK-DAG: QUAL.OMP.USE_DEVICE_ADDR{{.*}}[[I]])
  //CHECK-DAG: "QUAL.OMP.USE_DEVICE_ADDR:BYREF"{{.*}}[[J]])
  //CHECK-DAG: QUAL.OMP.USE_DEVICE_ADDR{{.*}}[[L2]])
  //CHECK-DAG: "QUAL.OMP.LIVEIN"(ptr [[KMAPTMP]])
  #pragma omp target data map(tofrom: i) use_device_addr(i, j, k[:i])
  {
    i++; j++; k++;
  }
  //CHECK: "DIR.OMP.END.TARGET.DATA"()
}

int a[10];
extern "C" int printf(const char*,...);
//CHECK-LABEL: zoo
void zoo() {
  a[1] = 111;
  int *p = &a[0];
  int *&pref = p;
  //CHECK-DAG: [[P:%p.*]] = alloca ptr,
  //CHECK-DAG: [[PREF:%pref.*]] = alloca ptr,
  //CHECK-DAG: [[PREFTEMP:%pref.map.ptr.tmp]] = alloca ptr,
  //CHECK-DAG: [[LPREF:%[0-9]+]] = load ptr, ptr [[PREF]],
  //CHECK: "DIR.OMP.TARGET.DATA"()
  //CHECK-SAME: "QUAL.OMP.MAP.TO"(ptr [[P]]
  //CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr [[LPREF]],
  //CHECK-SAME: "QUAL.OMP.USE_DEVICE_ADDR{{.*}}[[LPREF]])
  //CHECK-SAME: "QUAL.OMP.LIVEIN"(ptr [[PREFTEMP]]
  //CHECK-DAG: store ptr [[LPREF]], ptr [[PREFTEMP]]
  //CHECK-DAG: load ptr, ptr [[PREFTEMP]]
#pragma omp target data map(to:p) use_device_addr(pref)
  printf("%p\n", &pref);
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
  //CHECK: [[PTR:%ptr.*]] = alloca ptr,
  //CHECK: [[REF:%ref.*]] = alloca ptr,
  //CHECK: [[ARR:%arr.*]] = alloca [4 x float],
  //CHECK: [[PTRMPTMP:%ptr.map.ptr.tmp]] = alloca ptr
  //CHECK: [[ARRMPTMP:%arr.map.ptr.tmp]] = alloca ptr
  //CHECK: [[VLAMPTMP:%vla.map.ptr.tmp]] = alloca ptr
  //CHECK: [[VLA:%vla.*]] = alloca float, i64
  
  //CHECK: [[LPTR:%[0-9]+]] = load ptr, ptr [[PTR]]
  //CHECK: [[LREF:%[0-9]+]] = load ptr, ptr [[REF]]
  //CHECK: [[ARDEC:%arraydecay]] = getelementptr inbounds
  //CHECK: [[LPTRMAP:%[0-9]+]] = load ptr, ptr [[REF]]

  S s;
  s.foo();

  //CHECK: "DIR.OMP.TARGET.DATA"()
  //CHECK: "QUAL.OMP.MAP.TOFROM"(ptr [[A]]
  //CHECK: "QUAL.OMP.MAP.TOFROM"(ptr [[LPTR]]
  //CHECK: "QUAL.OMP.MAP.TOFROM"(ptr [[LREF]]
  //CHECK: "QUAL.OMP.MAP.TOFROM"(ptr [[ARDEC]]
  //CHECK: "QUAL.OMP.MAP.TOFROM"(ptr [[VLA]]

  //CHECK: QUAL.OMP.USE_DEVICE_ADDR{{.*}}[[A]])
  //CHECK: QUAL.OMP.USE_DEVICE_ADDR{{.*}}[[LPTR]])
  //CHECK: QUAL.OMP.USE_DEVICE_ADDR:BYREF"{{.*}}[[REF]])
  //CHECK: QUAL.OMP.USE_DEVICE_ADDR{{.*}}[[ARDEC]])
  //CHECK: QUAL.OMP.USE_DEVICE_ADDR{{.*}}[[VLA]])

  //CHECK: "QUAL.OMP.LIVEIN"(ptr [[PTRMPTMP]]
  //CHECK: "QUAL.OMP.LIVEIN"(ptr [[ARRMPTMP]]
  //CHECK: "QUAL.OMP.LIVEIN"(ptr [[VLAMPTMP]]

  #pragma omp target data \
               use_device_addr(a, ptr [3:4], ref, ptr[0], arr[:(int)a], vla[0])
  ++a, ++*ptr, ++ref, ++arr[0], ++vla[0];
  //CHECK: "DIR.OMP.END.TARGET.DATA"()
  zoo();
  return a;
}

// end INTEL_COLLAB
