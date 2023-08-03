; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:
;
; int fp[10];
; void foo() {
; #pragma omp parallel firstprivate(fp)
;   ;
; }

; CHECK: call void @llvm.memcpy.p0.p0.i64(ptr align 4 {{[^,]+}},
; CHECK-SAME: ptr align 4 {{[^,]+}}, i64 40, i1 false)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@fp = dso_local global [10 x i32] zeroinitializer, align 16

define dso_local void @foo() {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr @fp, i32 0, i64 10) ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]

  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)
