; RUN: opt < %s -analyze -hir-creation | FileCheck %s

; Check that the post-dominating switch case (L3 bblock) is linked after the switch while creating lexical links.
; CHECK: switch
; CHECK: case
; CHECK-NEXT: goto L3
; CHECK-NEXT: case
; CHECK: L2:
; CHECK: goto L3
; CHECK: default
; CHECK: goto L2
; CHECK: L3:


; ModuleID = 'goto.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common global [1000 x [1000 x float]] zeroinitializer, align 16
@B = common global [1000 x [1000 x float]] zeroinitializer, align 16
@C = common global [1000 x [1000 x float]] zeroinitializer, align 16

define void @sub3(i64 %n) {
entry:
  br label %for.body

for.body:                                         ; preds = %L3, %entry
  %indvars.iv = phi i64 [ 1, %entry ], [ %indvars.iv.next, %L3 ]
  %0 = trunc i64 %indvars.iv to i32
  %conv = sitofp i32 %0 to float
  %arrayidx = getelementptr inbounds [1000 x [1000 x float]], [1000 x [1000 x float]]* @A, i64 0, i64 0, i64 %indvars.iv
  store float %conv, float* %arrayidx, align 4
  switch i32 %0, label %if.end.11 [
    i32 2, label %L3
    i32 1, label %L2
  ]

if.end.11:                                        ; preds = %for.body
  %add = fadd float %conv, 1.000000e+00
  store float %add, float* %arrayidx, align 4
  br label %L2

L2:                                               ; preds = %if.end.11, %for.body
  %1 = phi float [ %conv, %for.body ], [ %add, %if.end.11 ]
  %add16 = fadd float %1, 1.000000e+00
  store float %add16, float* %arrayidx, align 4
  br label %L3

L3:                                               ; preds = %L2, %for.body
  %2 = phi float [ %conv, %for.body ], [ %add16, %L2 ]
  %add19 = fadd float %2, 1.000000e+00
  store float %add19, float* %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1000
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %L3
  ret void
}
