; REQUIRES: asserts
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-wrncollection -analyze -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,print<vpo-wrncollection>)' -S %s 2>&1 | FileCheck %s

; This test checks if 'firstprivate' clause on 'scope' construct is parsed correctly.
; The source IR was hand modified because front end does not yet support the 'firstprivate' clause with 'scope' construct.

; Test src:
;
; void foo() {
;   int x = 2;
; #pragma omp scope firstprivate(x)
;   {}
; }

; CHECK: FIRSTPRIVATE clause (size=1): TYPED({{.*}}, TYPE: i32, NUM_ELEMENTS: i32 1)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo() {
entry:
  %x = alloca i32, align 4
  store i32 2, ptr %x, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SCOPE"(),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %x, i32 0, i32 1) ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SCOPE"() ]

  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
