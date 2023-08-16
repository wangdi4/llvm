; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s

; Check parsing output for the loop verifying that we trace though multiple bitcasts in the incoming IR.

; CHECK: + DO i1 = 0, %n + -1, 1   <DO_LOOP>
; CHECK: |   + DO i2 = 0, %n + -1, 1   <DO_LOOP>
; CHECK: |   |   %5 = (float*)(%ptr)[(%mul * %mul1) * i1 + %mul1 * i2];
; CHECK: |   |   (%stptr)[0] = %5;
; CHECK: |   + END LOOP
; CHECK: + END LOOP


; ModuleID = 'interchnage-test.bc'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.CCGElem = type opaque

define hidden void @ccgDM_getMinMax(ptr %ptr, ptr %stptr, i64 %mul, i64 %mul1, i64 %n) {
entry:
  br label %for.body46.lr.ph.us

for.body46.lr.ph.us:                              ; preds = %for.inc51.us, %entry
  %indvars.iv180.us = phi i64 [ 0, %entry ], [ %indvars.iv.next181.us, %for.inc51.us ]
  %0 = mul nsw i64 %indvars.iv180.us, %mul
  br label %for.body46.us

for.body46.us:                                    ; preds = %for.body46.us, %for.body46.lr.ph.us
  %indvars.iv.us = phi i64 [ 0, %for.body46.lr.ph.us ], [ %indvars.iv.next.us, %for.body46.us ]
  %1 = add nsw i64 %indvars.iv.us, %0
  %2 = mul nsw i64 %1, %mul1
  %add.ptr.i.i.i.us = getelementptr inbounds i8, ptr %ptr, i64 %2
  %3 = bitcast ptr %add.ptr.i.i.i.us to ptr
  %4 = bitcast ptr %3 to ptr
  %5 = load float, ptr %4, align 4
  store float %5, ptr %stptr, align 4
  %indvars.iv.next.us = add nuw nsw i64 %indvars.iv.us, 1
  %exitcond.us = icmp eq i64 %indvars.iv.next.us, %n
  br i1 %exitcond.us, label %for.inc51.us, label %for.body46.us

for.inc51.us:                                     ; preds = %for.body46.us
  %indvars.iv.next181.us = add nuw nsw i64 %indvars.iv180.us, 1
  %exitcond184.us = icmp eq i64 %indvars.iv.next181.us, %n
  br i1 %exitcond184.us, label %for.end53.loopexit.us, label %for.body46.lr.ph.us

for.end53.loopexit.us:                            ; preds = %for.inc51.us
  ret void
}
