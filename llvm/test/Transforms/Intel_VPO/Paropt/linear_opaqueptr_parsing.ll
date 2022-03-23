; REQUIRES: asserts
; RUN: opt -enable-new-pm=0 -opaque-pointers -vpo-cfg-restructuring -vpo-wrncollection -analyze -S %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -passes='function(vpo-cfg-restructuring,print<vpo-wrncollection>)' -S %s 2>&1 | FileCheck %s


; #include <omp.h>
; int x[10];
; int y = 1;
; int main() {
; #pragma omp for linear (y)
;   for (int i = 0; i < 10; i++) {
;     x[i] = y;
;   }
; }


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"


; CHECK: PRIVATE clause (size=1): (ptr %{{.*}}) , TYPED (TYPE: i32, NUM_ELEMENTS: i32 1)
; CHECK: FIRSTPRIVATE clause (size=1): (ptr %{{.*}}) , TYPED (TYPE: i32, NUM_ELEMENTS: i32 1)
; CHECK: LINEAR clause (size=1): (ptr @y, TYPED (TYPE: i32, NUM_ELEMENTS: i32 1), i32 1)
; CHECK: IV clause:   %.omp.iv = alloca i32, align 4, TYPED (TYPE: i32, NUM_ELEMENTS: i32 1)
; CHECK: UB clause:   %.omp.ub = alloca i32, align 4, TYPED (TYPE: i32, NUM_ELEMENTS: i32 1)



@x = dso_local global [10 x i32] zeroinitializer, align 16
@y = dso_local global i32 1, align 4

; Function Attrs: mustprogress norecurse nounwind uwtable
define dso_local i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  %0 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #2
  %1 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %1) #2
  store i32 0, i32* %.omp.lb, align 4, !tbaa !4
  %2 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %2) #2
  store i32 9, i32* %.omp.ub, align 4, !tbaa !4
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(), "QUAL.OMP.LINEAR:TYPED"(i32* @y, i32 0, i32 1, i32 1), "QUAL.OMP.NORMALIZED.IV:TYPED"(i32* %.omp.iv, i32 0), "QUAL.OMP.FIRSTPRIVATE:TYPED"(i32* %.omp.lb, i32 0, i32 1), "QUAL.OMP.NORMALIZED.UB:TYPED"(i32* %.omp.ub, i32 0), "QUAL.OMP.PRIVATE:TYPED"(i32* %i, i32 0, i32 1) ]
  %4 = load i32, i32* %.omp.lb, align 4, !tbaa !4
  store i32 %4, i32* %.omp.iv, align 4, !tbaa !4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %5 = load i32, i32* %.omp.iv, align 4, !tbaa !4
  %6 = load i32, i32* %.omp.ub, align 4, !tbaa !4
  %cmp = icmp sle i32 %5, %6
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %7 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %7) #2
  %8 = load i32, i32* %.omp.iv, align 4, !tbaa !4
  %mul = mul nsw i32 %8, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %i, align 4, !tbaa !4
  %9 = load i32, i32* @y, align 4, !tbaa !4
  %10 = load i32, i32* %i, align 4, !tbaa !4
  %idxprom = sext i32 %10 to i64
  %arrayidx = getelementptr inbounds [10 x i32], [10 x i32]* @x, i64 0, i64 %idxprom, !intel-tbaa !8
  store i32 %9, i32* %arrayidx, align 4, !tbaa !8
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  %11 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %11) #2
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %12 = load i32, i32* %.omp.iv, align 4, !tbaa !4
  %add1 = add nsw i32 %12, 1
  store i32 %add1, i32* %.omp.iv, align 4, !tbaa !4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.LOOP"() ]
  %13 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %13) #2
  %14 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %14) #2
  %15 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %15) #2
  %16 = load i32, i32* %retval, align 4
  ret i32 %16
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

attributes #0 = { mustprogress norecurse nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { argmemonly nofree nosync nounwind willreturn }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 50}
!2 = !{i32 7, !"uwtable", i32 1}
!4 = !{!5, !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C++ TBAA"}
!8 = !{!9, !5, i64 0}
!9 = !{!"array@_ZTSA10_i", !5, i64 0}
