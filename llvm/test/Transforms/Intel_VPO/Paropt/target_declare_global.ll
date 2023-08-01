; Test that globals marked with "target_declare" are not optimized away by
;   "globalopt" (Global Variable Optimizer) or
;   "ipsccp" (Interprocedural Sparse Conditional Constant Propagation)

; RUN: opt -passes="globalopt" -S %s | FileCheck %s
; RUN: opt -passes="ipsccp" -S %s | FileCheck %s

; ModuleID = 'target_declare_global.cpp'
; test IR obtained with:   icx -Xclang -disable-llvm-passes -fiopenmp \
;                              -S -emit-llvm  target_declare_global.cpp
; // target_declare_global.cpp
; #pragma omp declare target
; static float var1;         // var1 must not be optimized away
; #pragma omp end declare target
; static float var2 = 2.0;   // var2 is expected to be optimized away
;
; void bar(float);
; void foo() {
;   var2 = 2.0;
;   bar(var2);
;   #pragma omp target
;     bar(3.0);
; }

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@_Z4var1 = internal target_declare global float 0.000000e+00, align 4
; CHECK: @_Z4var1 = {{.*}}target_declare
; verify that the target_declare variable var1 is not optimized away

@_Z4var2 = internal global float 2.000000e+00, align 4
; CHECK-NOT: @_Z4var2 =
; verify that the the regular static variable var2 is optimized away

define dso_local void @_Z3foov() {
entry:
  store float 2.000000e+00, ptr @_Z4var2, align 4
  %0 = load float, ptr @_Z4var2, align 4
  call void @_Z3barf(float %0)
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 1) ]
  call void @_Z3barf(float 3.000000e+00)
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

declare dso_local void @_Z3barf(float)
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

!omp_offload.info = !{!0, !1}
!0 = !{i32 0, i32 2051, i32 30414024, !"_Z3foov", i32 11, i32 1}
!1 = !{i32 1, !"_Z4var1", i32 0, i32 0}
