; REQUIRES: asserts
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt -S %s -debug-only=WRegionUtils 2>&1 | FileCheck %s
; RUN: opt -passes="function(vpo-cfg-restructuring),vpo-paropt" -S %s -debug-only=WRegionUtils 2>&1 | FileCheck %s

; Test src:
;
; void foo(int n) {
;   short x[n];
;   #pragma omp parallel private(x)
;     ;
; }

; CHECK: collectNonPointerValuesToBeUsedInOutlinedRegion: Non-pointer values to be passed into the outlined region: 'i64 %n.copy '

; Test IR was hand-modified so that the alloca was done with %n,
; but the size in the typed clause uses %n.copy.

; Ensures that we don't mark %n, the size of the alloca instruction,
; as SHARED, since it's not needed for the private allocation of %vla.
; %n.copy, from the TYPED clause will be used for that.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(i32 noundef %n) {
entry:
  %n.copy = zext i32 %n to i64
  %vla = alloca i16, i32 %n, align 16

  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %vla, i16 0, i64 %n.copy) ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]

  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
