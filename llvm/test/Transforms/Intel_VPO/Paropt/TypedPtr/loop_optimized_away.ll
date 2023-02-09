; RUN: opt -opaque-pointers=0 -enable-new-pm=0 -vpo-paropt -S -pass-remarks-missed=openmp %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers=0 -passes='vpo-paropt' -S -pass-remarks-missed=openmp %s 2>&1 | FileCheck %s

; Test src:

; #include <stdlib.h>

; void __attribute__((always_inline)) foo() { abort(); };
; int a[10];
; int zoo() {
; #pragma omp for
;  for (int i = 0; i < 10; i++) {
;    for (int j = 0; j < 10; j++)
;      a[j] = j;
;    foo();
;  }
; }

; Check for remarks about the loop construct being optimized away and ignored.
; CHECK: remark: <unknown>:0:0: loop construct's associated loop was optimized away

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@a = dso_local local_unnamed_addr global [10 x i32] zeroinitializer, align 16

; Function Attrs: alwaysinline mustprogress noreturn nounwind uwtable
define dso_local void @_Z3foov() local_unnamed_addr #0 {
entry:
  call void @abort() #5
  unreachable
}

; Function Attrs: nofree noreturn nounwind
declare dso_local void @abort() local_unnamed_addr #1

; Function Attrs: mustprogress nounwind uwtable
define dso_local noundef i32 @_Z3zoov() local_unnamed_addr #2 {
DIR.OMP.LOOP.25:
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  %0 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %0) #4
  %1 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %1) #4
  store i32 0, i32* %.omp.lb, align 4, !tbaa !4
  %2 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %2) #4
  store volatile i32 9, i32* %.omp.ub, align 4, !tbaa !4
  br label %DIR.OMP.LOOP.1

DIR.OMP.LOOP.1:                                   ; preds = %DIR.OMP.LOOP.25
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.PRIVATE"(i32* %j) ]
  br label %DIR.OMP.LOOP.26

DIR.OMP.LOOP.26:                                  ; preds = %DIR.OMP.LOOP.1
  br label %DIR.OMP.LOOP.2

DIR.OMP.LOOP.2:                                   ; preds = %DIR.OMP.LOOP.26
  %4 = load i32, i32* %.omp.lb, align 4, !tbaa !4
  store volatile i32 %4, i32* %.omp.iv, align 4, !tbaa !4
  %5 = load volatile i32, i32* %.omp.iv, align 4, !tbaa !4
  %6 = load volatile i32, i32* %.omp.ub, align 4, !tbaa !4
  %cmp.not = icmp sgt i32 %5, %6
  br i1 %cmp.not, label %DIR.OMP.END.LOOP.4, label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.LOOP.2
  %7 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %7) #4
  %8 = load volatile i32, i32* %.omp.iv, align 4, !tbaa !4
  store i32 %8, i32* %i, align 4, !tbaa !4
  %9 = bitcast i32* %j to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %9) #4
  br label %for.cond

for.cond:                                         ; preds = %for.body, %omp.inner.for.body
  %storemerge = phi i32 [ 0, %omp.inner.for.body ], [ %inc, %for.body ]
  store i32 %storemerge, i32* %j, align 4, !tbaa !4
  %cmp1 = icmp slt i32 %storemerge, 10
  br i1 %cmp1, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %9) #4
  call void @abort() #5
  unreachable

for.body:                                         ; preds = %for.cond
  %idxprom = sext i32 %storemerge to i64
  %arrayidx = getelementptr inbounds [10 x i32], [10 x i32]* @a, i64 0, i64 %idxprom, !intel-tbaa !8
  store i32 %storemerge, i32* %arrayidx, align 4, !tbaa !8
  %10 = load i32, i32* %j, align 4, !tbaa !4
  %inc = add nsw i32 %10, 1
  br label %for.cond, !llvm.loop !10

DIR.OMP.END.LOOP.4:                               ; preds = %DIR.OMP.LOOP.2
  br label %DIR.OMP.END.LOOP.3

DIR.OMP.END.LOOP.3:                               ; preds = %DIR.OMP.END.LOOP.4
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.LOOP"() ]
  br label %DIR.OMP.END.LOOP.47

DIR.OMP.END.LOOP.47:                              ; preds = %DIR.OMP.END.LOOP.3
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %2) #4
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %1) #4
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %0) #4
  ret i32 undef
}

; Function Attrs: argmemonly mustprogress nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #3

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #4

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #4

; Function Attrs: argmemonly mustprogress nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #3

attributes #0 = { alwaysinline mustprogress noreturn nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nofree noreturn nounwind "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #2 = { mustprogress nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #3 = { argmemonly mustprogress nocallback nofree nosync nounwind willreturn }
attributes #4 = { nounwind }
attributes #5 = { noreturn nounwind }

!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 50}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{!"clang version 15.0.0"}
!4 = !{!5, !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C++ TBAA"}
!8 = !{!9, !5, i64 0}
!9 = !{!"array@_ZTSA10_i", !5, i64 0}
!10 = distinct !{!10, !11}
!11 = !{!"llvm.loop.mustprogress"}
