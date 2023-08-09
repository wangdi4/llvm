; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:
;
; #include <stdio.h>
;
; void foo(int &y, int j, int k) {
; #pragma omp parallel for lastprivate(conditional: y) num_threads(10) schedule(static,1)
;   for (int i = 0; i < 10; i++) {
;   if (i == j)
;     y = i;
;   else if (i == k)
;     y = k;
;   }
; }
;
; /*
; int main() {
;   int y = 1;
;   foo(y, 2, 4);
;   printf("y = %d\n", y);
; }*/

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local void @_Z3fooRiii(ptr noundef nonnull align 4 dereferenceable(4) %y, i32 noundef %j, i32 noundef %k) #0 {
entry:
  %y.addr = alloca ptr, align 8
  %j.addr = alloca i32, align 4
  %k.addr = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  store ptr %y, ptr %y.addr, align 8
  store i32 %j, ptr %j.addr, align 4
  store i32 %k, ptr %k.addr, align 4
  store i32 0, ptr %.omp.lb, align 4
  store i32 9, ptr %.omp.ub, align 4
  %0 = load ptr, ptr %y.addr, align 8

; Check for initialization of local variables before the loop
; CHECK:  %y.addr.local.max.idx = alloca i32
; CHECK:  %y.addr.modified.by.thread = alloca i1
; CHECK:  %y.addr.modified.by.chunk = alloca i1
; CHECK:  %y.addr.lpriv = alloca i32
; CHECK:  %y.addr.lpriv.local.last = alloca i32
; CHECK:  %y.addr.lpriv.ref = alloca ptr
; CHECK:  store i32 0, ptr %y.addr.local.max.idx
; CHECK:  store i1 false, ptr %y.addr.modified.by.thread
; CHECK:  store i1 false, ptr %y.addr.modified.by.chunk
; CHECK:  store ptr %y.addr.lpriv, ptr %y.addr.lpriv.ref

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.LASTPRIVATE:BYREF.CONDITIONAL.TYPED"(ptr %y.addr, i32 0, i32 1),
    "QUAL.OMP.NUM_THREADS"(i32 10),
    "QUAL.OMP.SCHEDULE.STATIC"(i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %j.addr, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %k.addr, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1) ]
  %2 = load i32, ptr %.omp.lb, align 4
  store i32 %2, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

; Check for the initialization of %y.modified.by.chunk in the beginning of each chunk
; CHECK-DAG:  call void @__kmpc_for_static_init_4
; CHECK-DAG:  store i1 false, ptr %y.addr.modified.by.chunk
; CHECK-DAG:  %lb.new = load i32, ptr %lower.bnd

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
  %6 = load i32, ptr %i, align 4
  %7 = load i32, ptr %j.addr, align 4
  %cmp1 = icmp eq i32 %6, %7
  br i1 %cmp1, label %if.then, label %if.else

if.then:                                          ; preds = %omp.inner.for.body
  %8 = load i32, ptr %i, align 4
  %9 = load ptr, ptr %y.addr, align 8
; Check that the %y.add..modified.by.chunk flag is set to true when there is a
; store to a load of y.addr.
; CHECK-DAG: store i1 true, ptr %y.addr.modified.by.chunk
  store i32 %8, ptr %9, align 4
  br label %if.end4

if.else:                                          ; preds = %omp.inner.for.body
  %10 = load i32, ptr %i, align 4
  %11 = load i32, ptr %k.addr, align 4
  %cmp2 = icmp eq i32 %10, %11
  br i1 %cmp2, label %if.then3, label %if.end

if.then3:                                         ; preds = %if.else
  %12 = load i32, ptr %k.addr, align 4
  %13 = load ptr, ptr %y.addr, align 8
; CHECK-DAG: store i1 true, ptr %y.addr.modified.by.chunk
  store i32 %12, ptr %13, align 4
  br label %if.end

if.end:                                           ; preds = %if.then3, %if.else
  br label %if.end4

if.end4:                                          ; preds = %if.end, %if.then
  br label %omp.body.continue

