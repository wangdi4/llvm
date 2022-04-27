; RUN: opt -enable-new-pm=0 -vpo-cfg-restructuring -vpo-paropt-loop-collapse -vpo-paropt-prepare -vpo-paropt-ignore-regions-starting-with=1 -pass-remarks=openmp -S %s 2>&1 | FileCheck %s -check-prefix=IGNOREALL -check-prefix=ALL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-paropt-prepare)' -vpo-paropt-ignore-regions-starting-with=1 -pass-remarks=openmp -S %s 2>&1 | FileCheck %s -check-prefix=IGNOREALL -check-prefix=ALL

; RUN: opt -enable-new-pm=0 -vpo-cfg-restructuring -vpo-paropt-loop-collapse -vpo-paropt-prepare -vpo-paropt-ignore-regions-up-to=14 -pass-remarks=openmp -S %s 2>&1 | FileCheck %s -check-prefix=IGNOREALL -check-prefix=ALL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-paropt-prepare)' -vpo-paropt-ignore-regions-up-to=14 -pass-remarks=openmp -S %s 2>&1 | FileCheck %s -check-prefix=IGNOREALL -check-prefix=ALL

; RUN: opt -enable-new-pm=0 -vpo-cfg-restructuring -vpo-paropt-loop-collapse -vpo-paropt-prepare -vpo-paropt-ignore-regions-matching=4,8,9 -pass-remarks=openmp -S %s 2>&1 | FileCheck %s -check-prefix=IGNORERANGE -check-prefix=ALL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-paropt-prepare)' -vpo-paropt-ignore-regions-matching=4,8,9 -pass-remarks=openmp -S %s 2>&1 | FileCheck %s -check-prefix=IGNORERANGE -check-prefix=ALL

; RUN: opt -enable-new-pm=0 -vpo-cfg-restructuring -vpo-paropt-loop-collapse -vpo-paropt-prepare -vpo-paropt-ignore-regions-starting-with=4 -vpo-paropt-ignore-regions-up-to=9 -pass-remarks=openmp -S %s 2>&1 | FileCheck %s -check-prefix=IGNORERANGE -check-prefix=ALL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-paropt-prepare)' -vpo-paropt-ignore-regions-starting-with=4 -vpo-paropt-ignore-regions-up-to=9 -pass-remarks=openmp -S %s 2>&1 | FileCheck %s -check-prefix=IGNORERANGE -check-prefix=ALL

; Test src:

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

; IGNOREALL: remark: <unknown>:0:0: construct 3 (single) ignored on user's direction
; IGNORERANGE-NOT: remark: <unknown>:0:0: construct 3 (single) ignored on user's direction

; ALL: remark: <unknown>:0:0: construct 4 (critical) ignored on user's direction
; ALL: remark: <unknown>:0:0: construct 8 (parallel) ignored on user's direction
; ALL: remark: <unknown>:0:0: construct 9 (master) ignored on user's direction

; IGNOREALL: remark: <unknown>:0:0: construct 10 (barrier) ignored on user's direction
; IGNORERANGE-NOT: remark: <unknown>:0:0: construct 10 (barrier) ignored on user's direction

; IGNOREALL: remark: <unknown>:0:0: construct 13 (task) ignored on user's direction
; IGNORERANGE-NOT: remark: <unknown>:0:0: construct 13 (task) ignored on user's direction

; IGNOREALL: remark: <unknown>:0:0: construct 14 (target) ignored on user's direction
; IGNORERANGE-NOT: remark: <unknown>:0:0: construct 14 (target) ignored on user's direction

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

; Function Attrs: noinline nounwind optnone uwtable mustprogress
define dso_local void @_Z3barv() #0 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SINGLE"() ]
  fence acquire
  call void @_Z2f1i(i32 1) #1
  fence release
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SINGLE"() ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.CRITICAL"() ]
  fence acquire
  call void @_Z2f1i(i32 2)
  fence release
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.CRITICAL"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare dso_local void @_Z2f1i(i32) #2

; Function Attrs: noinline nounwind optnone uwtable mustprogress
define dso_local void @_Z3foov() #0 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"() ]
  call void @_Z2f1i(i32 3) #1
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.MASTER"() ]
  fence acquire
  call void @_Z2f1i(i32 4)
  fence release
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.MASTER"() ]
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.BARRIER"() ]
  fence acq_rel
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.BARRIER"() ]
  call void @_Z2f1i(i32 5)
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable mustprogress
define dso_local void @_Z3bazv() #0 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"() ]
  call void @_Z2f1i(i32 6) #1
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TASK"() ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0) ]
  call void @_Z2f1i(i32 7) #1
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

attributes #0 = { noinline nounwind optnone uwtable mustprogress "frame-pointer"="all" "may-have-openmp-directive"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nounwind }
attributes #2 = { "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}

!0 = !{i32 0, i32 66309, i32 99231124, !"_Z3bazv", i32 26, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
