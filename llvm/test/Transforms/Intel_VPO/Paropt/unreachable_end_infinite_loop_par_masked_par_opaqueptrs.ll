; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:

; void a() {
; #pragma omp parallel
; #pragma omp master
; #pragma omp parallel
;   for (;;);
; }

; Make sure that we don't comp-fail during codegen due to code-extractor issues
; caused by the infinite loop in the innermost parallel region.
;
; The test was crashing because the IR for the outer region, after handling the
; inner regions, had an early return, breaking the single-entry-single-exit
; expectations:
;
;  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"() ]
;  %2 = call i32 @__kmpc_masked(...)
;  call void @a.DIR.OMP.PARALLEL.58.split()
;  ret void
;  unreachable
;  call void @__kmpc_end_masked(...)
;  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
;  ret void

; CHECK: call void {{.*}} @__kmpc_fork_call({{.*}})
; CHECK: call i32 @__kmpc_masked({{.*}})
; CHECK: call void {{.*}} @__kmpc_fork_call({{.*}})

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @a() {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"() ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.MASTER"() ]
  fence acquire
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"() ]
  br label %for.cond

for.cond:                                         ; preds = %for.cond, %entry
  br label %for.cond

dummy:                                            ; No predecessors!
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.PARALLEL"() ]
  fence release
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.MASTER"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
