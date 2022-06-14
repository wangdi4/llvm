; RUN: not opt -enable-new-pm=0 -vpo-cfg-restructuring -vpo-wrncollection -analyze -S %s 2>&1 | FileCheck %s
; RUN: not opt -passes='function(vpo-cfg-restructuring,print<vpo-wrncollection>)' -S %s 2>&1 | FileCheck %s

; Test src:

; int x = 0;
; int a[10];
; int p[10];
;
; void foo() {
; #pragma omp for reduction(inscan,+: x)
;   for (int i = 0; i < 10; i++) {
;     p[i] = x;
; #pragma omp scan exclusive(x)
;     x += a[i];
;   }
; }

; The IR is a hand-modified version of the above test without scan/incan.

; CHECK: error: <unknown>:0:0: in function foo void (): reduction(inscan) is not supported on the loop construct.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = dso_local global [10 x i32] zeroinitializer, align 16
@p = dso_local global [10 x i32] zeroinitializer, align 16
@x = dso_local global i32 0, align 4

; Function Attrs: nounwind uwtable
define dso_local void @foo() #0 {
entry:
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  %0 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #2
  %1 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %1) #2
  store i32 9, i32* %.omp.ub, align 4, !tbaa !4

  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(),
  "QUAL.OMP.REDUCTION.ADD:TYPED.INSCAN"(i32* @x, i32 0, i32 1, i64 1),
  "QUAL.OMP.NORMALIZED.IV:TYPED"(i32* %.omp.iv, i32 0),
  "QUAL.OMP.NORMALIZED.UB:TYPED"(i32* %.omp.ub, i32 0),
  "QUAL.OMP.LINEAR:IV.TYPED"(i32* %i, i32 0, i32 1, i32 1) ]

  store i32 0, i32* %.omp.iv, align 4, !tbaa !4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %3 = load i32, i32* %.omp.iv, align 4, !tbaa !4
  %4 = load i32, i32* %.omp.ub, align 4, !tbaa !4
  %cmp = icmp sle i32 %3, %4
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %5 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %5) #2
  %6 = load i32, i32* %.omp.iv, align 4, !tbaa !4
  %mul = mul nsw i32 %6, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %i, align 4, !tbaa !4
  %7 = load i32, i32* @x, align 4, !tbaa !4
  %8 = load i32, i32* %i, align 4, !tbaa !4
  %idxprom = sext i32 %8 to i64
  %arrayidx = getelementptr inbounds [10 x i32], [10 x i32]* @p, i64 0, i64 %idxprom, !intel-tbaa !8
  store i32 %7, i32* %arrayidx, align 4, !tbaa !8

  %scan = call token @llvm.directive.region.entry() [ "DIR.OMP.SCAN"(), "QUAL.OMP.EXCLUSIVE"(i32* @x, i64 1) ]
  fence acq_rel
  call void @llvm.directive.region.exit(token %scan) [ "DIR.OMP.END.SCAN"() ]

  %9 = load i32, i32* %i, align 4, !tbaa !4
  %idxprom1 = sext i32 %9 to i64
  %arrayidx2 = getelementptr inbounds [10 x i32], [10 x i32]* @a, i64 0, i64 %idxprom1, !intel-tbaa !8
  %10 = load i32, i32* %arrayidx2, align 4, !tbaa !8
  %11 = load i32, i32* @x, align 4, !tbaa !4
  %add3 = add nsw i32 %11, %10
  store i32 %add3, i32* @x, align 4, !tbaa !4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  %12 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %12) #2
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %13 = load i32, i32* %.omp.iv, align 4, !tbaa !4
  %add4 = add nsw i32 %13, 1
  store i32 %add4, i32* %.omp.iv, align 4, !tbaa !4
  br label %omp.inner.for.cond, !llvm.loop !10

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.LOOP"() ]
  %14 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %14) #2
  %15 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %15) #2
  ret void
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

attributes #0 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { argmemonly nofree nosync nounwind willreturn }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 1}
!3 = !{!"clang version 13.0.0"}
!4 = !{!5, !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
!8 = !{!9, !5, i64 0}
!9 = !{!"array@_ZTSA10_i", !5, i64 0}
!10 = distinct !{!10, !11}
!11 = !{!"llvm.loop.vectorize.enable", i1 true}
