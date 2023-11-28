; REQUIRES: asserts
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-wrncollection -analyze -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,print<vpo-wrncollection>)' -S %s 2>&1 | FileCheck %s

; Test src:
;
; #include <omp.h>
; int x[10];
; int y = 1;
; int main() {
; #pragma omp for linear (y)
;   for (int i = 0; i < 10; i++) {
;     x[i] = y;
;   }
; }

; CHECK: PRIVATE clause (size=1): TYPED(ptr %{{.*}}, TYPE: i32, NUM_ELEMENTS: i32 1)
; CHECK: FIRSTPRIVATE clause (size=1): TYPED(ptr %{{.*}}, TYPE: i32, NUM_ELEMENTS: i32 1)
; CHECK: LINEAR clause (size=1): (TYPED(ptr @y, TYPE: i32, NUM_ELEMENTS: i32 1), i32 1)
; CHECK: IV clause:   %.omp.iv = alloca i32, align 4, TYPED (TYPE: i32, NUM_ELEMENTS: i32 1)
; CHECK: UB clause:   %.omp.ub = alloca i32, align 4, TYPED (TYPE: i32, NUM_ELEMENTS: i32 1)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@x = dso_local global [10 x i32] zeroinitializer, align 16
@y = dso_local global i32 1, align 4

; Function Attrs: nounwind uwtable
define dso_local i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 0, ptr %retval, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.iv) #2
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.lb) #2
  store i32 0, ptr %.omp.lb, align 4, !tbaa !4
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.ub) #2
  store i32 9, ptr %.omp.ub, align 4, !tbaa !4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(),
    "QUAL.OMP.LINEAR:TYPED"(ptr @y, i32 0, i32 1, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1) ]
  %1 = load i32, ptr %.omp.lb, align 4, !tbaa !4
  store i32 %1, ptr %.omp.iv, align 4, !tbaa !4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %2 = load i32, ptr %.omp.iv, align 4, !tbaa !4
  %3 = load i32, ptr %.omp.ub, align 4, !tbaa !4
  %cmp = icmp sle i32 %2, %3
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  call void @llvm.lifetime.start.p0(i64 4, ptr %i) #2
  %4 = load i32, ptr %.omp.iv, align 4, !tbaa !4
  %mul = mul nsw i32 %4, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %i, align 4, !tbaa !4
  %5 = load i32, ptr @y, align 4, !tbaa !4
  %6 = load i32, ptr %i, align 4, !tbaa !4
  %idxprom = sext i32 %6 to i64
  %arrayidx = getelementptr inbounds [10 x i32], ptr @x, i64 0, i64 %idxprom, !intel-tbaa !8
  store i32 %5, ptr %arrayidx, align 4, !tbaa !8
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  call void @llvm.lifetime.end.p0(i64 4, ptr %i) #2
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %7 = load i32, ptr %.omp.iv, align 4, !tbaa !4
  %add1 = add nsw i32 %7, 1
  store i32 %add1, ptr %.omp.iv, align 4, !tbaa !4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.LOOP"() ]
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.ub) #2
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.lb) #2
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.iv) #2
  %8 = load i32, ptr %retval, align 4
  ret i32 %8
}

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

attributes #0 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { argmemonly nocallback nofree nosync nounwind willreturn }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!4 = !{!5, !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
!8 = !{!9, !5, i64 0}
!9 = !{!"array@_ZTSA10_i", !5, i64 0}
