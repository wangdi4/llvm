// INTEL_COLLAB
// RUN: %clang_cc1 -opaque-pointers -emit-llvm -o - -fopenmp -fopenmp-late-outline -fopenmp-typed-clauses \
// RUN:  -triple x86_64-unknown-linux-gnu %s \
// RUN:  | FileCheck --check-prefixes CHECK,CHECK-NEW %s

// RUN: %clang_cc1 -opaque-pointers -emit-llvm -o - -fopenmp -fopenmp-late-outline -fopenmp-typed-clauses \
// RUN:  -fno-openmp-new-depend-ir -triple x86_64-unknown-linux-gnu %s \
// RUN:  | FileCheck --check-prefixes CHECK,CHECK-OLD %s

void foo1() {
  double w = 1.0;
  long x = 2;
  short y = 3;
  // CHECK: [[W:%.+]] = alloca double,
  // CHECK: [[X:%.+]] = alloca i64,
  // CHECK: [[Y:%.+]] = alloca i16,
  // CHECK-NEW: [[DARR:%.*]] = getelementptr inbounds [1 x %struct.kmp_depend_info], ptr %.dep.arr.addr, i64 0, i64 0
  // CHECK: DIR.OMP.TASK
  // CHECK-SAME: "QUAL.OMP.IF"(i32 0)
  // CHECK-SAME: "QUAL.OMP.TARGET.TASK"
  // CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[W]]
  // CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr [[X]]
  // CHECK-OLD-SAME: "QUAL.OMP.DEPEND.OUT"(ptr [[Y]])
  // CHECK-NEW-SAME: "QUAL.OMP.DEPARRAY"(i32 1, ptr [[DARR]])
  // CHECK: DIR.OMP.TARGET
  // CHECK-SAME: "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0)
  // CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[W]]
  // CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr [[X]]
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr [[Y]], ptr [[Y]], i64 2, i64 35
  // CHECK: DIR.OMP.END.TARGET
  // CHECK: DIR.OMP.END.TASK
  #pragma omp target private(w) firstprivate(x) map(tofrom:y) depend(out:y)
    y = 3;
}

void foo2() {
  int y = 2;
  int *yp = &y;
  volatile int size = 1;
  // CHECK: [[Y:%.+]] = alloca i32,
  // CHECK: [[YP:%.+]] = alloca ptr,
  // CHECK: [[SZ:%.+]] = alloca i32,
  // CHECK: [[YPMAP:%.+]] = alloca ptr, align 8
  // CHECK-NEW: [[DARR:%.*]] = getelementptr inbounds [1 x %struct.kmp_depend_info], ptr %.dep.arr.addr, i64 0, i64 0
  // CHECK: DIR.OMP.TASK
  // CHECK-SAME: "QUAL.OMP.TARGET.TASK"
  // CHECK-OLD-SAME: "QUAL.OMP.DEPEND.OUT"(ptr [[Y]])
  // CHECK-NEW-SAME: "QUAL.OMP.DEPARRAY"(i32 1, ptr [[DARR]])
  // CHECK-DAG: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr [[YP]]
  // CHECK-DAG: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr [[Y]]
  // CHECK-DAG: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr [[SZ]]
  // CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[YPMAP]]
  // CHECK: [[L0:%.+]] = load ptr, ptr [[YP]]
  // CHECK: [[L1:%.+]] = load ptr, ptr [[YP]]
  // CHECK: [[L2:%.+]] = load ptr, ptr [[YP]]
  // CHECK-NEXT: [[AI:%.+]] = getelementptr{{.*}}
  // CHECK: DIR.OMP.TARGET
  // CHECK-SAME: "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 1)
  // CHECK-SAME: "QUAL.OMP.NOWAIT"
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr [[L1]], ptr [[AI]], i64 %{{.*}}, i64 35
  // CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[YPMAP]]
  // CHECK: store ptr [[L1]], ptr [[YPMAP]], align 8
  // CHECK: DIR.OMP.END.TARGET
  // CHECK: DIR.OMP.END.TASK
  #pragma omp target  map(tofrom:yp[0:size])  nowait depend(out:y)
   {
      yp[1] = 0;
      y = 3;
   }
}

