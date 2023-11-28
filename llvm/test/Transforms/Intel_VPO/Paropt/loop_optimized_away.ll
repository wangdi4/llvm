; RUN: opt -bugpoint-enable-legacy-pm -vpo-paropt -S -pass-remarks-missed=openmp %s 2>&1 | FileCheck %s
; RUN: opt -passes='vpo-paropt' -S -pass-remarks-missed=openmp %s 2>&1 | FileCheck %s

; Test src:
;
; #include <stdlib.h>
;
; void __attribute__((always_inline)) foo() { abort(); };
; int a[10];
; void zoo() {
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

; Function Attrs: alwaysinline nounwind uwtable
define dso_local void @foo() local_unnamed_addr #0 {
entry:
  call void @abort() #5
  unreachable
}

; Function Attrs: nofree noreturn nounwind
declare dso_local void @abort() local_unnamed_addr #1

; Function Attrs: nounwind uwtable
define dso_local void @zoo() local_unnamed_addr #2 {
DIR.OMP.LOOP.25:
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %.omp.iv) #4
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %.omp.lb) #4
  store i32 0, ptr %.omp.lb, align 4, !tbaa !3
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %.omp.ub) #4
  store volatile i32 9, ptr %.omp.ub, align 4, !tbaa !3
  %end.dir.temp = alloca i1, align 1
  br label %DIR.OMP.LOOP.1

DIR.OMP.LOOP.1:                                   ; preds = %DIR.OMP.LOOP.25
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %j, i32 0, i32 1),
    "QUAL.OMP.JUMP.TO.END.IF"(ptr %end.dir.temp) ]
  br label %DIR.OMP.LOOP.26

DIR.OMP.LOOP.26:                                  ; preds = %DIR.OMP.LOOP.1
  %temp.load = load volatile i1, ptr %end.dir.temp, align 1
  br i1 %temp.load, label %DIR.OMP.END.LOOP.4, label %DIR.OMP.LOOP.2

DIR.OMP.LOOP.2:                                   ; preds = %DIR.OMP.LOOP.26
  %1 = load i32, ptr %.omp.lb, align 4, !tbaa !3
  store volatile i32 %1, ptr %.omp.iv, align 4, !tbaa !3
  %2 = load volatile i32, ptr %.omp.iv, align 4, !tbaa !3
  %3 = load volatile i32, ptr %.omp.ub, align 4, !tbaa !3
  %cmp.not = icmp sgt i32 %2, %3
  br i1 %cmp.not, label %DIR.OMP.END.LOOP.4, label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.LOOP.2
  call void @llvm.lifetime.start.p0(i64 4, ptr %i) #4
  %4 = load volatile i32, ptr %.omp.iv, align 4, !tbaa !3
  store i32 %4, ptr %i, align 4, !tbaa !3
  call void @llvm.lifetime.start.p0(i64 4, ptr %j) #4
  br label %for.cond

for.cond:                                         ; preds = %for.body, %omp.inner.for.body
  %storemerge = phi i32 [ 0, %omp.inner.for.body ], [ %inc, %for.body ]
  store i32 %storemerge, ptr %j, align 4, !tbaa !3
  %cmp1 = icmp slt i32 %storemerge, 10
  br i1 %cmp1, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %j) #4
  call void @abort() #5
  unreachable

for.body:                                         ; preds = %for.cond
  %idxprom = sext i32 %storemerge to i64
  %arrayidx = getelementptr inbounds [10 x i32], ptr @a, i64 0, i64 %idxprom, !intel-tbaa !7
  store i32 %storemerge, ptr %arrayidx, align 4, !tbaa !7
  %5 = load i32, ptr %j, align 4, !tbaa !3
  %inc = add nsw i32 %5, 1
  br label %for.cond, !llvm.loop !9

DIR.OMP.END.LOOP.4:                               ; preds = %DIR.OMP.LOOP.2, %DIR.OMP.LOOP.26
  br label %DIR.OMP.END.LOOP.3

DIR.OMP.END.LOOP.3:                               ; preds = %DIR.OMP.END.LOOP.4
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.LOOP"() ]
  br label %DIR.OMP.END.LOOP.47

DIR.OMP.END.LOOP.47:                              ; preds = %DIR.OMP.END.LOOP.3
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %.omp.ub) #4
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %.omp.lb) #4
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %.omp.iv) #4
  ret void
}

; Function Attrs: argmemonly mustprogress nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #3

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #4

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #4

; Function Attrs: argmemonly mustprogress nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #3

attributes #0 = { alwaysinline nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nofree noreturn nounwind "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #2 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #3 = { argmemonly mustprogress nocallback nofree nosync nounwind willreturn }
attributes #4 = { nounwind }
attributes #5 = { noreturn nounwind }

!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = !{!8, !4, i64 0}
!8 = !{!"array@_ZTSA10_i", !4, i64 0}
!9 = distinct !{!9, !10}
!10 = !{!"llvm.loop.mustprogress"}
