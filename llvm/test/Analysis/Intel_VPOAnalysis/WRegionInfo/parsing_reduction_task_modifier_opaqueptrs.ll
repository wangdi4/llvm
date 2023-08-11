; REQUIRES: asserts
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-wrncollection -analyze -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,print<vpo-wrncollection>)' -S %s 2>&1 | FileCheck %s

; This test checks if reduction(task) is parsed correctly on parallel construct.

; Test src:
;
;void foo() {
;  int x=0;
;  #pragma omp parallel reduction(task, + : x)
;  {
;    x++;
;  }
;}

; CHECK: REDUCTION clause (size=1): (TASK, ADD: TYPED(ptr %x, TYPE: i32, NUM_ELEMENTS: i32 1))

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo() {
entry:
  %x = alloca i32, align 4
  store i32 0, ptr %x, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.REDUCTION.ADD:TASK.TYPED"(ptr %x, i32 0, i32 1) ]

  %1 = load i32, ptr %x, align 4
  %inc = add nsw i32 %1, 1
  store i32 %inc, ptr %x, align 4
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  ret void
}

declare token @llvm.directive.region.entry() 

declare void @llvm.directive.region.exit(token)

