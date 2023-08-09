; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s
;
; Check that we can handle multiple reduction operands on the same taskloop construct
;
; Test src:
;
; int  x, y;
; void foo() {
; #pragma omp taskloop reduction(+: x, y)
;  for (int i = 0; i < 10; i++);
; }

; CHECK:  %x.red.struct = getelementptr inbounds %__struct.kmp_task_t_red_rec, ptr %taskt.red.rec, i32 0, i32 0
; CHECK:  %x.red.item = getelementptr inbounds %__struct.kmp_taskred_input_t, ptr %x.red.struct, i32 0, i32 0
; CHECK:  %x.red.orig = getelementptr inbounds %__struct.kmp_taskred_input_t, ptr %x.red.struct, i32 0, i32 1
; CHECK:  %x.red.size = getelementptr inbounds %__struct.kmp_taskred_input_t, ptr %x.red.struct, i32 0, i32 2
; CHECK:  %x.red.init = getelementptr inbounds %__struct.kmp_taskred_input_t, ptr %x.red.struct, i32 0, i32 3
; CHECK:  %x.red.fini = getelementptr inbounds %__struct.kmp_taskred_input_t, ptr %x.red.struct, i32 0, i32 4
; CHECK:  %x.red.comb = getelementptr inbounds %__struct.kmp_taskred_input_t, ptr %x.red.struct, i32 0, i32 5
; CHECK:  %x.red.flags = getelementptr inbounds %__struct.kmp_taskred_input_t, ptr %x.red.struct, i32 0, i32 6
; CHECK:  %y.red.struct = getelementptr inbounds %__struct.kmp_task_t_red_rec, ptr %taskt.red.rec, i32 0, i32 1
; CHECK:  %y.red.item = getelementptr inbounds %__struct.kmp_taskred_input_t, ptr %y.red.struct, i32 0, i32 0
; CHECK:  %y.red.orig = getelementptr inbounds %__struct.kmp_taskred_input_t, ptr %y.red.struct, i32 0, i32 1
; CHECK:  %y.red.size = getelementptr inbounds %__struct.kmp_taskred_input_t, ptr %y.red.struct, i32 0, i32 2
; CHECK:  %y.red.init = getelementptr inbounds %__struct.kmp_taskred_input_t, ptr %y.red.struct, i32 0, i32 3
; CHECK:  %y.red.fini = getelementptr inbounds %__struct.kmp_taskred_input_t, ptr %y.red.struct, i32 0, i32 4
; CHECK:  %y.red.comb = getelementptr inbounds %__struct.kmp_taskred_input_t, ptr %y.red.struct, i32 0, i32 5
; CHECK:  %y.red.flags = getelementptr inbounds %__struct.kmp_taskred_input_t, ptr %y.red.struct, i32 0, i32 6

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@x = dso_local global i32 0, align 4
@y = dso_local global i32 0, align 4

define dso_local void @foo() {
entry:
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKGROUP"(),
    "QUAL.OMP.IMPLICIT"(),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr @x, i32 0, i32 1),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr @y, i32 0, i32 1) ]

  store i32 0, ptr %.omp.lb, align 4
  store i32 9, ptr %.omp.ub, align 4
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKLOOP"(),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr @x, i32 0, i32 1),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr @y, i32 0, i32 1),
    "QUAL.OMP.INREDUCTION.ADD:TYPED"(ptr @x, i32 0, i32 1),
    "QUAL.OMP.INREDUCTION.ADD:TYPED"(ptr @y, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1) ]

  %2 = load i32, ptr %.omp.lb, align 4
  store i32 %2, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %3 = load i32, ptr %.omp.iv, align 4
  %4 = load i32, ptr %.omp.ub, align 4
  %cmp = icmp sle i32 %3, %4
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %5 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %5, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %i, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %6 = load i32, ptr %.omp.iv, align 4
  %add1 = add nsw i32 %6, 1
  store i32 %add1, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TASKLOOP"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TASKGROUP"() ]
  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)
