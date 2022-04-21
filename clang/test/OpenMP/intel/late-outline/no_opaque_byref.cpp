// INTEL_COLLAB
// RUN: %clang_cc1 -no-opaque-pointers -emit-llvm -o - -fopenmp -fintel-compatibility \
// RUN:  -fopenmp-late-outline -triple x86_64-unknown-linux-gnu %s \
// RUN:  | FileCheck %s
// RUN: %clang_cc1 -no-opaque-pointers -emit-llvm -o - -fexceptions -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s

#define LOOP for(int i=0;i<16;++i) {}

int foo(int);

// CHECK-LABEL: bar1
void bar1(int &d) {
  // CHECK: [[DADDR:%d.*]] = alloca i32*, align
  // CHECK: [[T0:%[0-9]+]] = call token @llvm.directive.region.entry()
  // CHECK-SAME: "DIR.OMP.PARALLEL.LOOP"()
  // CHECK-SAME: "QUAL.OMP.PRIVATE:BYREF"(i32** [[DADDR]])
  // CHECK: directive.region.exit(token [[T0]]) [ "DIR.OMP.END.PARALLEL.LOOP"
  #pragma omp parallel for private(d)
  for(int i=0;i<16;++i) {
    foo(d);
  }
}

// CHECK-LABEL: bar2
void bar2(int &d) {
  // CHECK: [[DADDR:%d.*]] = alloca i32*, align
  // CHECK: [[T0:%[0-9]+]] = call token @llvm.directive.region.entry()
  // CHECK-SAME: "DIR.OMP.PARALLEL.LOOP"()
  // CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:BYREF"(i32** [[DADDR]])
  // CHECK: directive.region.exit(token [[T0]]) [ "DIR.OMP.END.PARALLEL.LOOP"
  #pragma omp parallel for firstprivate(d)
  for(int i=0;i<16;++i) {
    foo(d);
  }
}

// CHECK-LABEL: bar3
void bar3(int &d) {
  // CHECK: [[DADDR:%d.*]] = alloca i32*, align
  // CHECK: [[T0:%[0-9]+]] = call token @llvm.directive.region.entry()
  // CHECK-SAME: "DIR.OMP.PARALLEL.LOOP"()
  // CHECK-SAME: "QUAL.OMP.LASTPRIVATE:BYREF"(i32** [[DADDR]])
  // CHECK: directive.region.exit(token [[T0]]) [ "DIR.OMP.END.PARALLEL.LOOP"
  #pragma omp parallel for lastprivate(d)
  for(int i=0;i<16;++i) {
    foo(d);
  }
}

// CHECK-LABEL: bar4
void bar4(int &d) {
  // CHECK: [[DADDR:%d.*]] = alloca i32*, align
  // CHECK: [[T0:%[0-9]+]] = call token @llvm.directive.region.entry()
  // CHECK-SAME: "DIR.OMP.PARALLEL.LOOP"()
  // CHECK-SAME: "QUAL.OMP.REDUCTION.ADD:BYREF"(i32** [[DADDR]])
  // CHECK: directive.region.exit(token [[T0]]) [ "DIR.OMP.END.PARALLEL.LOOP"
  #pragma omp parallel for reduction(+:d)
  for(int i=0;i<16;++i) {
    foo(d);
  }
}

void bar5(int &yref, int other, short *&zptr_ref, float (*&y_arrptr_ref)[10]) {
  // CHECK: [[YREFDADDR:%yref.addr.*]] = alloca i32*,
  // CHECK: [[OTHERDADDR:%other.addr.*]] = alloca i32,
  // CHECK: [[ZPTR_REFDADDR:%zptr_ref.addr.*]] = alloca i16**,
  // CHECK: [[Y_ARRPTR_REFDADDR:%y_arrptr_ref.addr.*]] = alloca [10 x float]**,
  // CHECK: [[YREF_ADDR:%yref_addr.*]] = alloca i32*,
  // CHECK: [[ZPTR_REF_ADDR:%zptr_ref_addr.*]] = alloca i16**,
  // CHECK: [[Y_ARRPTR_REF_ADDR:%y_arrptr_ref_addr.*]] = alloca [10 x float]**,

  int *yref_addr = &yref;
  short **zptr_ref_addr = &zptr_ref;
  float(**y_arrptr_ref_addr)[10] = &y_arrptr_ref;

  // CHECK: [[T0:%[0-9]+]] = call token @llvm.directive.region.entry()
  // CHECK-SAME: "DIR.OMP.PARALLEL.LOOP"()
  // CHECK-SAME: "QUAL.OMP.LINEAR:BYREF"(i32** [[YREFDADDR]], i32 1)
  // CHECK-SAME: "QUAL.OMP.LINEAR"(i32* [[OTHERDADDR]], i32 1)
  // CHECK-SAME: "QUAL.OMP.LINEAR:BYREF"(i16*** [[ZPTR_REFDADDR]], i32 1)
  // CHECK-SAME: "QUAL.OMP.LINEAR:BYREF"([10 x float]*** [[Y_ARRPTR_REFDADDR]],
  // CHECK-SAME: "QUAL.OMP.SHARED"{{.*}}[[YREF_ADDR]]
  // CHECK-SAME: "QUAL.OMP.SHARED"{{.*}}[[ZPTR_REF_ADDR]]
  // CHECK-SAME: "QUAL.OMP.SHARED"{{.*}}[[Y_ARRPTR_REF_ADDR]]
  #pragma omp parallel for linear(yref, other, zptr_ref, y_arrptr_ref)
  for (int i = 0; i < 5; i++) {
    foo(yref_addr != &yref);
    foo(zptr_ref_addr != &zptr_ref);
    foo(y_arrptr_ref_addr != &y_arrptr_ref);
  }
}

struct Iterator {
  int* It;
  Iterator(int *i) : It(i) {}
  Iterator(const Iterator& i) : It(i.It) {}
  int& operator*() const { return *It; }
  Iterator& operator++() { ++It; return *this; }
  Iterator operator+(long n) const noexcept { return Iterator(It + n); }
};

inline bool operator<(const Iterator &lhs, const Iterator &rhs) noexcept
{ return lhs.It < rhs.It; }

inline long operator-(const Iterator& lhs, const Iterator& rhs) noexcept
{ return lhs.It - rhs.It; }

struct A {
  int arr[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  Iterator begin() { return Iterator(&arr[0]); }
  Iterator end() { return Iterator(&arr[10]); }
};

//CHECK-LABEL: bar6
int bar6()
{
  A avar;
  int sum = 0;

  //CHECK:[[L0:%[0-9]+]] = load {{.*}}Iterator{{.*}}capture_expr
  //CHECK-NEXT: call {{.*}}_ZmiRK8IteratorS1_{{.*}}[[L0]]
  //CHECK: [[T0:%[0-9]+]] = call token @llvm.directive.region.entry()
  //CHECK-SAME: "DIR.OMP.PARALLEL.LOOP"()
  #pragma omp parallel for reduction(+:sum)
  for (auto i = avar.begin(); i < avar.end(); ++i)
    sum += *i + 2;
  //CHECK: directive.region.exit(token [[T0]]) [ "DIR.OMP.END.PARALLEL.LOOP"
  return sum;
}
// end INTEL_COLLAB
