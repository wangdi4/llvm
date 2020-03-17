// INTEL_COLLAB
// RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s

// CHECK-LABEL: foo_close
void foo_close(int ii){
  // Map of a scalar.
  int a = ii;
  // CHECK: [[TV:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM:CLOSE"(i32* %a, i32* %a, i64 4, i64 1059)
  // CHECK:  region.exit(token [[TV]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target map(close, tofrom: a)
  {
    a++;
  }
  // CHECK: [[TV1:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM:ALWAYS.CLOSE"(i32* %a, i32* %a, i64 4, i64 1063)
  // CHECK:  region.exit(token [[TV1]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target map(always, close, tofrom: a)
  {
     a++;
  }
}

// CHECK-LABEL: foo_close_one
void foo_close_one(int arg)
{
  float lb[arg];
  // CHECK: [[ARG:%.+]] = alloca i32
  // CHECK: [[L:%[0-9]+]] = load i32, i32* [[ARG]]
  // CHECK: [[L1:%[0-9]+]] = zext i32 [[L]] to i64
  // CHECK: [[VLA:%.+]] = alloca float, i64 [[L1]]
  // CHECK: [[L3:%[0-9]+]] =  mul nuw i64 [[L1]], 4
  // CHECK: [[ARR:%.+]] = getelementptr inbounds float, float* [[VLA]], i64 0
  // CHECK: [[TV1:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET.ENTER.DATA
  // CHECK-SAME: "QUAL.OMP.MAP.TO:CLOSE"(float* [[VLA]], float* [[ARR]], i64 [[L3]], i64 1057)
  // CHECK:  region.exit(token [[TV1]]) [ "DIR.OMP.END.TARGET.ENTER.DATA"() ]
  #pragma omp target enter data map(close, to: lb)
  {++arg;}
  // CHECK: [[TV2:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET.ENTER.DATA
  // CHECK-SAME: "QUAL.OMP.MAP.TO:ALWAYS.CLOSE"(float* %vla, {{.*}} 1061)
  // CHECK:  region.exit(token [[TV2]]) [ "DIR.OMP.END.TARGET.ENTER.DATA"() ]
  #pragma omp target enter data map(always close, to: lb)
  {++arg;}
}

class B {
public:
  B();
  void start();
  double* zoo;
  double* xoo;
};

void B::start()
{
  // CHECK: [[L8:%[0-9]+]] = sdiv exact i64
  // CHECK: [[T:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(%class.B* %this1, double** %zoo, i64 [[L8]], i64 32)
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM:CHAIN"(double** %zoo, double* %arrayidx, i64 136, i64 281474976710675)
  // CHECK-SAME: "QUAL.OMP.MAP.FROM:CHAIN"(double** %xoo, double* %arrayidx4, i64 48, i64 281474976710674)
  #pragma omp target map(tofrom: zoo[7:17]) map(from: xoo[1:6])
  zoo[2] = 7,xoo[2] = 8;
 xoo[2] = 8;
}

typedef struct {
  int *ptrBase;
  int valBase;
  int *ptrBase1;
} Base;

typedef struct : public Base {
  int *ptr;
  int *ptr2;
  int val;
  int *ptr1;
} StructWithPtr;

void map_with_overlap_elems() {
  StructWithPtr s;
// CHECK: [[L8:%[0-9]+]] = sdiv exact i64
// CHECK: [[L9:%[0-9]+]] = getelementptr i32*, i32** %ptrBase1, i64 1
// CHECK: [[L15:%[0-9]+]] = sdiv exact i64
// CHECK: [[L16:%[0-9]+]] = getelementptr i32*, i32** %ptr1, i64 1
// CHECK: [[L22:%[0-9]+]] = sdiv exact i64
// CHECK: [[T:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(%struct.StructWithPtr* %s, %struct.StructWithPtr* %s, {{.*}}, i64 32)
// CHECK-SAME: "QUAL.OMP.MAP.TO:CHAIN"(%struct.StructWithPtr* %s, %struct.StructWithPtr* %s, i64 [[L8]], i64 281474976710657)
// CHECK-SAME: "QUAL.OMP.MAP.TO:CHAIN"(%struct.StructWithPtr* %s, i32** [[L9]], i64 [[L15]], i64 281474976710657)
// CHECK-SAME: "QUAL.OMP.MAP.TO:CHAIN"(%struct.StructWithPtr* %s, i32** [[L16]], i64 [[L22]], i64 281474976710657)
// CHECK-SAME: "QUAL.OMP.MAP.TO:CHAIN"(i32** %ptr11, i32* %arrayidx, i64 4, i64 281474976710673)
// CHECK-SAME: "QUAL.OMP.MAP.TO:CHAIN"(i32** %ptrBase13, i32* %arrayidx5, i64 4, i64 281474976710673)
#pragma omp target map(to:s, s.ptr1 [0:1], s.ptrBase1 [0:1])
  {
    s.val++;
    s.ptr1[0]++;
    s.ptrBase1[0] = 10001;
  }
  // CHECK: region.exit(token [[T]]) [ "DIR.OMP.END.TARGET"() ]
}

template<typename T>
class Mapper {
private:
  T* ptr;
  int * pt;
public:
  Mapper (T* p) : ptr(p) {
    int *axx;
  // CHECK: [[L7:%[0-9]+]] = sdiv
  // CHECK: [[T:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: "QUAL.OMP.MAP.TO"(i32** %axx, i32** %axx, i64 8, i64 33)
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(%class.Mapper* %this1, %class.AOO** %ptr4, i64 [[L7]], i64 32)
  // CHECK: [[L:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.PARALLEL.LOOP
  // CHECK-SAME: "QUAL.OMP.REDUCTION.ADD:ARRSECT"(i32** %pt3
#pragma omp target parallel for map(to:axx) reduction(+:pt[0:9])
    for (int i=0; i <20 ; i++) {
  // CHECK:load i32*, i32** %axx,
      axx[1] = 10;
      ptr[1] = 20;
    }
  }
};
class AOO : public Mapper<AOO> {
public:
  AOO(int s) : Mapper<AOO>(this) { }
  AOO operator +(AOO);
  AOO();
};

int test_complex_class() {
  AOO *obj = new AOO(1000);
  return 0;
}
// end INTEL_COLLAB
