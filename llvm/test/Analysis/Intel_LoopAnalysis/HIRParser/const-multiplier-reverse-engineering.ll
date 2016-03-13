; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Check parsing output for the loop verifying that the constant multipler case is handled correctly while reverse engineering SCEV for the subscript index.
; CHECK: DO i1 = 0, zext.i32.i64((-1 + %size_y)), 1   <DO_LOOP>
; CHECK-NEXT: if (undef #UNDEF# undef)
; CHECK-NEXT: {
; CHECK-NEXT: %1 = trunc.i64.i32(undef * i1);
; CHECK-NEXT: DO i2 = 0, zext.i32.i64((-1 + %size_x)), 1   <DO_LOOP>
; CHECK-NEXT: %tmp16.0.copyload = (i16*)(%buf)[2 * i2 + sext.i32.i64((2 * %1))];
; CHECK-NEXT: END LOOP
; CHECK-NEXT: }
; CHECK-NEXT: END LOOP


; ModuleID = 'bugpoint-reduced-simplified.bc'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @buf2img(i8* nocapture readonly %buf, i32 %size_x, i32 %size_y) {
entry:
  br label %for.cond.22.preheader

for.cond.22.preheader:                            ; preds = %for.inc.39, %entry
  %indvars.iv219 = phi i64 [ 0, %entry ], [ %indvars.iv.next220, %for.inc.39 ]
  br i1 undef, label %for.body.25.lr.ph, label %for.inc.39

for.body.25.lr.ph:                                ; preds = %for.cond.22.preheader
  %0 = mul nsw i64 %indvars.iv219, undef
  %1 = trunc i64 %0 to i32
  br label %for.body.25

for.body.25:                                      ; preds = %for.body.25, %for.body.25.lr.ph
  %indvars.iv213 = phi i64 [ 0, %for.body.25.lr.ph ], [ %indvars.iv.next214, %for.body.25 ]
  %i.1183 = phi i32 [ 0, %for.body.25.lr.ph ], [ %inc37, %for.body.25 ]
  %add27 = add nsw i32 %i.1183, %1
  %mul28 = shl nsw i32 %add27, 1
  %idx.ext = sext i32 %mul28 to i64
  %add.ptr = getelementptr inbounds i8, i8* %buf, i64 %idx.ext
  %tmp16.0.add.ptr.sroa_cast = bitcast i8* %add.ptr to i16*
  %tmp16.0.copyload = load i16, i16* %tmp16.0.add.ptr.sroa_cast, align 1
  %indvars.iv.next214 = add nuw nsw i64 %indvars.iv213, 1
  %inc37 = add nuw nsw i32 %i.1183, 1
  %lftr.wideiv217 = trunc i64 %indvars.iv.next214 to i32
  %exitcond218 = icmp eq i32 %lftr.wideiv217, %size_x
  br i1 %exitcond218, label %for.inc.39.loopexit, label %for.body.25

for.inc.39.loopexit:                              ; preds = %for.body.25
  br label %for.inc.39

for.inc.39:                                       ; preds = %for.inc.39.loopexit, %for.cond.22.preheader
  %indvars.iv.next220 = add nuw nsw i64 %indvars.iv219, 1
  %lftr.wideiv222 = trunc i64 %indvars.iv.next220 to i32
  %exitcond223 = icmp eq i32 %lftr.wideiv222, %size_y
  br i1 %exitcond223, label %for.end, label %for.cond.22.preheader

for.end:                                       ; preds = %for.inc.39
  ret void
}
