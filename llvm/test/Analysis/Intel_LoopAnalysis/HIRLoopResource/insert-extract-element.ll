; RUN: opt -passes="hir-ssa-deconstruction,print<hir-loop-resource>" -disable-output < %s 2>&1 | FileCheck %s

; Verify that we can successfully compute loop resource for loop containing
; insertelement/extractelement instructions.

; + DO i1 = 0, 3, 1   <DO_LOOP>
; |   %extract = extractelement %in.vec,  i1;
; |   (%ptr)[i1] = %extract;
; |   %res.vec.phi = insertelement %res.vec.phi,  %extract,  i1;
; + END LOOP

; CHECK: + DO i1 = 0, 3, 1   <DO_LOOP>
; CHECK:    Integer Operations: 4
; CHECK:    Integer Operations Cost: 6
; CHECK:    Integer Memory Writes: 1
; CHECK:    Memory Operations Cost: 4
; CHECK:    Total Cost: 10
; CHECK:    Integer Bound
; CHECK: + END LOOP

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(ptr %ptr, <4 x i32> %in.vec, <4 x i32> %res.vec) {
for.body.lr.ph:                                   ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %res.vec.phi = phi <4 x i32> [ %res.vec, %for.body.lr.ph ], [ %res.vec1, %for.body ]
  %extract = extractelement <4 x i32> %in.vec, i64 %indvars.iv
  %arrayidx = getelementptr inbounds i32, ptr %ptr, i64 %indvars.iv
  store i32 %extract, ptr %arrayidx, align 8
  %res.vec1 = insertelement <4 x i32> %res.vec.phi, i32 %extract, i64 %indvars.iv 
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 4
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

