// RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s

class B {
public:
  B();
  void start();
  double* zoo;
  double* xoo;
};

void B::start()
{
  // CHECK: [[T:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM:AGGRHEAD"(double** %zoo, double** %zoo, i64 8)
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM:AGGR"(double** %zoo, double* %arrayidx, i64 136)
  // CHECK-SAME: "QUAL.OMP.MAP.FROM:AGGRHEAD"(double** %xoo, double** %xoo, i64 8)
  // CHECK-SAME: "QUAL.OMP.MAP.FROM:AGGR"(double** %xoo, double* %arrayidx2, i64 48)
  #pragma omp target map(tofrom: zoo[7:17]) map(from: xoo[1:6])
  zoo[2] = 7,xoo[2] = 8;
 // CHECK: load double*, double** %zoo, align 8
 // CHECK: load double*, double** %xoo, align 8
 xoo[2] = 8;
 // CHECK-NOT: load double*, double** %xoo,
}
template<typename T>
class Mapper {
private:
  T* ptr;
  int * pt;
public:
  Mapper (T* p) : ptr(p) {
    int *axx;
  // CHECK: [[T:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: "QUAL.OMP.MAP.TO"(i32** %axx)
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(%class.AOO** %ptr4)
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

