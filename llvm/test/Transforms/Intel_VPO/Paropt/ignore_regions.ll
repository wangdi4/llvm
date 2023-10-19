; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-loop-collapse -vpo-paropt-prepare -vpo-paropt-ignore-regions-starting-with=1 -pass-remarks=openmp -S %s 2>&1 | FileCheck %s -check-prefix=IGNOREALL -check-prefix=ALL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-paropt-prepare)' -vpo-paropt-ignore-regions-starting-with=1 -pass-remarks=openmp -S %s 2>&1 | FileCheck %s -check-prefix=IGNOREALL -check-prefix=ALL

; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-loop-collapse -vpo-paropt-prepare -vpo-paropt-ignore-regions-up-to=14 -pass-remarks=openmp -S %s 2>&1 | FileCheck %s -check-prefix=IGNOREALL -check-prefix=ALL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-paropt-prepare)' -vpo-paropt-ignore-regions-up-to=14 -pass-remarks=openmp -S %s 2>&1 | FileCheck %s -check-prefix=IGNOREALL -check-prefix=ALL

; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-loop-collapse -vpo-paropt-prepare -vpo-paropt-ignore-regions-matching=4,8,9 -pass-remarks=openmp -S %s 2>&1 | FileCheck %s -check-prefix=IGNORERANGE -check-prefix=ALL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-paropt-prepare)' -vpo-paropt-ignore-regions-matching=4,8,9 -pass-remarks=openmp -S %s 2>&1 | FileCheck %s -check-prefix=IGNORERANGE -check-prefix=ALL

; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-loop-collapse -vpo-paropt-prepare -vpo-paropt-ignore-regions-starting-with=4 -vpo-paropt-ignore-regions-up-to=9 -pass-remarks=openmp -S %s 2>&1 | FileCheck %s -check-prefix=IGNORERANGE -check-prefix=ALL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-paropt-prepare)' -vpo-paropt-ignore-regions-starting-with=4 -vpo-paropt-ignore-regions-up-to=9 -pass-remarks=openmp -S %s 2>&1 | FileCheck %s -check-prefix=IGNORERANGE -check-prefix=ALL

; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-paropt-prepare),vpo-paropt,function(intel-ir-optreport-emitter)' -vpo-paropt-ignore-regions-starting-with=4 -vpo-paropt-ignore-regions-up-to=9 -intel-opt-report=medium -intel-opt-report-file=stdout -disable-output < %s | FileCheck %s --strict-whitespace --check-prefix=OPTREPORT

; Test src:
;
; extern void f1(int);
;
; void bar() {
; #pragma omp single
;   f1(1);
;
; #pragma omp critical
;   f1(2);
; }
;
; void foo() {
; #pragma omp parallel
;   f1(3);
;
; #pragma omp master
;   f1(4);
;
; #pragma omp barrier
;   f1(5);
; }
;
; void baz() {
; #pragma omp task
;   f1(6);
;
; #pragma omp target
;   f1(7);
; }

; IGNOREALL: remark: <unknown>:0:0: construct 3 (single) ignored at user's direction
; IGNORERANGE-NOT: remark: <unknown>:0:0: construct 3 (single) ignored at user's direction

; ALL: remark: <unknown>:0:0: construct 4 (critical) ignored at user's direction
; ALL: remark: <unknown>:0:0: construct 8 (parallel) ignored at user's direction
; ALL: remark: <unknown>:0:0: construct 9 (masked) ignored at user's direction

; IGNOREALL: remark: <unknown>:0:0: construct 10 (barrier) ignored at user's direction
; IGNORERANGE-NOT: remark: <unknown>:0:0: construct 10 (barrier) ignored at user's direction

; IGNOREALL: remark: <unknown>:0:0: construct 13 (task) ignored at user's direction
; IGNORERANGE-NOT: remark: <unknown>:0:0: construct 13 (task) ignored at user's direction

; IGNOREALL: remark: <unknown>:0:0: construct 14 (target) ignored at user's direction
; IGNORERANGE-NOT: remark: <unknown>:0:0: construct 14 (target) ignored at user's direction

; OPTREPORT: Global optimization report for : _Z3barv

; OPTREPORT-NOT:     remark #30009: Construct 3 (single) ignored at user's direction

; OPTREPORT: OMP CRITICAL BEGIN
; OPTREPORT:     remark #30009: Construct 4 (critical) ignored at user's direction
; OPTREPORT: OMP CRITICAL END

; OPTREPORT: Global optimization report for : _Z3foov

; OPTREPORT: OMP PARALLEL BEGIN
; OPTREPORT:     remark #30009: Construct 8 (parallel) ignored at user's direction
; OPTREPORT: OMP PARALLEL END

; OPTREPORT: OMP MASKED BEGIN
; OPTREPORT:     remark #30009: Construct 9 (masked) ignored at user's direction
; OPTREPORT: OMP MASKED END

