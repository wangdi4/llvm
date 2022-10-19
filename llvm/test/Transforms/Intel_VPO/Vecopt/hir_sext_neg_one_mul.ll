; RUN: opt -passes='hir-ssa-deconstruction,print<hir>,hir-vec-dir-insert,hir-vplan-vec,print<hir>' -vplan-force-vf=4 -hir-details -disable-output < %s 2>&1 | FileCheck %s

; Verify that the multiplication (-1 * sext.i8.i32(%t)) is performed by
; vectorizer in 32 bits.

; Previously it was happening in 8 bits, something like this-
; %.vec1 = -1 * %t;

; <RVAL-REG> LINEAR sext.<4 x i8>.<4 x i32>(-1 * %t)

; Dump Before-

; CHECK: + DO i64 i1 = 0, 99, 1   <DO_LOOP>
; CHECK: |   %0 = (%in)[i1];
; CHECK: |   (%out)[i1] = (((-1 * sext.i8.i32(%t)) + %0) * %t1);
; CHECK: + END LOOP


; Dump After-

; CHECK: + DO i64 i1 = 0, 99, 4   <DO_LOOP> <auto-vectorized> <novectorize>
; CHECK: |   %.vec = (<4 x i32>*)(%in)[i1];
; CHECK: |   %.vec1 = %t  *  -1;
; CHECK: |   <LVAL-REG> NON-LINEAR <4 x i32> %.vec1
; CHECK: |   <RVAL-REG> LINEAR sext.<4 x i8>.<4 x i32>(%t)
; CHECK: |
; CHECK: |   %.vec2 = %t1  *  %.vec + %.vec1;
; CHECK: |   (<4 x i32>*)(%out)[i1] = %.vec2;
; CHECK: + END LOOP


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(i32* noalias %out, i32* noalias %in, i8 %t, i32 %t1) {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds i32, i32* %in, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 1
  %conv = sext i8 %t to i32
  %mul = sub i32 0, %conv
  %add = add i32 %0, %mul
  %mul1 = mul i32 %t1, %add
  %arrayidx2 = getelementptr inbounds i32, i32* %out, i64 %indvars.iv
  store i32 %mul1, i32* %arrayidx2, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

