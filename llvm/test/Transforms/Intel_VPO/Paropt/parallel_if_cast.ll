; RUN: opt -bugpoint-enable-legacy-pm -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,loop-simplify,sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:
;
; #include <omp.h>
;
; void foo() {
; #pragma omp parallel if (!omp_in_parallel())
;  {}
; }

; This is actually a compfail test to verify that a boolean value
; passed to IF clause is properly casted to match the return type
; of __kmpc_ok_to_fork, because these two values need to be ANDed
; together.  Just check that the call to omp_in_parallel remains
; in the code.
; CHECK: call i32 @omp_in_parallel()

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo() {
entry:
  %call = call i32 @omp_in_parallel()
  %tobool = icmp ne i32 %call, 0
  %lnot = xor i1 %tobool, true
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.IF"(i1 %lnot) ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]

  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare dso_local i32 @omp_in_parallel()
