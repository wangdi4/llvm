; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -disable-output 2>&1 | FileCheck %s

; CHECK: BEGIN REGION { }
; CHECK:       + DO i1 = 0, %N + -1, 1   <DO_LOOP>
; CHECK:       |   (%A)[i1].0 = 5.000000e+00;
; CHECK:       + END LOOP
; CHECK: END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%complex_64bit = type { float, float }

declare ptr @llvm.intel.subscript.p0.i64(i8, i64, i64, ptr, i64)

define void @foo(i64 %SP, ptr %A, i64 %N) {
entry:
  br label %loop

loop:
  %iv = phi i64 [ 1, %entry ], [ %ivp, %loop ]
  %ptr = tail call ptr @llvm.intel.subscript.p0.i64(i8 0, i64 1, i64 %SP, ptr elementtype(%complex_64bit) %A, i64 %iv)
  %ptr1 = getelementptr inbounds %complex_64bit, ptr %ptr, i64 0, i32 0
  store float 5.000000e+00, ptr %ptr1
  %ivp = add nuw nsw i64 %iv, 1
  %cmp = icmp eq i64 %iv, %N
  br i1 %cmp, label %exit, label %loop

exit:
  ret void
}

