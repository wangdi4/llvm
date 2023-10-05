// INTEL_COLLAB
// RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s

class A
{
public:
  A();
  ~A();
  int a;
};

// CHECK-LABEL: foo
void foo(A & f)
{
// CHECK: [[F_ADDR:%f.addr]] = alloca ptr,
// CHECK: [[MAP_ADDR:%f.map.ptr.tmp]] = alloca ptr,
// CHECK: [[L:%[0-9]+]] = load ptr, ptr [[F_ADDR]]
// CHECK: [[TV1:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr [[L]]
// CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[MAP_ADDR]]
// CHECK: region.exit(token [[TV1]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target firstprivate(f)
  for (int i = 0 ; i < 10 ; i ++)
    f.a = 10;
}

// CHECK-LABEL: zoo
void zoo(int &one)
{
  int a = 0;
// CHECK: [[ONE_ADDR:%one.addr]] = alloca ptr
// CHECK: [[A_ADDR:%a]] = alloca i32
// CHECK: [[MAP_ADDR:%one.map.ptr.tmp]] = alloca ptr
// CHECK: [[L:%[0-9]+]] = load ptr, ptr [[ONE_ADDR]]
// CHECK: [[TV1:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr [[A_ADDR]]
// CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr [[L]]
// CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[MAP_ADDR]]
// CHECK: region.exit(token [[TV1]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target map(a) firstprivate(one)
  a += one;
}
