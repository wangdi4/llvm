; RUN: opt -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s
;
; // C source for testing schedule modifiers MONOTONIC and NONMONOTONIC
; int main() {
;   int i;
;   #pragma omp for schedule(monotonic:dynamic)
;   for (i = 0; i < 1000; ++i);
;
;   #pragma omp for schedule(nonmonotonic:dynamic)
;   for (i = 0; i < 1000; ++i);
;
;   #pragma omp for schedule(nonmonotonic:static)
;   for (i = 0; i < 1000; ++i);
; }
;
; Dynamic schedule type is 35, so monotonic:dynamic is: 35 | (1<<29) = 536870947
; CHECK:  call void @__kmpc_dispatch_init_4({{.*}} i32 536870947,
;
; Dynamic schedule type is 35, so nonmonotonic:dynamic is: 35 | (1<<30) = 1073741859
; CHECK:  call void @__kmpc_dispatch_init_4({{.*}} i32 1073741859,
;
; Static schedule type is 34. Check that nonmonotonic:static remains at 34
; CHECK: call void @__kmpc_for_static_init_4({{.*}} i32 34,

; ModuleID = 'lit.c'
source_filename = "lit.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %i = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %tmp2 = alloca i32, align 4
  %.omp.iv3 = alloca i32, align 4
  %.omp.lb4 = alloca i32, align 4
  %.omp.ub5 = alloca i32, align 4
  %tmp16 = alloca i32, align 4
  %.omp.iv17 = alloca i32, align 4
  %.omp.lb18 = alloca i32, align 4
  %.omp.ub19 = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  store i32 0, i32* %.omp.lb, align 4
  store i32 999, i32* %.omp.ub, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(), "QUAL.OMP.SCHEDULE.DYNAMIC:MONOTONIC"(i32 1), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub) ]
  %1 = load i32, i32* %.omp.lb, align 4
  store i32 %1, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %2 = load i32, i32* %.omp.iv, align 4
  %3 = load i32, i32* %.omp.ub, align 4
  %cmp = icmp sle i32 %2, %3
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %4 = load i32, i32* %.omp.iv, align 4
  %mul = mul nsw i32 %4, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %i, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %5 = load i32, i32* %.omp.iv, align 4
  %add1 = add nsw i32 %5, 1
  store i32 %add1, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.LOOP"() ]
  store i32 0, i32* %.omp.lb4, align 4
  store i32 999, i32* %.omp.ub5, align 4
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(), "QUAL.OMP.SCHEDULE.DYNAMIC:NONMONOTONIC"(i32 1), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv3), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb4), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub5) ]
  %7 = load i32, i32* %.omp.lb4, align 4
  store i32 %7, i32* %.omp.iv3, align 4
  br label %omp.inner.for.cond6

omp.inner.for.cond6:                              ; preds = %omp.inner.for.inc12, %omp.loop.exit
  %8 = load i32, i32* %.omp.iv3, align 4
  %9 = load i32, i32* %.omp.ub5, align 4
  %cmp7 = icmp sle i32 %8, %9
  br i1 %cmp7, label %omp.inner.for.body8, label %omp.inner.for.end14

omp.inner.for.body8:                              ; preds = %omp.inner.for.cond6
  %10 = load i32, i32* %.omp.iv3, align 4
  %mul9 = mul nsw i32 %10, 1
  %add10 = add nsw i32 0, %mul9
  store i32 %add10, i32* %i, align 4
  br label %omp.body.continue11

omp.body.continue11:                              ; preds = %omp.inner.for.body8
  br label %omp.inner.for.inc12

omp.inner.for.inc12:                              ; preds = %omp.body.continue11
  %11 = load i32, i32* %.omp.iv3, align 4
  %add13 = add nsw i32 %11, 1
  store i32 %add13, i32* %.omp.iv3, align 4
  br label %omp.inner.for.cond6

omp.inner.for.end14:                              ; preds = %omp.inner.for.cond6
  br label %omp.loop.exit15

omp.loop.exit15:                                  ; preds = %omp.inner.for.end14
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.LOOP"() ]
  store i32 0, i32* %.omp.lb18, align 4
  store i32 999, i32* %.omp.ub19, align 4
  %12 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(), "QUAL.OMP.SCHEDULE.STATIC:NONMONOTONIC"(i32 0), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv17), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb18), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub19) ]
  %13 = load i32, i32* %.omp.lb18, align 4
  store i32 %13, i32* %.omp.iv17, align 4
  br label %omp.inner.for.cond20

omp.inner.for.cond20:                             ; preds = %omp.inner.for.inc26, %omp.loop.exit15
  %14 = load i32, i32* %.omp.iv17, align 4
  %15 = load i32, i32* %.omp.ub19, align 4
  %cmp21 = icmp sle i32 %14, %15
  br i1 %cmp21, label %omp.inner.for.body22, label %omp.inner.for.end28

omp.inner.for.body22:                             ; preds = %omp.inner.for.cond20
  %16 = load i32, i32* %.omp.iv17, align 4
  %mul23 = mul nsw i32 %16, 1
  %add24 = add nsw i32 0, %mul23
  store i32 %add24, i32* %i, align 4
  br label %omp.body.continue25

omp.body.continue25:                              ; preds = %omp.inner.for.body22
  br label %omp.inner.for.inc26

omp.inner.for.inc26:                              ; preds = %omp.body.continue25
  %17 = load i32, i32* %.omp.iv17, align 4
  %add27 = add nsw i32 %17, 1
  store i32 %add27, i32* %.omp.iv17, align 4
  br label %omp.inner.for.cond20

omp.inner.for.end28:                              ; preds = %omp.inner.for.cond20
  br label %omp.loop.exit29

omp.loop.exit29:                                  ; preds = %omp.inner.for.end28
  call void @llvm.directive.region.exit(token %12) [ "DIR.OMP.END.LOOP"() ]
  %18 = load i32, i32* %retval, align 4
  ret i32 %18
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 50}
!2 = !{i32 7, !"uwtable", i32 1}
!3 = !{i32 7, !"frame-pointer", i32 2}
