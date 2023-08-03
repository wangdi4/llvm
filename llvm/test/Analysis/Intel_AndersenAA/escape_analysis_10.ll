; This test verifies that Escape analysis (in Andersens) shouldn't
; consider memset intrinsic as escape point.

; RUN: opt < %s -passes='require<anders-aa>,function(aa-eval)' -aa-pipeline=anders-aa -print-all-alias-modref-info -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -passes='require<anders-aa>,function(aa-eval)' -aa-pipeline=anders-aa -evaluate-loopcarried-alias -print-all-alias-modref-info -disable-output 2>&1 | FileCheck %s

; CHECK: NoAlias:     double* %0, double* %Ptr

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@sumdeijda = internal unnamed_addr global ptr null, align 8

define dso_local i32 @foo(ptr %Ptr) {
L0:
  %0 = load ptr, ptr @sumdeijda
  %1 = tail call noalias ptr @malloc(i64 500)
  store ptr %1, ptr bitcast (ptr @sumdeijda to ptr)
  %2 = bitcast ptr %1 to ptr
  br label %L2
L1:
  %3 = load ptr, ptr @sumdeijda
  br label %L2
L2:
  %4 = phi ptr [ %2, %L0 ], [ %3, %L1 ]
  %5 = bitcast ptr %4 to ptr

  ; dead loads, needed to get aa-eval to trigger
  %ld.0 = load double, ptr %0, align 8
  %ld.Ptr = load double, ptr %Ptr, align 8

  call void @llvm.memset.p0i8.i64(ptr %5, i8 1, i64 1024, i1 false)
  ret i32 0
}

declare dso_local noalias ptr @malloc(i64)
declare void @llvm.memset.p0i8.i64(ptr nocapture writeonly, i8, i64, i1 immarg)
