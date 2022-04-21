// INTEL_COLLAB
// RUN: %clang_cc1 -opaque-pointers -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s

// CHECK-LABEL: foo_close
void foo_close(int ii){
  // Map of a scalar.
  int a = ii;
  // CHECK: [[TV:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM:CLOSE"(ptr %a, ptr %a, i64 4, i64 1059
  // CHECK:  region.exit(token [[TV]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target map(close, tofrom: a)
  {
    a++;
  }
  // CHECK: [[TV1:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM:ALWAYS.CLOSE"(ptr %a, ptr %a, i64 4, i64 1063
  // CHECK:  region.exit(token [[TV1]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target map(always, close, tofrom: a)
  {
     a++;
  }
}

// CHECK-LABEL: foo_constexpr
void foo_constexpr()
{
  constexpr int N = 100;
  float v2[N];

// CHECK: [[TV:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-SAME: "QUAL.OMP.MAP.TO"(ptr %v2, ptr %v2, i64 400, i64 33
// CHECK-NOT: "QUAL.OMP.MAP.TOFROM"
// CHECK:  region.exit(token [[TV]]) [ "DIR.OMP.END.TARGET"() ]
#pragma omp target teams distribute parallel for map(to: v2)
  for (int i = 0; i < N; ++i) {
    v2[1] *= 2;
  }
}

// CHECK-LABEL: foo_close_one
void foo_close_one(int arg)
{
  float lb[arg];
  // CHECK: [[ARG:%.+]] = alloca i32
  // CHECK: [[L:%[0-9]+]] = load i32, ptr [[ARG]]
  // CHECK: [[L1:%[0-9]+]] = zext i32 [[L]] to i64
  // CHECK: [[VLA:%.+]] = alloca float, i64 [[L1]]
  // CHECK: [[L3:%[0-9]+]] =  mul nuw i64 [[L1]], 4
  // CHECK: [[ARR:%.+]] = getelementptr inbounds float, ptr [[VLA]], i64 0
  // CHECK: [[TV1:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET.ENTER.DATA
  // CHECK-SAME: "QUAL.OMP.MAP.TO:CLOSE"(ptr [[VLA]], ptr [[ARR]], i64 [[L3]], i64 1025
  // CHECK:  region.exit(token [[TV1]]) [ "DIR.OMP.END.TARGET.ENTER.DATA"() ]
  #pragma omp target enter data map(close, to: lb)
  {++arg;}
  // CHECK: [[TV2:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET.ENTER.DATA
  // CHECK-SAME: "QUAL.OMP.MAP.TO:ALWAYS.CLOSE"(ptr %vla, {{.*}} 1029
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
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr %this1, ptr %zoo, i64 [[L8]], i64 32
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM:CHAIN"(ptr %zoo, ptr %arrayidx, i64 136, i64 281474976710675
  // CHECK-SAME: "QUAL.OMP.MAP.FROM:CHAIN"(ptr %xoo, ptr %arrayidx4, i64 48, i64 281474976710674
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
// CHECK: [[T:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr %s, ptr %s, {{.*}}, i64 32
// CHECK-SAME: "QUAL.OMP.MAP.TO:CHAIN"(ptr %s, ptr %s, i64 {{56|28}}, i64 281474976710657
// CHECK-SAME: "QUAL.OMP.MAP.TO:CHAIN"(ptr %ptr1, ptr %arrayidx, i64 4, i64 281474976710673
// CHECK-SAME: "QUAL.OMP.MAP.TO:CHAIN"(ptr %ptrBase1, ptr %arrayidx3, i64 4, i64 281474976710673
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
  // CHECK: [[PT_TMP_MAP:%pt.map.ptr.tmp]] = alloca ptr, align 8
  // CHECK: [[L1:%[0-9]+]] = load ptr, ptr %pt{{.}}
  // CHECK: [[T:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: "QUAL.OMP.MAP.TO"(ptr %axx, ptr %axx, i64 8, i64 33
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr %this1, ptr %ptr{{.*}}, i64 8, i64 547
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr [[L1]],
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(ptr [[PT_TMP_MAP]]),
  // CHECK: store ptr [[L1]]
  // CHECK: [[L:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.PARALLEL.LOOP
  // CHECK-SAME: "QUAL.OMP.REDUCTION.ADD:ARRSECT"(ptr [[PT_TMP_MAP]],
#pragma omp target parallel for map(to:axx) reduction(+:pt[0:9])
    for (int i=0; i <20 ; i++) {
  // CHECK:load ptr, ptr %axx,
  // CHECK:load ptr, ptr [[PT_TMP_MAP]],
      axx[1] = 10;
      ptr[1] = 20;
      pt[1] = 20;
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

struct ThreadData { int teams; int threads;};

void ThreadsTeams(ThreadData* threads)
{
// CHECK: [[TEAMS:%teams]] = getelementptr inbounds %struct.ThreadData,
// CHECK: [[T:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.TARGET
// CHECK-NOT: "QUAL.OMP.MAP.TOFROM"
// CHECK: [[T1:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.TEAMS
// CHECK: [[THREADS:%threads1]] = getelementptr inbounds %struct.ThreadData
// CHECK: [[T2:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.DISTRIBUTE.PARLOOP
#pragma omp target teams distribute parallel for num_teams(threads->teams) num_threads(threads->threads)
   for (int i = 0; i < 100; ++i)
   {
   }
// CHECK: region.exit(token [[T2]]) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]
// CHECK: region.exit(token [[T1]]) [ "DIR.OMP.END.TEAMS"() ]
// CHECK: region.exit(token [[T]]) [ "DIR.OMP.END.TARGET"() ]
}

struct A {
  A() : pt(&x), ptr(pt) {}
  int *pt;
  int x = 0;
  int *&ptr;

  void foo()
  {
  // CHECK: [[PTR:%ptr]] = alloca ptr
  // CHECK: [[PT_TMP_MAP:%ptr.map.ptr.tmp]] = alloca ptr, align 8
  // CHECK: [[L1:%[0-9]+]] = load ptr, ptr %ptr3
  // CHECK: [[L3:%[0-9]+]] = load ptr, ptr [[L1]]
  // CHECK: [[T:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME "QUAL.OMP.PRIVATE"(ptr %ptr.map.ptr.tmp)
  // CHECK: store ptr [[L3]], ptr [[PT_TMP_MAP]]
  // CHECK: [[T1:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.PARALLEL.LOOP
  // CHECK: "QUAL.OMP.REDUCTION.ADD:ARRSECT"(ptr [[PT_TMP_MAP]]
    #pragma omp target parallel for reduction(+:ptr[:1])
    for (int i=0; i < 20 ; i++) {
      // CHECK: load ptr, ptr [[PT_TMP_MAP]]
      ptr[0] = 1;
    }
   // CHECK: region.exit(token [[T1]]) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
   // CHECK: region.exit(token [[T]]) [ "DIR.OMP.END.TARGET"() ]
  }
};

void bar()
{
   A a;
   a.foo();
}
// end INTEL_COLLAB
