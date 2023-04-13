; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -simplifycfg -S %s | FileCheck --check-prefixes=CHECK,CHECKPA %s
; RUN: opt -opaque-pointers=0 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt,function(simplifycfg)' -S %s | FileCheck --check-prefixes=CHECK,CHECKPA %s
; INTEL_CUSTOMIZATION
; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -simplifycfg -S %s | FileCheck --check-prefixes=CHECK,CHECKIV %s
; RUN: opt -opaque-pointers=0 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt,function(simplifycfg)' -S %s | FileCheck --check-prefixes=CHECK,CHECKIV %s
; end INTEL_CUSTOMIZATION
;
; This test checks that paropt pass adds parallel access metadata to loops with
; concurrent clause that have non-monotonic property.
;
; void test1(float *P) {
; #pragma omp teams distribute parallel for order(concurrent)
;   for (int I = 0 ; I < 100; I++)
;     P[I] = I;
; }

; CHECK: call void @__kmpc_dist_for_static_init_4
; CHECK: [[FP1:%.+]] = sitofp i32 %{{.+}} to float
; CHECK: store float [[FP1]]{{.*}}, !llvm.access.group ![[AG1:[0-9]+]]
; CHECK: br i1 %{{.+}}, label %{{.+}}, label %{{.+}}, !llvm.loop ![[ID1:[0-9]+]]

; CHECKPA-DAG: ![[AG1]] = distinct !{}
; CHECKPA-DAG: ![[PA1:[0-9]+]] = !{!"llvm.loop.parallel_accesses", ![[AG1]]}
; CHECKPA-DAG: ![[ID1]] = distinct !{![[ID1]]{{.*}}, ![[PA1]]{{[,\}]}}

; INTEL_CUSTOMIZATION
; CHECKIV-DAG: ![[IV1:[0-9]+]] = !{!"llvm.loop.vectorize.ivdep_loop", i32 0}
; CHECKIV-DAG: ![[ID1]] = distinct !{![[ID1]]{{.*}}, ![[IV1]]{{[,\}]}}
; end INTEL_CUSTOMIZATION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @test1(float* %P) {
entry:
  %P.addr = alloca float*, align 8
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %I = alloca i32, align 4
  store float* %P, float** %P.addr, align 8
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.SHARED"(float** %P.addr), "QUAL.OMP.PRIVATE"(i32* %.omp.iv), "QUAL.OMP.PRIVATE"(i32* %.omp.lb), "QUAL.OMP.PRIVATE"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %I), "QUAL.OMP.PRIVATE"(i32* %tmp) ]
  store i32 0, i32* %.omp.lb, align 4
  store i32 99, i32* %.omp.ub, align 4
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(), "QUAL.OMP.ORDER.CONCURRENT"(), "QUAL.OMP.SHARED"(float** %P.addr), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %I) ]
  %2 = load i32, i32* %.omp.lb, align 4
  store i32 %2, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %3 = load i32, i32* %.omp.iv, align 4
  %4 = load i32, i32* %.omp.ub, align 4
  %cmp = icmp sle i32 %3, %4
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %5 = load i32, i32* %.omp.iv, align 4
  %mul = mul nsw i32 %5, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %I, align 4
  %6 = load i32, i32* %I, align 4
  %conv = sitofp i32 %6 to float
  %7 = load float*, float** %P.addr, align 8
  %8 = load i32, i32* %I, align 4
  %idxprom = sext i32 %8 to i64
  %ptridx = getelementptr inbounds float, float* %7, i64 %idxprom
  store float %conv, float* %ptridx, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %9 = load i32, i32* %.omp.iv, align 4
  %add1 = add nsw i32 %9, 1
  store i32 %add1, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TEAMS"() ]
  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)
