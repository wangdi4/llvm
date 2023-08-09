; RUN: opt -bugpoint-enable-legacy-pm -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -simplifycfg -switch-to-offload -vpo-paropt-gpu-execution-scheme=0 -S %s | FileCheck %s
; RUN: opt -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,loop-simplify,sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt,function(simplifycfg)' -switch-to-offload -vpo-paropt-gpu-execution-scheme=0 -S %s | FileCheck %s

; Test src:
;
; void foo() {
; #pragma omp target teams distribute num_teams(12) dist_schedule(static, 2)
;   for (int i = 0; i < 100; ++i)
;     ;
; }

; CHECK: [[LOOPLBV:%.+]] = load i32, ptr [[LOOPLBP:%[A-Za-z0-9._]+lower.bnd]]
; CHECK: [[LOOPUBV:%.+]] = load i32, ptr [[LOOPUBP:%[A-Za-z0-9._]+upper.bnd]]
; CHECK: [[TMP1:%.+]] = call spir_func i64 @_Z14get_num_groupsj(i32 0)
; CHECK: [[NUMGROUPS:%.+]] = trunc i64 [[TMP1]] to i32
; CHECK: [[STRIDEV:%.+]] = mul i32 [[NUMGROUPS]], 2
; CHECK: store i32 [[STRIDEV]], ptr [[TEAMINCP:%[A-Za-z0-9._]+]]
; CHECK: [[TMP2:%.+]] = call spir_func i64 @_Z12get_group_idj(i32 0)
; CHECK: [[GROUPID:%.+]] = trunc i64 [[TMP2]] to i32
; CHECK: [[OFFSET:%.+]] = mul i32 [[GROUPID]], 2
; CHECK: [[TEAMLBV:%.+]] = add i32 [[LOOPLBV]], [[OFFSET]]
; CHECK: store i32 [[TEAMLBV]], ptr [[LOOPLBP]]
; CHECK: store i32 [[TEAMLBV]], ptr [[TEAMLBP:%[A-Za-z0-9._]+]]
; CHECK: [[TEAMUBV:%.+]] = add i32 [[TEAMLBV]], 1
; CHECK: [[MINUBPRED:%.+]] = icmp slt i32 [[TEAMUBV]], [[LOOPUBV]]
; CHECK: [[MINUBV:%.+]] = select i1 [[MINUBPRED]], i32 [[TEAMUBV]], i32 [[LOOPUBV]]
; CHECK: store i32 [[MINUBV]], ptr [[LOOPUBP:%[A-Za-z0-9._]+]]
; CHECK: store i32 [[MINUBV]], ptr [[TEAMUBP:%[A-Za-z0-9._]+]]
; CHECK: br label %[[TEAMDISPHDR:[A-Za-z0-9._]+]]

; CHECK: [[TEAMDISPHDR]]:
; CHECK: [[TMP3:%.+]] = load i32, ptr [[TEAMUBP]]
; CHECK: [[UBMIN:%.+]] = icmp sle i32 [[TMP3]], [[NUBV:%[A-Za-z0-9._]+]]
; CHECK: store i32 [[NUBV]], ptr [[TEAMUBP]]
; CHECK: [[TMP4:%.+]] = load i32, ptr [[TEAMLBP]]
; CHECK: [[TMP5:%.+]] = load i32, ptr [[TEAMUBP]]
; CHECK: [[PRED2:%.+]] = icmp sle i32 [[TMP4]], [[TMP5]]
; CHECK: br i1 [[PRED2]], label %[[TEAMDISPINNERBODY:[A-Za-z0-9._]+]], label %[[TEAMDISPLATCH:[A-Za-z0-9._]+]]

; CHECK: [[TEAMDISPINNERBODY]]:
; CHECK: [[TMP6:%.+]] = load i32, ptr [[TEAMLBP]]
; CHECK: store i32 [[TMP6]], ptr [[LOOPLBP]]
; CHECK: [[TMP7:%.+]] = load i32, ptr [[TEAMUBP]]
; CHECK: store i32 [[TMP7]], ptr [[LOOPUBP]]
; CHECK: [[TMP8:%.+]] = load i32, ptr [[LOOPLBP]]
; CHECK: [[TMP9:%.+]] = load i32, ptr [[LOOPUBP]]
; CHECK: [[PRED3:%.+]] = icmp sle i32 [[TMP8]], [[TMP9]]
; CHECK: br i1 [[PRED3]], label %[[INNERFORBODY:[A-Za-z0-9._]+]], label %[[TEAMDISPINC:[A-Za-z0-9._]+]]

; CHECK: [[INNERFORBODY]]:
; CHECK: br i1{{.*}}, label %[[INNERFORBODY]], label %[[SPLITBB:[A-Za-z0-9._]+]]

; CHECK: [[TEAMDISPINC]]:
; CHECK: [[TEAMINCV:%.+]] = load i32, ptr [[TEAMINCP]]
; CHECK: [[NEWTEAMLBV:%.+]] = add i32 [[TMP4]], [[TEAMINCV]]
; CHECK: [[NEWTEAMUBV:%.+]] = add i32 [[TMP5]], [[TEAMINCV]]
; CHECK: store i32 [[NEWTEAMLBV]], ptr [[TEAMLBP]]
; CHECK: store i32 [[NEWTEAMUBV]], ptr [[TEAMUBP]]
; CHECK: br label %[[TEAMDISPHDR]]

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: convergent noinline nounwind optnone
define protected spir_func void @foo() #0 {
entry:
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %i = alloca i32, align 4
  %.omp.lb.ascast = addrspacecast ptr %.omp.lb to ptr addrspace(4)
  %.omp.ub.ascast = addrspacecast ptr %.omp.ub to ptr addrspace(4)
  %tmp.ascast = addrspacecast ptr %tmp to ptr addrspace(4)
  %.omp.iv.ascast = addrspacecast ptr %.omp.iv to ptr addrspace(4)
  %i.ascast = addrspacecast ptr %i to ptr addrspace(4)
  store i32 0, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 99, ptr addrspace(4) %.omp.ub.ascast, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1) ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.NUM_TEAMS"(i32 12),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1) ]
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE"(),
    "QUAL.OMP.DIST_SCHEDULE.STATIC"(i32 2),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1) ]
  %3 = load i32, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 %3, ptr addrspace(4) %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %4 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %5 = load i32, ptr addrspace(4) %.omp.ub.ascast, align 4
  %cmp = icmp sle i32 %4, %5
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %6 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %mul = mul nsw i32 %6, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr addrspace(4) %i.ascast, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %7 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %add1 = add nsw i32 %7, 1
  store i32 %add1, ptr addrspace(4) %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.DISTRIBUTE"() ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { convergent noinline nounwind optnone "approx-func-fp-math"="true" "contains-openmp-target"="true" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4, !5}
!opencl.compiler.options = !{!6}

!0 = !{i32 0, i32 53, i32 -1916375442, !"_Z3foo", i32 2, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 51}
!3 = !{i32 7, !"openmp-device", i32 51}
!4 = !{i32 7, !"PIC Level", i32 2}
!5 = !{i32 7, !"frame-pointer", i32 2}
!6 = !{}
