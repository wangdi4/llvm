; REQUIRES: asserts
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-wrncollection -analyze -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,print<vpo-wrncollection>)' -S %s 2>&1 | FileCheck %s

; This test checks the parsing support for the "primary" modifier on "proc_bind"
; source IR was hand modified because front end does not generate QUAL.OMP.PROC_BIND.PRIMARY yet

; Test src:
;
; #include <omp.h>
;  int main() {
; #pragma omp parallel proc_bind(primary)
;   {}
; }

; CHECK:   PROCBIND: PRIMARY

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local i32 @main() {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
     "QUAL.OMP.PROC_BIND.PRIMARY"() ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  ret i32 0
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)
