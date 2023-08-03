; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src: codegen for the scope construct -- without nowait clause
;void foo()
;{
;  int x = 0;
;  int l = 0;
;  #pragma omp scope private(x) reduction(+:l)
;  {
;    x = 10;
;    ++l;
;  }
;}

; Note: The test IR is obtained from Frontend.

; CHECK: %l.red = alloca i32, align 4
; CHECK: %x.priv = alloca i32, align 4
; CHECK: call void @__kmpc_scope(
; CHECK: store i32 10, ptr %x.priv, align 4
; CHECK: call i32 @__kmpc_reduce(
; CHECK: call void @__kmpc_end_reduce(
; CHECK: call void @__kmpc_end_scope(
; CHECK: call void @__kmpc_barrier(

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo() {
entry:
  %x = alloca i32, align 4
  %l = alloca i32, align 4
  store i32 0, ptr %x, align 4
  store i32 0, ptr %l, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SCOPE"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %x, i32 0, i32 1),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr %l, i32 0, i32 1) ]

  store i32 10, ptr %x, align 4
  %1 = load i32, ptr %l, align 4
  %inc = add nsw i32 %1, 1
  store i32 %inc, ptr %l, align 4
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SCOPE"() ]

  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
