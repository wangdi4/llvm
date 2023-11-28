; RUN: opt -vpo-paropt-loop-mapping-scheme=1 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-loop-collapse -S %s | FileCheck %s
; RUN: opt -vpo-paropt-loop-mapping-scheme=1 -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse)' -S %s | FileCheck %s
; RUN: opt -vpo-paropt-loop-mapping-scheme=0 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s
; RUN: opt -vpo-paropt-loop-mapping-scheme=0 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s

; Test src:
;
; int aaa[1000];
; void foo() {
; #pragma omp target
; #pragma omp loop
;   for (int i = 0; i < 100; i++) {
;     aaa[i] += i + i;
;   }
; }

; Check that the "loop" construct is mapped to "parallel for"

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
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr @aaa, ptr @aaa, i64 4000, i64 547, ptr null, ptr null),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp, i32 0, i32 1) ]
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.iv) #1
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.lb) #1
  store i32 0, ptr %.omp.lb, align 4, !tbaa !5
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.ub) #1
  store i32 99, ptr %.omp.ub, align 4, !tbaa !5

; Verify that GENERICLOOP is mapped to PARALLEL.LOOP
; CHECK-NOT: call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(), {{.*}}
; CHECK: call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.SHARED:TYPED"({{.*}}), "QUAL.OMP.NORMALIZED.IV:TYPED"({{.*}}), "QUAL.OMP.FIRSTPRIVATE:TYPED"({{.*}}), "QUAL.OMP.NORMALIZED.UB:TYPED"({{.*}}), "QUAL.OMP.PRIVATE:TYPED"({{.*}})

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(),
    "QUAL.OMP.SHARED:TYPED"(ptr @aaa, i32 0, i64 1000),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1) ]
  %2 = load i32, ptr %.omp.lb, align 4, !tbaa !5
  store i32 %2, ptr %.omp.iv, align 4, !tbaa !5
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %3 = load i32, ptr %.omp.iv, align 4, !tbaa !5
  %4 = load i32, ptr %.omp.ub, align 4, !tbaa !5
  %cmp = icmp sle i32 %3, %4
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  call void @llvm.lifetime.start.p0(i64 4, ptr %i) #1
  %5 = load i32, ptr %.omp.iv, align 4, !tbaa !5
  %mul = mul nsw i32 %5, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %i, align 4, !tbaa !5
  %6 = load i32, ptr %i, align 4, !tbaa !5
  %7 = load i32, ptr %i, align 4, !tbaa !5
  %add1 = add nsw i32 %6, %7
  %8 = load i32, ptr %i, align 4, !tbaa !5
  %idxprom = sext i32 %8 to i64
  %arrayidx = getelementptr inbounds [1000 x i32], ptr @aaa, i64 0, i64 %idxprom, !intel-tbaa !9
  %9 = load i32, ptr %arrayidx, align 4, !tbaa !9
  %add2 = add nsw i32 %9, %add1
  store i32 %add2, ptr %arrayidx, align 4, !tbaa !9
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  call void @llvm.lifetime.end.p0(i64 4, ptr %i) #1
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %10 = load i32, ptr %.omp.iv, align 4, !tbaa !5
  %add3 = add nsw i32 %10, 1
  store i32 %add3, ptr %.omp.iv, align 4, !tbaa !5
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end

; CHECK-NOT: call void @llvm.directive.region.exit(token %{{.*}}) [ "DIR.OMP.END.GENERICLOOP"() {{.*}}
; CHECK: call void @llvm.directive.region.exit(token %{{.*}}) [ "DIR.OMP.END.PARALLEL.LOOP"() {{.*}}

  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.GENERICLOOP"() ]
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.ub) #1
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.lb) #1
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.iv) #1
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #2

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #2

attributes #0 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }
attributes #2 = { argmemonly nocallback nofree nosync nounwind willreturn }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3}

!0 = !{i32 0, i32 53, i32 -1935601455, !"_Z3foo", i32 3, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 51}
!3 = !{i32 7, !"uwtable", i32 2}
!5 = !{!6, !6, i64 0}
!6 = !{!"int", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}
!9 = !{!10, !6, i64 0}
!10 = !{!"array@_ZTSA1000_i", !6, i64 0}