// CHECK: define {{.*}}_ZN1AC2Ev
// CHECK: DIR.OMP.TASK
// CHECK-SAME: "QUAL.OMP.TARGET.TASK"
// CHECK: DIR.OMP.TARGET
// CHECK-SAME: "QUAL.OMP.DEVICE"
// CHECK: DIR.OMP.END.TARGET
// CHECK: DIR.OMP.END.TASK

struct A {
  A() {
    char b;
    #pragma omp target device(b) nowait
    ;
  }
};

#pragma omp declare target
void foo3()
{
  A avar;
}
#pragma omp end declare target
void foo3(float *vx_in, float *vx_out) {
  // CHECK: [[IN:%.+]] = alloca ptr,
  // CHECK: [[OUT:%.+]] = alloca ptr,
  // CHECK: DIR.OMP.TASK
  // CHECK: "QUAL.OMP.TARGET.TASK"
  // CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr [[IN]]
  // CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr [[OUT]]
  // CHECK: DIR.OMP.TARGET
  // CHECK: DIR.OMP.END.TARGET
  // CHECK: DIR.OMP.END.TASK
  #pragma omp target nowait depend (in: vx_in) depend (out: vx_out)
  for (int i = 0; i < 10; ++i)
     vx_out[i] = vx_in[i] + 10;
}

float *xp, *yp, **za[10];
void bar(float*, float*, float*);
struct Str { float x[10]; };
Str s, *sp;
void foo4() {
  // CHECK: [[L3:%.+]] = load ptr, ptr @yp
  // CHECK: [[L4:%.+]] = load ptr, ptr @sp,
  // CHECK: [[L5:%.+]] = load ptr, ptr @sp,
  // CHECK: DIR.OMP.TASK
  // CHECK-SAME: "QUAL.OMP.TARGET.TASK"
  // CHECK-SAME: "QUAL.OMP.SHARED:TYPED"(ptr @xp
  // CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr @yp
  // CHECK-SAME: "QUAL.OMP.SHARED:TYPED"(ptr @za
  // CHECK-SAME: "QUAL.OMP.SHARED:TYPED"(ptr @s
  // CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr @sp
  // CHECK: DIR.OMP.END.TASK
  #pragma omp target nowait map(tofrom:xp) map(to:za[0:1]) map(to:s.x[0:]) map(to: sp->x[0:1])
  bar(xp, yp, za[0][0]);

  // CHECK: DIR.OMP.TASK
  // CHECK-SAME: "QUAL.OMP.TARGET.TASK"
  // CHECK-SAME: QUAL.OMP.INREDUCTION.ADD:ARRSECT.PTR_TO_PTR.TYPED"(ptr @xp
  // CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr @yp
  // CHECK-SAME: "QUAL.OMP.SHARED:TYPED"(ptr @za
  // CHECK: DIR.OMP.END.TASK
  #pragma omp target nowait map(tofrom:xp) in_reduction(+ : xp[0:1])
  bar(xp, yp, za[0][0]);

  // CHECK: DIR.OMP.TASK
  // CHECK-SAME: "QUAL.OMP.TARGET.TASK"
  // CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr @xp
  // CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr @yp
  // CHECK-SAME: "QUAL.OMP.SHARED:TYPED"(ptr @za
  #pragma omp target nowait map(tofrom:xp[0:1])
  bar(xp, yp, za[0][0]);
}


void DoTask(int a, int b);
void foo5(int x, int y)
{
// CHECK: DIR.OMP.PARALLEL
// CHECK-SAME "QUAL.OMP.PRIVATE:TYPED"(ptr %a
// CHECK-SAME "QUAL.OMP.PRIVATE:TYPED"(ptr %aa
#pragma omp parallel
  {
   int &a = x;
   int &aa = y;
// CHECK: DIR.OMP.TASK
// CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:BYREF.TYPED"(ptr %a
// CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:BYREF.TYPED"(ptr %aa
#pragma omp task
   DoTask(a, aa);
  }
// CHECK: DIR.OMP.END.TASK
// CHECK: DIR.OMP.END.PARALLEL
}
// end INTEL_COLLAB
