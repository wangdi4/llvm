; RUN: opt -vpo-paropt-loop-mapping-scheme=1 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-loop-collapse -S %s | FileCheck -check-prefix=NON-CONFM %s
; RUN: opt -vpo-paropt-loop-mapping-scheme=1 -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse)' -S %s | FileCheck -check-prefix=NON-CONFM %s
; RUN: opt -vpo-paropt-loop-mapping-scheme=0 -bugpoint-enable-legacy-pm -vpo-paropt-map-loop-bind-teams-to-distribute=false -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck -check-prefix=NON-CONFM %s
; RUN: opt -vpo-paropt-loop-mapping-scheme=0 -vpo-paropt-map-loop-bind-teams-to-distribute=false -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck -check-prefix=NON-CONFM %s
; RUN: opt -vpo-paropt-loop-mapping-scheme=0 -bugpoint-enable-legacy-pm -vpo-paropt-map-loop-bind-teams-to-distribute=true -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck -check-prefix=CONFM %s
; RUN: opt -vpo-paropt-loop-mapping-scheme=0 -vpo-paropt-map-loop-bind-teams-to-distribute=true -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck -check-prefix=CONFM %s

; Test src:
;
; int aaa[1000];
; void foo() {
; #pragma omp target teams
;   for (int i = 0; i < 1000; ++i) {
; #pragma omp loop
;     for (int j = 0; j < 100; j++) {
;       aaa[i] += i + j;
;     }
;   }
; }

; SCHEME0:
; This test checks "loop" construct mapping after prepare pass if binding rule is teams.
; For conforming behavior, loop is mapped to "distribute" and for non-conforming mapping is
; "distribute parallel for" which can be toggled using flag '-vpo-paropt-map-loop-bind-teams-to-distribute'

; SCHEME1: 
; The "loop" construct is mapped to "distribute parallel for" in this case because it does not have a
; "parallel" construct as its immediate child. For simplicity, reuse the NON-CONFM checker from SCHEME0.

; NON-CONFM-NOT: call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(), {{.*}}
; NON-CONFM: call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(), "QUAL.OMP.SHARED:TYPED"({{.*}}), "QUAL.OMP.SHARED:TYPED"({{.*}}), "QUAL.OMP.NORMALIZED.IV:TYPED"({{.*}}), "QUAL.OMP.FIRSTPRIVATE:TYPED"({{.*}}), "QUAL.OMP.NORMALIZED.UB:TYPED"({{.*}}), "QUAL.OMP.PRIVATE:TYPED"({{.*}})
; NON-CONFM-NOT: call void @llvm.directive.region.exit(token %{{.*}}) [ "DIR.OMP.END.GENERICLOOP"() {{.*}}
; NON-CONFM: call void @llvm.directive.region.exit(token %{{.*}}) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() {{.*}}

; CONFM-NOT: call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(), {{.*}}
; CONFM: call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE"(), "QUAL.OMP.NORMALIZED.IV:TYPED"({{.*}}), "QUAL.OMP.FIRSTPRIVATE:TYPED"({{.*}}), "QUAL.OMP.NORMALIZED.UB:TYPED"({{.*}}), "QUAL.OMP.PRIVATE:TYPED"({{.*}})
; CONFM-NOT: call void @llvm.directive.region.exit(token %{{.*}}) [ "DIR.OMP.END.GENERICLOOP"() {{.*}}
; CONFM: call void @llvm.directive.region.exit(token %{{.*}}) [ "DIR.OMP.END.DISTRIBUTE"() {{.*}}

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
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr @aaa, ptr @aaa, i64 4000, i64 547, ptr null, ptr null),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %j, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp, i32 0, i32 1) ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.SHARED:TYPED"(ptr @aaa, i32 0, i64 1000),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %j, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp, i32 0, i32 1) ]
  call void @llvm.lifetime.start.p0(i64 4, ptr %i) #1
  store i32 0, ptr %i, align 4, !tbaa !5
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %2 = load i32, ptr %i, align 4, !tbaa !5
  %cmp = icmp slt i32 %2, 1000
  br i1 %cmp, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  call void @llvm.lifetime.end.p0(i64 4, ptr %i) #1
  br label %for.end

for.body:                                         ; preds = %for.cond
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.iv) #1
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.lb) #1
  store i32 0, ptr %.omp.lb, align 4, !tbaa !5
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.ub) #1
  store i32 99, ptr %.omp.ub, align 4, !tbaa !5

  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(),
    "QUAL.OMP.SHARED:TYPED"(ptr @aaa, i32 0, i64 1000),
    "QUAL.OMP.SHARED:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %j, i32 0, i32 1) ]
  %4 = load i32, ptr %.omp.lb, align 4, !tbaa !5
  store i32 %4, ptr %.omp.iv, align 4, !tbaa !5
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %for.body
  %5 = load i32, ptr %.omp.iv, align 4, !tbaa !5
  %6 = load i32, ptr %.omp.ub, align 4, !tbaa !5
  %cmp1 = icmp sle i32 %5, %6
  br i1 %cmp1, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  call void @llvm.lifetime.start.p0(i64 4, ptr %j) #1
  %7 = load i32, ptr %.omp.iv, align 4, !tbaa !5
  %mul = mul nsw i32 %7, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %j, align 4, !tbaa !5
  %8 = load i32, ptr %i, align 4, !tbaa !5
  %9 = load i32, ptr %j, align 4, !tbaa !5
  %add2 = add nsw i32 %8, %9
  %10 = load i32, ptr %i, align 4, !tbaa !5
  %idxprom = sext i32 %10 to i64
  %arrayidx = getelementptr inbounds [1000 x i32], ptr @aaa, i64 0, i64 %idxprom, !intel-tbaa !9
  %11 = load i32, ptr %arrayidx, align 4, !tbaa !9
  %add3 = add nsw i32 %11, %add2
  store i32 %add3, ptr %arrayidx, align 4, !tbaa !9
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  call void @llvm.lifetime.end.p0(i64 4, ptr %j) #1
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %12 = load i32, ptr %.omp.iv, align 4, !tbaa !5
  %add4 = add nsw i32 %12, 1
  store i32 %add4, ptr %.omp.iv, align 4, !tbaa !5
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end

  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.GENERICLOOP"() ]
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.ub) #1
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.lb) #1
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.iv) #1
  br label %for.inc

for.inc:                                          ; preds = %omp.loop.exit
  %13 = load i32, ptr %i, align 4, !tbaa !5
  %inc = add nsw i32 %13, 1
  store i32 %inc, ptr %i, align 4, !tbaa !5
  br label %for.cond, !llvm.loop !11

for.end:                                          ; preds = %for.cond.cleanup
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]
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

!0 = !{i32 0, i32 53, i32 -1942391772, !"_Z3foo", i32 3, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 51}
!3 = !{i32 7, !"uwtable", i32 2}
!5 = !{!6, !6, i64 0}
!6 = !{!"int", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}
!9 = !{!10, !6, i64 0}
!10 = !{!"array@_ZTSA1000_i", !6, i64 0}
!11 = distinct !{!11, !12}
!12 = !{!"llvm.loop.mustprogress"}
