; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:
;
; #include <stdio.h>
; int y = 1;
;
; void foo(int j, int k) {
; //#pragma omp parallel for lastprivate(conditional: y) num_threads(4) schedule(dynamic)
; #pragma omp for lastprivate(conditional: y) schedule(dynamic)
;   for (int i = 0; i <10; i++) {
;     if (i == j || i == k)
;       y = i;
;   }
; }
;
; /*
; int main() {
;   foo(2, 4);
;   printf("y = %d\n", y);
; }*/

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@y = dso_local global i32 1, align 4

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo(i32 noundef %j, i32 noundef %k) #0 {
entry:
  %j.addr = alloca i32, align 4
  %k.addr = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 %j, ptr %j.addr, align 4
  store i32 %k, ptr %k.addr, align 4
  store i32 0, ptr %.omp.lb, align 4
  store i32 9, ptr %.omp.ub, align 4

; Check for initialization of local variables before the loop
; CHECK:  %y.local.max.idx = alloca i32
; CHECK:  store i32 0, ptr %y.local.max.idx
; CHECK:  %y.modified.by.thread = alloca i1
; CHECK:  store i1 false, ptr %y.modified.by.thread
; CHECK:  %y.modified.by.chunk = alloca i1
; CHECK:  store i1 false, ptr %y.modified.by.chunk
; CHECK:  %y.lpriv = alloca i32
; CHECK:  %y.lpriv.local.last = alloca i32

  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(),
    "QUAL.OMP.LASTPRIVATE:CONDITIONAL.TYPED"(ptr @y, i32 0, i32 1),
    "QUAL.OMP.SCHEDULE.DYNAMIC"(i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1) ]
  %1 = load i32, ptr %.omp.lb, align 4
  store i32 %1, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

; Check for the initialization of %y.modified.by.chunk in the beginning of each chunk
; CHECK:  call void @__kmpc_dispatch_init_4
; CHECK:  %{{[^ ]+}} = call i32 @__kmpc_dispatch_next_4
; CHECK:  store i1 false, ptr %y.modified.by.chunk
; CHECK:  %lb.new = load i32, ptr %lower.bnd

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %2 = load i32, ptr %.omp.iv, align 4
  %3 = load i32, ptr %.omp.ub, align 4
  %cmp = icmp sle i32 %2, %3
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %4 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %4, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %i, align 4
  %5 = load i32, ptr %i, align 4
  %6 = load i32, ptr %j.addr, align 4
  %cmp1 = icmp eq i32 %5, %6
  br i1 %cmp1, label %if.then, label %lor.lhs.false

lor.lhs.false:                                    ; preds = %omp.inner.for.body
  %7 = load i32, ptr %i, align 4
  %8 = load i32, ptr %k.addr, align 4
  %cmp2 = icmp eq i32 %7, %8
  br i1 %cmp2, label %if.then, label %if.end

if.then:                                          ; preds = %lor.lhs.false, %omp.inner.for.body
  %9 = load i32, ptr %i, align 4
; Check that the %y.modified.by.chunk flag is set to true when y is written.
; CHECK: store i1 true, ptr %y.modified.by.chunk
  store i32 %9, ptr @y, align 4
  br label %if.end

if.end:                                           ; preds = %if.then, %lor.lhs.false
  br label %omp.body.continue

omp.body.continue:                                ; preds = %if.end
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %10 = load i32, ptr %.omp.iv, align 4
  %add3 = add nsw i32 %10, 1
  store i32 %add3, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

; Check for code emitted at the end of each chunk
; CHECK:  [[LOAD1:[^ ]+]] = load i1, ptr %y.modified.by.chunk
; CHECK:  %y.modified = icmp eq i1 [[LOAD1]], true
; CHECK:  [[LOAD2:[^ ]+]] = load i32, ptr %y.local.max.idx
; CHECK:  %y.chunk.is.higher = icmp uge i32 %lb.new, [[LOAD2]]
; CHECK:  %y.modified.and.chunk.is.higher = and i1 %y.modified, %y.chunk.is.higher
; CHECK:  br i1 %y.modified.and.chunk.is.higher, label %[[LABEL1:[^ ,]+]], label %{{[^ ]+}}
;
; CHECK:[[LABEL1]]:
; CHECK:  store i1 true, ptr %y.modified.by.thread
; CHECK:  store i32 %lb.new, ptr %y.local.max.idx
; CHECK:  [[LOAD3:[^ ]+]] = load i32, ptr %y.lpriv
; CHECK:  store i32 [[LOAD3]], ptr %y.lpriv.local.last

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.LOOP"() ]
  ret void

; Check for the the computation of global max index to get which thread
; wrote the last chunk

; CHECK:  [[LOAD4:[^ ]+]] = load i1, ptr %y.modified.by.thread
; CHECK:  %y.written.by.thread = icmp eq i1 [[LOAD4]], true
; CHECK:  br i1 %y.written.by.thread, label %[[LABEL2:[^ ,]+]], label %[[LABEL3:[^ ,]+]]
;
; CHECK:[[LABEL2]]:
; CHECK:  [[LOAD5:[^ ]+]] = load i32, ptr %y.local.max.idx
; CHECK:  call void @__kmpc_critical
; CHECK:  [[LOAD6:[^ ]+]] = load i32, ptr @y.global.max.idx
; CHECK:  %y.is.local.idx.higher = icmp uge i32 [[LOAD5]], [[LOAD6]]
; CHECK:  br i1 %y.is.local.idx.higher, label %[[LABEL4:[^ ,]+]], label %[[LABEL5:[^ ,]+]]
;
; CHECK:[[LABEL4]]:
; CHECK:  store i32 [[LOAD5]], ptr @y.global.max.idx
; CHECK:  br label %[[LABEL5]]
;
; CHECK:[[LABEL5]]:
; CHECK:  call void @__kmpc_end_critical
; CHECK:  br label %[[LABEL3]]
;
; CHECK:[[LABEL3]]:
; CHECK:  call void @__kmpc_barrier

; Check for the final copyout code
; CHECK:  [[LOAD7:[^ ]+]] = load i32, ptr %y.local.max.idx
; CHECK:  [[LOAD8:[^ ]+]] = load i32, ptr @y.global.max.idx
; CHECK:  %y.copyout.or.not = icmp eq i32 [[LOAD7]], [[LOAD8]]
; CHECK:  br i1 %y.copyout.or.not, label %[[LABEL6:[^ ,]+]], label %{{.*}}
;
; CHECK:[[LABEL6]]:
; CHECK:  [[LOAD9:[^ ]+]] = load i32, ptr %y.lpriv.local.last
; CHECK:  store i32 [[LOAD9]], ptr %y.lpriv
; CHECK:  [[LOAD10:[^ ]+]]  = load i32, ptr %y.lpriv
; CHECK:  store i32 [[LOAD10]], ptr @y

}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}
