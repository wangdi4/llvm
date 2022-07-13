; RUN: opt -enable-new-pm=0 -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Verify that vpo-parops works with bisect limit 0:
; RUN: opt -enable-new-pm=0 -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S -opt-bisect-limit=0 %s | FileCheck %s
; RUN: opt -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S -opt-bisect-limit=0 %s | FileCheck %s

; Verify correctness of CFG and run-time calls.

; Test src:
;
; #define LOOP() for (int i = 0; i < 1000; ++i)
; void distribute_parallel_for_schedule_static() {
; #pragma omp distribute parallel for schedule(static, 44)
;   LOOP();
; }

; CHECK-LABEL: @_Z39distribute_parallel_for_schedule_staticv

; CHECK: call {{.*}} @__kmpc_fork_call({{.*}}@[[PARREG:_Z39distribute_parallel_for_schedule_staticv.DIR.OMP.DISTRIBUTE.PARLOOP.[0-9]]]

; CHECK: define internal void @[[PARREG]]

; ZTT block:
; CHECK: %[[ZTT1:.+]] = icmp sle i32 %[[NLB:[^,]+]], %[[NUB:[^,]+]]
; CHECK: br i1 %[[ZTT1]], label %[[PHB:[^,]+]], label %[[EXIT:[^,]+]]

; PHB:
; 'schedule' 33 is kmp_sch_static_chunked
; CHECK: [[PHB]]:
; CHECK: call void @__kmpc_dist_for_static_init_4({{.*}}, i32 %[[TID:[^,]+]], i32 33, ptr %[[PISLAST:[^,]+]], ptr %[[PLB:[^,]+]], ptr %[[PUB:[^,]+]], ptr %[[PUD:[^,]+]], ptr %[[PST:[^,]+]], i32 1, i32 44)
; CHECK: %[[UD:.+]] = load i32, ptr %[[PUD]]
; CHECK: br label %[[DISPH:[^,]+]]

; DISPH:
; CHECK: [[DISPH]]:
; CHECK: %[[UBTMP:.+]] = load i32, ptr %[[PUB]]
; CHECK: %[[MINP:.+]] = icmp sle i32 %[[UBTMP]], %[[UD]]
; CHECK: br i1 %[[MINP]], label %[[DISPB:[^,]+]], label %[[DISPMINUB:[^,]+]]

; DISPMINUB:
; CHECK: [[DISPMINUB]]:
; CHECK: store i32 %[[UD]], ptr %[[PUB]]
; CHECK: br label %[[DISPB]]

; DISPB:
; CHECK: [[DISPB]]:
; CHECK: %[[LBNEW:.+]] = load i32, ptr %[[PLB]]
; CHECK: %[[UBNEW:.+]] = load i32, ptr %[[PUB]]
; CHECK: %[[ZTT2:.+]] = icmp sle i32 %[[LBNEW]], %[[UBNEW]]
; CHECK: br i1 %[[ZTT2]], label %[[LOOPBODY:[^,]+]], label %[[DISPLATCH:[^,]+]]

; LOOPBODY:
; CHECK: [[LOOPBODY]]:
; CHECK: br label %[[BODYCONT:[^,]+]]

; BODYCONT:
; CHECK: [[BODYCONT]]:
; CHECK: br label %[[LOOPINC:[^,]+]]

; LOOPINC:
; CHECK: [[LOOPINC]]:
; CHECK: br i1 {{.*}}, label %[[LOOPBODY]], label %[[DISPINC:[^,]+]]

; DISPINC:
; CHECK: [[DISPINC]]:
; CHECK: br label %[[DISPH]]

; DISPLATCH:
; CHECK: [[DISPLATCH]]:
; CHECK: br label %[[LOOPREGIONEXIT:[^,]+]]

; LOOPREGIONEXIT:
; CHECK: [[LOOPREGIONEXIT]]:
; CHECK: call void @__kmpc_for_static_fini({{.*}}, i32 %[[TID]])
; CHECK: br label %[[EXIT]]

; EXIT:
; CHECK: [[EXIT]]:

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress nounwind uwtable
define dso_local void @_Z39distribute_parallel_for_schedule_staticv() #0 {
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
    "QUAL.OMP.SCHEDULE.STATIC"(i32 44),
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
