; RUN: opt < %s -analyze -hir-scc-formation -enable-new-pm=0 | FileCheck %s
; RUN: opt < %s -passes="print<hir-scc-formation>" 2>&1 -disable-output | FileCheck %s

; Check formation of one SCC with %conv3 and %ans.012.
; We were previously giving up on %conv3 as it is a cast inst with multiple uses
; as cast instructions can cause type mismatch. The logic now allows multiple
; use casts but gives up later on during validation if the instructions are not
; removed as 'intermediate' insts and can cause type mismatch.


; CHECK: SCC1: %conv3 -> %ans.012
; CHECK-NOT: SCC2

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define float @do_comp(i32 %n) {
entry:
  %arr = alloca [200 x float], align 16
  %0 = bitcast [200 x float]* %arr to i8*
  %cmp10 = icmp sgt i32 %n, 0
  br i1 %cmp10, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  %wide.trip.count14 = zext i32 %n to i64
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %ans.012 = phi float [ 0.000000e+00, %for.body.preheader ], [ %conv3, %for.body ]
  %arrayidx = getelementptr inbounds [200 x float], [200 x float]* %arr, i64 0, i64 %indvars.iv
  %1 = load float, float* %arrayidx, align 4
  %conv = fpext float %1 to double
  %conv1 = fpext float %ans.012 to double
  %add = fadd fast double %conv1, 1.000000e+00
  %add2 = fadd fast double %add, %conv
  %conv3 = fptrunc double %add2 to float
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count14
  br i1 %exitcond.not, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  %conv3.lcssa = phi float [ %conv3, %for.body ]
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  %ans.0.lcssa = phi float [ 0.000000e+00, %entry ], [ %conv3.lcssa, %for.end.loopexit ]
  ret float %ans.0.lcssa
}

