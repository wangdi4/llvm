; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -verify -S %s | FileCheck --check-prefix=CHECK %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt,verify' -S %s | FileCheck --check-prefix=CHECK %s

; Test src:
;
; void foo(int n) {
;   int s[n];
; #pragma omp parallel reduction(+: s)
;   ;
; }

; CHECK: [[VLASIZEVAR:@[A-Za-z0-9_.]+]] = private thread_local global i64 0
; CHECK: define{{.*}}@foo_tree_reduce_2(ptr [[DST:%[A-Za-z0-9_.]+]], ptr [[SRC:%[A-Za-z0-9_.]+]]) {
; CHECK: [[VLASIZE:%[A-Za-z0-9_.]+]] = load i64, ptr [[VLASIZEVAR]]

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(i32 noundef %n) {
entry:
  %n.addr = alloca i32, align 4
  %saved_stack = alloca ptr, align 8
  %__vla_expr0 = alloca i64, align 8
  %omp.vla.tmp = alloca i64, align 8
  store i32 %n, ptr %n.addr, align 4
  %0 = load i32, ptr %n.addr, align 4
  %1 = zext i32 %0 to i64
  %2 = call ptr @llvm.stacksave()
  store ptr %2, ptr %saved_stack, align 8
  %vla = alloca i32, i64 %1, align 16
  store i64 %1, ptr %__vla_expr0, align 8
  store i64 %1, ptr %omp.vla.tmp, align 8
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr %vla, i32 0, i64 %1),
    "QUAL.OMP.SHARED:TYPED"(ptr %omp.vla.tmp, i64 0, i32 1) ]

  %4 = load i64, ptr %omp.vla.tmp, align 8
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.PARALLEL"() ]

  %5 = load ptr, ptr %saved_stack, align 8
  call void @llvm.stackrestore(ptr %5)
  ret void
}

declare ptr @llvm.stacksave()

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare void @llvm.stackrestore(ptr)
