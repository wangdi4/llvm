; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s


; Verify that we handle vector types in incoming IR.

; CHECK: + DO i1 = 0, 19, 1   <DO_LOOP>
; CHECK: |   %add.phi = %stride  +  %add.phi;
; CHECK: + END LOOP


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local <2 x float> @_Z3fooCf(<2 x float> %init, <2 x float> %stride) {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %add.phi = phi <2 x float> [ %init, %entry ], [ %add, %for.body ]
  %count.027 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %add = fadd fast <2 x float> %stride, %add.phi
  %inc = add nuw nsw i32 %count.027, 1
  %exitcond = icmp eq i32 %inc, 20
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  %retval = phi <2 x float> [ %add, %for.body ]
  ret <2 x float> %retval
}
