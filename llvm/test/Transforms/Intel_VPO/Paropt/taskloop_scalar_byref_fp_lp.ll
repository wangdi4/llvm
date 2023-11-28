; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s
;
; Test src:
;
; #include <stdio.h>
;
; void foo(int &a) {
; #pragma omp taskloop lastprivate(a) firstprivate(a)
;   for (int i = 0; i < 10; i++) {
;     a += i;
;   }
; }
;
; //int main() {
; //  int a;
; //  a = 1;
; //  foo(a);
; //  printf("%d\n", a);
; //  return 0;
; //}

; Check for the space allocated for the private copy. { i32, i64, i64, i32 }
; i32 - %a.addr
; i64 - %.omp.lb
; i64 - %.omp.ub
; i32 - %i
; CHECK: %__struct.kmp_privates.t = type { i32, i64, i64, i32 }

; Check shared thunk for space allocated for pointer to the original %a.addr for lastprivate copyout.
; i32** - %a.addr
; CHECK: %__struct.shared.t = type { ptr }

; For the outlined function for the task, check that the address of the local
; copy for '%a.addr' is stored to an ptr, and then that is used instead of
; '%a.addr' in the region.
; CHECK: define internal void @{{.*}}DIR.OMP.TASK{{.*}}
; CHECK: [[A_PRIVATE:%[^ ]+]] = getelementptr inbounds %__struct.kmp_privates.t, ptr {{[^ ]+}}, i32 0, i32 0
; CHECK: [[A_SHR_ADDR:%[^ ]+]] = getelementptr inbounds %__struct.shared.t, ptr {{[^ ]+}}, i32 0, i32 0
; CHECK: [[A_SHR:%[^ ]+]] = load ptr, ptr [[A_SHR_ADDR]]
; CHECK: store ptr [[A_PRIVATE]], ptr {{[^ ]+}}

; Check for the lastprivate copyout code
; CHECK: [[A_SHR_LOAD:%[^ ]+]] = load ptr, ptr [[A_SHR]]
; CHECK: [[A_PRIVATE_LOAD:%[^ ]+]] = load i32, ptr [[A_PRIVATE]]
; CHECK: store i32 [[A_PRIVATE_LOAD]], ptr [[A_SHR_LOAD]]

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @_Z3fooRi(ptr dereferenceable(4) %a) {
entry:
  %a.addr = alloca ptr, align 8
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i64, align 8
  %.omp.ub = alloca i64, align 8
  %i = alloca i32, align 4
  store ptr %a, ptr %a.addr, align 8
  store i64 0, ptr %.omp.lb, align 8
  store i64 9, ptr %.omp.ub, align 8
  %0 = load ptr, ptr %a.addr, align 8
  %1 = load ptr, ptr %a.addr, align 8
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKLOOP"(),
    "QUAL.OMP.LASTPRIVATE:BYREF.TYPED"(ptr %a.addr, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:BYREF.TYPED"(ptr %a.addr, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i64 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i64 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1) ]

  %3 = load i64, ptr %.omp.lb, align 8
  %conv = trunc i64 %3 to i32
  store i32 %conv, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %4 = load i32, ptr %.omp.iv, align 4
  %conv1 = sext i32 %4 to i64
  %5 = load i64, ptr %.omp.ub, align 8
  %cmp = icmp ule i64 %conv1, %5
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %6 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %6, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %i, align 4
  %7 = load i32, ptr %i, align 4
  %8 = load ptr, ptr %a.addr, align 8
  %9 = load i32, ptr %8, align 4
  %add2 = add nsw i32 %9, %7
  store i32 %add2, ptr %8, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %10 = load i32, ptr %.omp.iv, align 4
  %add3 = add nsw i32 %10, 1
  store i32 %add3, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TASKLOOP"() ]

  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
