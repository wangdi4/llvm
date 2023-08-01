; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -instcombine -S %s | FileCheck %s
; RUN: opt -opaque-pointers=0 -passes='function(instcombine)' -S %s | FileCheck %s
;
; // Test C source:
; void bar(int*);
; void foo()
; {
;     int *a = (int *)malloc(sizeof(int));
;     #pragma omp target
;     {}
;     bar(a);
; }
;
; Check that instcombine does not move the bitcast into the TARGET region.
; Before the fix, instcombine moved the bitcast from "entry" to the
; top of "split" to be closer to its use. As a result, it landed inside
; the TARGET region, corrupting the OpenMP semantics.
;
; Check that there is no bitcast in the "split" BB:
; CHECK: split:
; CHECK-NOT: bitcast i8* %call to i32*

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

define dso_local void @foo() local_unnamed_addr #0 {
entry:
  %call = call i8* @malloc(i64 4)
  %0 = bitcast i8* %call to i32*
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0) ]
  br label %split

split:                           ; preds = %entry
  ; make sure instcombine does not move the bitcast instruction here
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET"() ]
  call void @bar(i32* %0)
  ret void
}

declare dso_local noalias i8* @malloc(i64) local_unnamed_addr
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare dso_local void @bar(i32*) local_unnamed_addr

attributes #0 = { "may-have-openmp-directive"="true" }

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 57, i32 -698066943, !"foo", i32 5, i32 0, i32 0}
