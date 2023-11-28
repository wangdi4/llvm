; RUN: opt -vpo-paropt-loop-mapping-scheme=1 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-loop-collapse -S %s | FileCheck %s
; RUN: opt -vpo-paropt-loop-mapping-scheme=1 -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse)' -S %s | FileCheck %s
; RUN: opt -vpo-paropt-loop-mapping-scheme=0 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s
; RUN: opt -vpo-paropt-loop-mapping-scheme=0 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s

; Test src:
;
; int aaa[1000];
; void foo() {
; #pragma omp distribute
;   for (int i = 0; i < 1000; ++i) {
; #pragma omp loop
;     for (int j = 0; j < 100; j++) {
;       aaa[i] += i + j;
;     }
;   }
; }

; This test checks that the "loop" construct is mapped to "parallel for"

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@aaa = dso_local global [1000 x i32] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define dso_local void @foo() #0 {
entry:
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  %tmp1 = alloca i32, align 4
  %.omp.iv2 = alloca i32, align 4
  %.omp.lb3 = alloca i32, align 4
  %.omp.ub4 = alloca i32, align 4
  %j = alloca i32, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.iv) #2
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.lb) #2
  store i32 0, ptr %.omp.lb, align 4, !tbaa !4
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.ub) #2
  store i32 999, ptr %.omp.ub, align 4, !tbaa !4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE"(),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv2, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb3, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub4, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %j, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp1, i32 0, i32 1) ]
  %1 = load i32, ptr %.omp.lb, align 4, !tbaa !4
  store i32 %1, ptr %.omp.iv, align 4, !tbaa !4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc14, %entry
  %2 = load i32, ptr %.omp.iv, align 4, !tbaa !4
  %3 = load i32, ptr %.omp.ub, align 4, !tbaa !4
  %cmp = icmp sle i32 %2, %3
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end16

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  call void @llvm.lifetime.start.p0(i64 4, ptr %i) #2
  %4 = load i32, ptr %.omp.iv, align 4, !tbaa !4
  %mul = mul nsw i32 %4, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %i, align 4, !tbaa !4
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.iv2) #2
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.lb3) #2
  store i32 0, ptr %.omp.lb3, align 4, !tbaa !4
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.ub4) #2
  store i32 99, ptr %.omp.ub4, align 4, !tbaa !4

; Verify that DIR.OMP.GENERICLOOP is mapped to DIR.OMP.PARALLEL.LOOP
; CHECK-NOT: call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(), {{.*}}
; CHECK: call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.SHARED:TYPED"({{.*}}), "QUAL.OMP.SHARED:TYPED"({{.*}}), "QUAL.OMP.NORMALIZED.IV:TYPED"({{.*}}), "QUAL.OMP.FIRSTPRIVATE:TYPED"({{.*}}), "QUAL.OMP.NORMALIZED.UB:TYPED"({{.*}}), "QUAL.OMP.PRIVATE:TYPED"({{.*}})

  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(),
    "QUAL.OMP.SHARED:TYPED"(ptr @aaa, i32 0, i64 1000),
    "QUAL.OMP.SHARED:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv2, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb3, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub4, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %j, i32 0, i32 1) ]
  %6 = load i32, ptr %.omp.lb3, align 4, !tbaa !4
  store i32 %6, ptr %.omp.iv2, align 4, !tbaa !4
  br label %omp.inner.for.cond5

omp.inner.for.cond5:                              ; preds = %omp.inner.for.inc, %omp.inner.for.body
  %7 = load i32, ptr %.omp.iv2, align 4, !tbaa !4
  %8 = load i32, ptr %.omp.ub4, align 4, !tbaa !4
  %cmp6 = icmp sle i32 %7, %8
  br i1 %cmp6, label %omp.inner.for.body7, label %omp.inner.for.end

omp.inner.for.body7:                              ; preds = %omp.inner.for.cond5
  call void @llvm.lifetime.start.p0(i64 4, ptr %j) #2
  %9 = load i32, ptr %.omp.iv2, align 4, !tbaa !4
  %mul8 = mul nsw i32 %9, 1
  %add9 = add nsw i32 0, %mul8
  store i32 %add9, ptr %j, align 4, !tbaa !4
  %10 = load i32, ptr %i, align 4, !tbaa !4
  %11 = load i32, ptr %j, align 4, !tbaa !4
  %add10 = add nsw i32 %10, %11
  %12 = load i32, ptr %i, align 4, !tbaa !4
  %idxprom = sext i32 %12 to i64
  %arrayidx = getelementptr inbounds [1000 x i32], ptr @aaa, i64 0, i64 %idxprom, !intel-tbaa !8
  %13 = load i32, ptr %arrayidx, align 4, !tbaa !8
  %add11 = add nsw i32 %13, %add10
  store i32 %add11, ptr %arrayidx, align 4, !tbaa !8
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body7
  call void @llvm.lifetime.end.p0(i64 4, ptr %j) #2
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %14 = load i32, ptr %.omp.iv2, align 4, !tbaa !4
  %add12 = add nsw i32 %14, 1
  store i32 %add12, ptr %.omp.iv2, align 4, !tbaa !4
  br label %omp.inner.for.cond5

omp.inner.for.end:                                ; preds = %omp.inner.for.cond5
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end

; CHECK-NOT: call void @llvm.directive.region.exit(token %{{.*}}) [ "DIR.OMP.END.GENERICLOOP"() {{.*}}
; CHECK: call void @llvm.directive.region.exit(token %{{.*}}) [ "DIR.OMP.END.PARALLEL.LOOP"() {{.*}}

  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.GENERICLOOP"() ]
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.ub4) #2
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.lb3) #2
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.iv2) #2
  br label %omp.body.continue13

omp.body.continue13:                              ; preds = %omp.loop.exit
  call void @llvm.lifetime.end.p0(i64 4, ptr %i) #2
  br label %omp.inner.for.inc14

omp.inner.for.inc14:                              ; preds = %omp.body.continue13
  %15 = load i32, ptr %.omp.iv, align 4, !tbaa !4
  %add15 = add nsw i32 %15, 1
  store i32 %add15, ptr %.omp.iv, align 4, !tbaa !4
  br label %omp.inner.for.cond

omp.inner.for.end16:                              ; preds = %omp.inner.for.cond
  br label %omp.loop.exit17

omp.loop.exit17:                                  ; preds = %omp.inner.for.end16
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.DISTRIBUTE"() ]
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.ub) #2
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.lb) #2
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.iv) #2
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