; OPTREPORT-NOT:     remark #30009: Construct 10 (barrier) ignored at user's direction

; Note that the region numbers are like this because collapse pass and
; prepare pass, both build WRGraph, and each invocation increases the node
; counters. So, nodes are created in the order:
;   collapse(bar): 1, 2
;   prepare(bar):  3, 4     (removed in prepare)
;   collapse(foo): 5, 6, 7
;   prepare(foo):  8, 9, 10 (removed in prepare)
;   collapse(baz): 11, 12
;   prepare(baz):  13, 14   (removed in prepare)

; If the removal of regions was to be moved to the collapse pass, then the
; problem would be that the numbering would change depending on whether
; nodes were removed or not. For example, for the same test, if no nodes
; were removed, the numbering would be same as listed above. But with
; the flag -vpo-paropt-ignore-regions-starting-with=1, the numbering would
; be:
;
;   collapse(bar): 1, 2    (removed in collapse)
;   prepare(bar):  -       (no node left)
;   collapse(foo): 3, 4, 5 (problem: IDs without removal were 5, 6, 7) (removed here)
;   prepare(foo):  -
;   collapse(baz): 6, 7    (problem: IDs without removal were 11, 12)
;   prepare(baz):  -

; This variation in the numbering would make the flags to ignore regions
; based on region numbers unusable.

; In the future, if we want to improve this mechanism, one possible way
; would be to save the WRegion ID number into the IR (as metadate or
; operand-bundle) during the first build of WRGraph, and reuse it
; in subsequent builds for the same function. Or maybe the frontends
; could assign some unique IDs to all nodes in a module.

; IGNOREALL-NOT: call i32 @__kmpc_single
; IGNORERANGE: call i32 @__kmpc_single

; ALL-NOT: call void @__kmpc_critical
; ALL-NOT: call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(){{.*}}]
; ALL-NOT: call i32 @__kmpc_master

; IGNOREALL-NOT: call void @__kmpc_barrier
; IGNORERANGE: call void @__kmpc_barrier

; IGNOREALL-NOT: call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(){{.*}}]
; IGNORERANGE: call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(){{.*}}]

; IGNOREALL-NOT: call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(){{.*}}]
; IGNORERANGE: call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(){{.*}}]

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "x86_64"

$__clang_call_terminate = comdat any

@llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 0, ptr @.omp_offloading.requires_reg, ptr null }]

define dso_local void @_Z3barv() personality ptr @__gxx_personality_v0 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SINGLE"() ]
  fence acquire
  call void @_Z2f1i(i32 noundef 1)
  fence release
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SINGLE"() ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.CRITICAL"() ]
  fence acquire
  invoke void @_Z2f1i(i32 noundef 2)
          to label %invoke.cont unwind label %terminate.lpad

invoke.cont:                                      ; preds = %entry
  fence release
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.CRITICAL"() ]
  ret void

terminate.lpad:                                   ; preds = %entry
  %2 = landingpad { ptr, i32 }
          catch ptr null
  %3 = extractvalue { ptr, i32 } %2, 0
  call void @__clang_call_terminate(ptr %3)
  unreachable
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare dso_local void @_Z2f1i(i32 noundef)
declare dso_local i32 @__gxx_personality_v0(...)

define linkonce_odr hidden void @__clang_call_terminate(ptr %0) comdat {
  %2 = call ptr @__cxa_begin_catch(ptr %0)
  call void @_ZSt9terminatev()
  unreachable
}

declare dso_local ptr @__cxa_begin_catch(ptr)
declare dso_local void @_ZSt9terminatev()

define dso_local void @_Z3foov() personality ptr @__gxx_personality_v0 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"() ]
  call void @_Z2f1i(i32 noundef 3)
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.MASTER"() ]
  fence acquire
  invoke void @_Z2f1i(i32 noundef 4)
          to label %invoke.cont unwind label %terminate.lpad

invoke.cont:                                      ; preds = %entry
  fence release
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.MASTER"() ]
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.BARRIER"() ]
  fence acq_rel
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.BARRIER"() ]
  call void @_Z2f1i(i32 noundef 5)
  ret void

terminate.lpad:                                   ; preds = %entry
  %3 = landingpad { ptr, i32 }
          catch ptr null
  %4 = extractvalue { ptr, i32 } %3, 0
  call void @__clang_call_terminate(ptr %4)
  unreachable
}

define dso_local void @_Z3bazv() {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"() ]
  call void @_Z2f1i(i32 noundef 6)
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TASK"() ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0) ]
  call void @_Z2f1i(i32 noundef 7)
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

define internal void @.omp_offloading.requires_reg() section ".text.startup" {
entry:
  call void @__tgt_register_requires(i64 1)
  ret void
}

declare void @__tgt_register_requires(i64)

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 53, i32 -1923893656, !"_Z3bazv", i32 26, i32 0, i32 0}
