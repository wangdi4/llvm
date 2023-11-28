; RUN: opt -vpo-paropt-loop-mapping-scheme=1 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-loop-collapse -S %s | FileCheck %s
; RUN: opt -vpo-paropt-loop-mapping-scheme=1 -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse)' -S %s | FileCheck %s
; RUN: opt -vpo-paropt-loop-mapping-scheme=0 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s
; RUN: opt -vpo-paropt-loop-mapping-scheme=0 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s

; Test src:
;
; int aaa[1000];
; void foo() {
;   for (int i = 0; i < 1000; ++i) {
; #pragma omp loop bind(parallel)
;     for (int j = 0; j < 100; j++) {
;       aaa[i] += i + j;
;     }
;   }
; }

; This test checks that the "loop" construct is mapped to "for"
; after prepare pass, if binding rule is parallel.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@aaa = dso_local global [1000 x i32] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define dso_local void @foo() #0 {
entry:
  %i = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %j = alloca i32, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr %i) #2
  store i32 0, ptr %i, align 4, !tbaa !4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %0 = load i32, ptr %i, align 4, !tbaa !4
  %cmp = icmp slt i32 %0, 1000
  br i1 %cmp, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  call void @llvm.lifetime.end.p0(i64 4, ptr %i) #2
  br label %for.end

for.body:                                         ; preds = %for.cond
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.iv) #2
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.lb) #2
  store i32 0, ptr %.omp.lb, align 4, !tbaa !4
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.ub) #2
  store i32 99, ptr %.omp.ub, align 4, !tbaa !4

; Verify that DIR.OMP.GENERICLOOP is mapped to DIR.OMP.LOOP
; CHECK-NOT: call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(), {{.*}}
; CHECK: call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(),
; CHECK-NOT: "QUAL.OMP.BIND.PARALLEL
; CHECK-NOT: "QUAL.OMP.SHARED
; CHECK-SAME: "QUAL.OMP.NORMALIZED.IV:TYPED"({{.*}}), "QUAL.OMP.FIRSTPRIVATE:TYPED"({{.*}}), "QUAL.OMP.NORMALIZED.UB:TYPED"({{.*}}), "QUAL.OMP.PRIVATE:TYPED"({{.*}})

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(),
    "QUAL.OMP.BIND.PARALLEL"(),
    "QUAL.OMP.SHARED:TYPED"(ptr @aaa, i32 0, i64 1000),
    "QUAL.OMP.SHARED:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %j, i32 0, i32 1) ]
  %2 = load i32, ptr %.omp.lb, align 4, !tbaa !4
  store i32 %2, ptr %.omp.iv, align 4, !tbaa !4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %for.body
  %3 = load i32, ptr %.omp.iv, align 4, !tbaa !4
  %4 = load i32, ptr %.omp.ub, align 4, !tbaa !4
  %cmp1 = icmp sle i32 %3, %4
  br i1 %cmp1, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  call void @llvm.lifetime.start.p0(i64 4, ptr %j) #2
  %5 = load i32, ptr %.omp.iv, align 4, !tbaa !4
  %mul = mul nsw i32 %5, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %j, align 4, !tbaa !4
  %6 = load i32, ptr %i, align 4, !tbaa !4
  %7 = load i32, ptr %j, align 4, !tbaa !4
  %add2 = add nsw i32 %6, %7
  %8 = load i32, ptr %i, align 4, !tbaa !4
  %idxprom = sext i32 %8 to i64
  %arrayidx = getelementptr inbounds [1000 x i32], ptr @aaa, i64 0, i64 %idxprom, !intel-tbaa !8
  %9 = load i32, ptr %arrayidx, align 4, !tbaa !8
  %add3 = add nsw i32 %9, %add2
  store i32 %add3, ptr %arrayidx, align 4, !tbaa !8
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  call void @llvm.lifetime.end.p0(i64 4, ptr %j) #2
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %10 = load i32, ptr %.omp.iv, align 4, !tbaa !4
  %add4 = add nsw i32 %10, 1
  store i32 %add4, ptr %.omp.iv, align 4, !tbaa !4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end

; CHECK-NOT: call void @llvm.directive.region.exit(token %{{.*}}) [ "DIR.OMP.END.GENERICLOOP"() {{.*}}
; CHECK: call void @llvm.directive.region.exit(token %{{.*}}) [ "DIR.OMP.END.LOOP"() {{.*}}

  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.GENERICLOOP"() ]
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.ub) #2
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.lb) #2
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.iv) #2
  br label %for.inc

for.inc:                                          ; preds = %omp.loop.exit
  %11 = load i32, ptr %i, align 4, !tbaa !4
  %inc = add nsw i32 %11, 1
  store i32 %inc, ptr %i, align 4, !tbaa !4
  br label %for.cond, !llvm.loop !10

for.end:                                          ; preds = %for.cond.cleanup
  ret void
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
!9 = !{!"array@_ZTSA1000_i", !5, i64 0}
!10 = distinct !{!10, !11}
!11 = !{!"llvm.loop.mustprogress"}
