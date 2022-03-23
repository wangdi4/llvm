; RUN: opt -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -simplifycfg -switch-to-offload -vpo-paropt-gpu-execution-scheme=0 -S %s | FileCheck %s
; RUN: opt -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,loop-simplify,sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt,function(simplifycfg)' -switch-to-offload -vpo-paropt-gpu-execution-scheme=0 -S %s | FileCheck %s

; Original code:
; void foo() {
; #pragma omp target teams distribute num_teams(12) dist_schedule(static, 2)
;   for (int i = 0; i < 100; ++i);
; }

; CHECK: [[LOOPLBV:%.+]] = load i32, i32* [[LOOPLBP:%[A-Za-z0-9._]+lower.bnd]]
; CHECK: [[LOOPUBV:%.+]] = load i32, i32* [[LOOPUBP:%[A-Za-z0-9._]+upper.bnd]]
; CHECK: [[TMP1:%.+]] = call spir_func i64 @_Z14get_num_groupsj(i32 0)
; CHECK: [[NUMGROUPS:%.+]] = trunc i64 [[TMP1]] to i32
; CHECK: [[STRIDEV:%.+]] = mul i32 [[NUMGROUPS]], 2
; CHECK: store i32 [[STRIDEV]], i32* [[TEAMINCP:%[A-Za-z0-9._]+]]
; CHECK: [[TMP2:%.+]] = call spir_func i64 @_Z12get_group_idj(i32 0)
; CHECK: [[GROUPID:%.+]] = trunc i64 [[TMP2]] to i32
; CHECK: [[OFFSET:%.+]] = mul i32 [[GROUPID]], 2
; CHECK: [[TEAMLBV:%.+]] = add i32 [[LOOPLBV]], [[OFFSET]]
; CHECK: store i32 [[TEAMLBV]], i32* [[LOOPLBP]]
; CHECK: store i32 [[TEAMLBV]], i32* [[TEAMLBP:%[A-Za-z0-9._]+]]
; CHECK: [[TEAMUBV:%.+]] = add i32 [[TEAMLBV]], 1
; CHECK: [[MINUBPRED:%.+]] = icmp slt i32 [[TEAMUBV]], [[LOOPUBV]]
; CHECK: [[MINUBV:%.+]] = select i1 [[MINUBPRED]], i32 [[TEAMUBV]], i32 [[LOOPUBV]]
; CHECK: store i32 [[MINUBV]], i32* [[LOOPUBP:%[A-Za-z0-9._]+]]
; CHECK: store i32 [[MINUBV]], i32* [[TEAMUBP:%[A-Za-z0-9._]+]]
; CHECK: br label %[[TEAMDISPHDR:[A-Za-z0-9._]+]]

; CHECK: [[TEAMDISPHDR]]:
; CHECK: [[TMP3:%.+]] = load i32, i32* [[TEAMUBP]]
; CHECK: [[PRED1:%.+]] = icmp sle i32 [[TMP3]], [[NUBV:%[A-Za-z0-9._]+]]
; CHECK: [[UBMIN:%.+]] = select i1 [[PRED1]], i32 [[TMP3]], i32 [[NUBV]]
; CHECK: store i32 [[UBMIN]], i32* [[TEAMUBP]]
; CHECK: [[TMP4:%.+]] = load i32, i32* [[TEAMLBP]]
; CHECK: [[TMP5:%.+]] = load i32, i32* [[TEAMUBP]]
; CHECK: [[PRED2:%.+]] = icmp sle i32 [[TMP4]], [[TMP5]]
; CHECK: br i1 [[PRED2]], label %[[TEAMDISPINNERBODY:[A-Za-z0-9._]+]], label %[[TEAMDISPLATCH:[A-Za-z0-9._]+]]

; CHECK: [[TEAMDISPINNERBODY]]:
; CHECK: [[TMP6:%.+]] = load i32, i32* [[TEAMLBP]]
; CHECK: store i32 [[TMP6]], i32* [[LOOPLBP]]
; CHECK: [[TMP7:%.+]] = load i32, i32* [[TEAMUBP]]
; CHECK: store i32 [[TMP7]], i32* [[LOOPUBP]]
; CHECK: [[TMP8:%.+]] = load i32, i32* [[LOOPLBP]]
; CHECK: [[TMP9:%.+]] = load i32, i32* [[LOOPUBP]]
; CHECK: [[PRED3:%.+]] = icmp sle i32 [[TMP8]], [[TMP9]]
; CHECK: br i1 [[PRED3]], label %[[INNERFORBODY:[A-Za-z0-9._]+]], label %[[TEAMDISPINC:[A-Za-z0-9._]+]]

; CHECK: [[INNERFORBODY]]:
; CHECK: br i1{{.*}}, label %[[INNERFORBODY]], label %[[SPLITBB:[A-Za-z0-9._]+]]

; CHECK: [[TEAMDISPINC]]:
; CHECK: [[TEAMINCV:%.+]] = load i32, i32* [[TEAMINCP]]
; CHECK: [[NEWTEAMLBV:%.+]] = add i32 [[TMP4]], [[TEAMINCV]]
; CHECK: [[NEWTEAMUBV:%.+]] = add i32 [[TMP5]], [[TEAMINCV]]
; CHECK: store i32 [[NEWTEAMLBV]], i32* [[TEAMLBP]]
; CHECK: store i32 [[NEWTEAMUBV]], i32* [[TEAMUBP]]
; CHECK: br label %[[TEAMDISPHDR]]

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: noinline nounwind
define hidden spir_func void @foo() #0 {
entry:
  %.omp.lb = alloca i32, align 4
  %.omp.lb.ascast = addrspacecast i32* %.omp.lb to i32 addrspace(4)*
  %.omp.ub = alloca i32, align 4
  %.omp.ub.ascast = addrspacecast i32* %.omp.ub to i32 addrspace(4)*
  %tmp = alloca i32, align 4
  %tmp.ascast = addrspacecast i32* %tmp to i32 addrspace(4)*
  %.omp.iv = alloca i32, align 4
  %.omp.iv.ascast = addrspacecast i32* %.omp.iv to i32 addrspace(4)*
  %i = alloca i32, align 4
  %i.ascast = addrspacecast i32* %i to i32 addrspace(4)*
  store i32 0, i32 addrspace(4)* %.omp.lb.ascast, align 4
  store i32 99, i32 addrspace(4)* %.omp.ub.ascast, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast) ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.NUM_TEAMS"(i32 12), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.SHARED"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.SHARED"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast) ]
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE"(), "QUAL.OMP.DIST_SCHEDULE.STATIC"(i32 2), "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast) ]
  %3 = load i32, i32 addrspace(4)* %.omp.lb.ascast, align 4
  store i32 %3, i32 addrspace(4)* %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %4 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %5 = load i32, i32 addrspace(4)* %.omp.ub.ascast, align 4
  %cmp = icmp sle i32 %4, %5
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %6 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %mul = mul nsw i32 %6, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32 addrspace(4)* %i.ascast, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %7 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %add1 = add nsw i32 %7, 1
  store i32 %add1, i32 addrspace(4)* %.omp.iv.ascast, align 4
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

attributes #0 = { noinline nounwind "contains-openmp-target"="true" "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2}
!opencl.used.extensions = !{!3}
!opencl.used.optional.core.features = !{!3}
!opencl.compiler.options = !{!3}
!llvm.ident = !{!4}

!0 = !{i32 0, i32 2054, i32 116134417, !"_Z3foo", i32 2, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"PIC Level", i32 2}
!3 = !{}
!4 = !{!"clang version 10.0.0"}
