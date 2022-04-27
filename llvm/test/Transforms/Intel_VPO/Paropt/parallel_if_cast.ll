; RUN: opt -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,loop-simplify,sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; This is actually a compfail test to verify that a boolean value
; passed to IF clause is properly casted to match the return type
; of __kmpc_ok_to_fork, because these two values need to be ANDed
; together.  Just check that the call to omp_in_parallel remains
; in the code.
; CHECK: call i32 @omp_in_parallel()

; Original code:
; #include <omp.h>
;
; void foo() {
; #pragma omp parallel if (!omp_in_parallel())
;  {}
; }

; ModuleID = 'repro.cc'
source_filename = "repro.cc"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: uwtable
define dso_local void @_Z3foov() #0 {
entry:
  %call = call i32 @omp_in_parallel()
  %tobool = icmp ne i32 %call, 0
  %lnot = xor i1 %tobool, true
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.IF"(i1 %lnot) ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare dso_local i32 @omp_in_parallel() #1

attributes #0 = { uwtable "may-have-openmp-directive"="true" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}
