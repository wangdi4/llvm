; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-pre-vec-complete-unroll -print-before=hir-pre-vec-complete-unroll -print-after=hir-pre-vec-complete-unroll -disable-output 2>&1 < %s | FileCheck %s

; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-pre-vec-complete-unroll,print<hir>" -disable-output 2>&1 < %s | FileCheck %s

; Check that we do not replace constant array with constant values when bitcasted

; CHECK: Function:
;
; CHECK:  BEGIN REGION { }
; CHECK:       + DO i1 = 0, %n, 1   <DO_LOOP>
; CHECK:       |   + DO i2 = 0, 3, 1   <DO_LOOP>
; CHECK:       |   |   (float*)(%B)[i2] = (float*)(@A)[0][i2];
; CHECK:       |   + END LOOP
; CHECK:       + END LOOP
; CHECK:  END REGION
;
; CHECK: Function:
;
; CHECK:  BEGIN REGION { modified }
; CHECK:       + DO i1 = 0, %n, 1   <DO_LOOP>
; CHECK:       |   (float*)(%B)[0] = (float*)(@A)[0][0];
; CHECK:       |   (float*)(%B)[1] = (float*)(@A)[0][1];
; CHECK:       |   (float*)(%B)[2] = (float*)(@A)[0][2];
; CHECK:       |   (float*)(%B)[3] = (float*)(@A)[0][3];
; CHECK:       + END LOOP
; CHECK:  END REGION



target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = internal unnamed_addr constant [4 x i32] [i32 1, i32 2, i32 3, i32 4], align 16

; Function Attrs: norecurse nounwind uwtable
define void @foo(i32* nocapture %B, i64 %n) local_unnamed_addr #0 {
entry:
  br label %for.outer

for.outer:
  %j = phi i64 [ 0, %entry ], [ %j.next, %latch ]
  br label %for.body

for.body:                                         ; preds = %for.body, %for.outer
  %indvars.iv = phi i64 [ 0, %for.outer ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [4 x i32], [4 x i32]* @A, i64 0, i64 %indvars.iv
  %ld.ptr = bitcast i32* %arrayidx to float*
  %0 = load float, float* %ld.ptr, align 4
  %arrayidx2 = getelementptr inbounds i32, i32* %B, i64 %indvars.iv
  %st.ptr = bitcast i32* %arrayidx2 to float*
  store float %0, float* %st.ptr, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 4
  br i1 %exitcond, label %latch, label %for.body

latch:
  %j.next = add nuw nsw i64 %j, 1
  %cmp = icmp eq i64 %j, %n
  br i1 %cmp, label %for.end, label %for.outer

for.end:                                          ; preds = %for.body
  ret void
}

