; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; This test checks codegen for the 'scope' construct -- with 'firstprivate' clause.
; The source IR was hand modified because front end does not yet support the 'firstprivate' clause with 'scope' construct.

; Test src:

; void foo() {
;   int x = 2;
; #pragma omp parallel firstprivate(x)
;   {
;     x = 3;
;   }
; }

; CHECK: %x.fpriv = alloca i32, align 4
; CHECK: call void @__kmpc_scope(
; CHECK: store i32 3, ptr %x.fpriv, align 4
; CHECK: call void @__kmpc_end_scope(
; CHECK: call void @__kmpc_barrier(

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo() {
entry:
  %x = alloca i32, align 4
  store i32 2, ptr %x, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SCOPE"(),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %x, i32 0, i32 1) ]

  store i32 3, ptr %x, align 4
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SCOPE"() ]

  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
