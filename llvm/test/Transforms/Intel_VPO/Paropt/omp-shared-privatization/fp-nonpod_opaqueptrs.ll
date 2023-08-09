; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-shared-privatization -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-shared-privatization)' -S %s 2>&1 | FileCheck %s

; Test src:
;
; struct S {
;   int x;
;   S(): x(111) {};
; };
; S s;
;
; void f1() {
; #pragma omp parallel firstprivate(s)
;   ;
; }

; The test IR is a reduced version of the above test.

; Make sure that the firstprivate clause on @s is not changed to private
; because it's a nonpod.

; CHECK: "QUAL.OMP.FIRSTPRIVATE:NONPOD.TYPED"(ptr @s, %struct.S zeroinitializer, i32 1, ptr null, ptr null)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.S = type { i32 }
@s = dso_local global %struct.S zeroinitializer, align 4

define dso_local void @_Z2f1v() {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.FIRSTPRIVATE:NONPOD.TYPED"(ptr @s, %struct.S zeroinitializer, i32 1, ptr null, ptr null) ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]

  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
