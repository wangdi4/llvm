; RUN: opt -bugpoint-enable-legacy-pm -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -simplifycfg -S %s | FileCheck %s
; RUN: opt -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,loop-simplify,sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt,function(simplifycfg)' -S %s | FileCheck %s

; Verify correctness of CFG and run-time calls.

; Test src:
;
; #define LOOP() for (int i = 0; i < 1000; ++i)
; void distribute_parallel_for_dist_schedule_schedule_dynamic() {
; #pragma omp distribute parallel for dist_schedule(static, 88)                  \
;     schedule(dynamic, 99)
;   LOOP();
; }

; CHECK-LABEL: @_Z54distribute_parallel_for_dist_schedule_schedule_dynamicv

; CHECK: call {{.*}} @__kmpc_fork_call({{.*}}@[[PARREG:_Z54distribute_parallel_for_dist_schedule_schedule_dynamicv.DIR.OMP.DISTRIBUTE.PARLOOP.[0-9]]]

; CHECK: define internal void @[[PARREG]]

; ZTT block:
; CHECK: %[[ZTT1:.+]] = icmp sle i32 %[[NLB:[^,]+]], %[[NUB:[^,]+]]
; CHECK: br i1 %[[ZTT1]], label %[[PHB:[^,]+]], label %[[EXIT:[^,]+]]

; PHB:
; CHECK: [[PHB]]:
; CHECK: call void @__kmpc_team_static_init_4({{.*}}, i32 %[[TID:[^,]+]], ptr %[[PTEAMISLAST:[^,]+]], ptr %[[PTEAMLB:[^,]+]], ptr %[[PTEAMUB:[^,]+]], ptr %[[PTEAMST:[^,]+]], i32 1, i32 88)
; CHECK: br label %[[TEAMDISPH:[^,]+]]

; TEAMDISPH:
; CHECK: [[TEAMDISPH]]:
; CHECK: %[[TEAMUBTMP:.+]] = load i32, ptr %[[PTEAMUB]]
; CHECK: %[[TEAMMINP:.+]] = icmp sle i32 %[[TEAMUBTMP]], %[[ORIGUB:[^,]+]]
; CHECK: br i1 %[[TEAMMINP]], label %[[TEAMDISPB:[^,]+]], label %[[TEAMDISPMINUB:[^,]+]]

; TEAMDISPMINUB:
; CHECK: [[TEAMDISPMINUB]]:
; CHECK: store i32 %[[ORIGUB]], ptr %[[PTEAMUB]]
; CHECK: br label %[[TEAMDISPB]]

; TEAMDISPB:
; CHECK: [[TEAMDISPB]]:
; CHECK: %[[TEAMLBNEW:.+]] = load i32, ptr %[[PTEAMLB]]
; CHECK: %[[TEAMUBNEW:.+]] = load i32, ptr %[[PTEAMUB]]
; CHECK: %[[TEAMZTT:.+]] = icmp sle i32 %[[TEAMLBNEW]], %[[TEAMUBNEW]]
; CHECK: br i1 %[[TEAMZTT]], label %[[TEAMDISPIB:[^,]+]], label %[[EXIT:[^,]+]]

; TEAMDISPIB:
; CHECK: [[TEAMDISPIB]]:
; 'schedule' 35 is kmp_sch_dynamic_chunked
; CHECK: store i32 %[[TEAMLBNEW]], ptr %[[PLB:[^,]+]]
; CHECK: store i32 %[[TEAMUBNEW]], ptr %[[PUB:[^,]+]]
; CHECK: %[[DISPLB:.+]] = load i32, ptr %[[PLB]]
; CHECK: %[[DISPUB:.+]] = load i32, ptr %[[PUB]]
; CHECK: call void @__kmpc_dispatch_init_4({{.*}}, i32 %[[TID]], i32 35, i32 %[[DISPLB]], i32 %[[DISPUB]], i32 1, i32 99)
; CHECK: br label %[[DISPNEXT:[^,]+]]

; DISPNEXT:
; CHECK: [[DISPNEXT]]:
; CHECK: %[[NEXT:.+]] = call i32 @__kmpc_dispatch_next_4({{.*}}, i32 %[[TID]], ptr %[[PISLAST:[^,]+]], ptr %[[PLB]], ptr %[[PUB]], ptr %[[PST:[^,]+]])
; CHECK: %[[NEXTP:.+]] = icmp ne i32 %[[NEXT]], 0
; CHECK: br i1 %[[NEXTP]], label %[[DISPB:[^,]+]], label %[[TEAMINC:[^,]+]]

; DISPB:
; CHECK: [[DISPB]]:
; CHECK: %[[LBNEW:.+]] = load i32, ptr %[[PLB]]
; CHECK: %[[UBNEW:.+]] = load i32, ptr %[[PUB]]
; CHECK: %[[ZTT2:.+]] = icmp sle i32 %[[LBNEW]], %[[UBNEW]]
; CHECK: br i1 %[[ZTT2]], label %[[LOOPBODY:[^,]+]], label %[[TEAMINC:[^,]+]]

; LOOPBODY:
; CHECK: [[LOOPBODY]]:
; CHECK: br i1 {{.*}}, label %[[LOOPBODY]], label %[[DISPNEXT:[^,]+]]

; TEAMINC:
; CHECK: [[TEAMINC]]:
; CHECK: br label %[[TEAMDISPH]]

; EXIT:
; CHECK: [[EXIT]]:

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress nounwind uwtable
define dso_local void @_Z54distribute_parallel_for_dist_schedule_schedule_dynamicv() #0 {
entry:
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.iv) #2
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.lb) #2
  store i32 0, ptr %.omp.lb, align 4, !tbaa !4
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.ub) #2
  store i32 999, ptr %.omp.ub, align 4, !tbaa !4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(),
    "QUAL.OMP.DIST_SCHEDULE.STATIC"(i32 88),
    "QUAL.OMP.SCHEDULE.DYNAMIC"(i32 99),
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
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  call void @llvm.lifetime.end.p0(i64 4, ptr %i) #2
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %5 = load i32, ptr %.omp.iv, align 4, !tbaa !4
  %add1 = add nsw i32 %5, 1
  store i32 %add1, ptr %.omp.iv, align 4, !tbaa !4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]
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

attributes #0 = { mustprogress nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { argmemonly nocallback nofree nosync nounwind willreturn }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!4 = !{!5, !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C++ TBAA"}