omp.body.continue:                                ; preds = %if.end4
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %14 = load i32, ptr %.omp.iv, align 4
  %add5 = add nsw i32 %14, 1
  store i32 %add5, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

; Check for code emitted at the end of each chunk
; CHECK-DAG:  [[LOAD1:[^ ]+]] = load i1, ptr %y.addr.modified.by.chunk
; CHECK-DAG:  %y.addr.modified = icmp eq i1 [[LOAD1]], true
; CHECK-DAG:  %y.addr.chunk.is.higher = icmp uge i32 %lb.new, [[LOAD2:[^ ]+]]
; CHECK-DAG:  [[LOAD2]] = load i32, ptr %y.addr.local.max.idx
; CHECK-DAG:  %y.addr.modified.and.chunk.is.higher = and i1 %y.addr.modified, %y.addr.chunk.is.higher
; CHECK-DAG:  br i1 %y.addr.modified.and.chunk.is.higher, label %[[LABEL1:[^ ,]+]], label %{{[^ ]+}}
;
; CHECK-DAG:[[LABEL1]]:
; CHECK-DAG:  store i1 true, ptr %y.addr.modified.by.thread
; CHECK-DAG:  store i32 %lb.new, ptr %y.addr.local.max.idx
; CHECK-DAG:  store i32 [[LOAD3:[^ ]+]], ptr %y.addr.lpriv.local.last
; CHECK-DAG:  [[LOAD3]] = load i32, ptr %y.addr.lpriv

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  ret void

; Check for the the computation of global max index to get which thread
; wrote the last chunk

; CHECK-DAG:  %y.addr.written.by.thread = icmp eq i1 [[LOAD4:[^ ]+]], true
; CHECK-DAG:  [[LOAD4]] = load i1, ptr %y.addr.modified.by.thread
; CHECK-DAG:  br i1 %y.addr.written.by.thread, label %[[LABEL2:[^ ,]+]], label %[[LABEL3:[^ ,]+]]
;
; CHECK-DAG:[[LABEL2]]:
; CHECK-DAG:  [[LOAD5:[^ ]+]] = load i32, ptr %y.addr.local.max.idx
; CHECK-DAG:  call void @__kmpc_critical
; CHECK-DAG:  [[LOAD6:[^ ]+]] = load i32, ptr @y.addr.global.max.idx
; CHECK-DAG:  %y.addr.is.local.idx.higher = icmp uge i32 [[LOAD5]], [[LOAD6]]
; CHECK-DAG:  br i1 %y.addr.is.local.idx.higher, label %[[LABEL4:[^ ,]+]], label %[[LABEL5:[^ ,]+]]
;
; CHECK-DAG:[[LABEL4]]:
; CHECK-DAG:  store i32 [[LOAD5]], ptr @y.addr.global.max.idx
; CHECK-DAG:  br label %[[LABEL5]]
;
; CHECK-DAG:[[LABEL5]]:
; CHECK-DAG:  call void @__kmpc_end_critical
; CHECK-DAG:  br label %[[LABEL3]]
;
; CHECK-DAG:[[LABEL3]]:
; CHECK-DAG:  call void @__kmpc_barrier

; Check for the final copyout code
; CHECK-DAG:  [[LOAD7:[^ ]+]] = load i32, ptr %y.addr.local.max.idx
; CHECK-DAG:  [[LOAD8:[^ ]+]] = load i32, ptr @y.addr.global.max.idx
; CHECK-DAG:  %y.addr.copyout.or.not = icmp eq i32 [[LOAD7]], [[LOAD8]]
; CHECK-DAG:  br i1 %y.addr.copyout.or.not, label %[[LABEL6:[^ ,]+]], label %{{.*}}
;
; CHECK-DAG:[[LABEL6]]:
; CHECK-DAG:  store i32 [[LOAD9:[^ ]+]], ptr %y.addr.lpriv
; CHECK-DAG:  [[LOAD9]] = load i32, ptr %y.addr.lpriv.local.last
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { mustprogress noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}
