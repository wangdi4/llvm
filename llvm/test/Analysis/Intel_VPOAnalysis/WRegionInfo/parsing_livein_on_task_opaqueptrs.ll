; REQUIRES: asserts
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-wrncollection -analyze -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,print<vpo-wrncollection>)' -S %s 2>&1 | FileCheck %s
;
; Check parsing of the 'livein' clause on 'task'
; The test IR is hand-modified by adding QUAL.OMP.LIVEIN
;
; Test src:
;
; void bar();
; void foo() {
; int x;
; #pragma omp task 
;   bar();
; }

; CHECK: BEGIN TASK ID=1 {
; CHECK:  LIVEIN clause (size=1): (ptr %x)
; CHECK:} END TASK ID=1

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo() {
entry:
  %x = alloca i32, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(),
    "QUAL.OMP.LIVEIN"(ptr %x) ]

  call void (...) @bar()
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TASK"() ]
  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare dso_local void @bar(...)
