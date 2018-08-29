target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; This test checks that the reduction initializer appears before the outer loop and
; and the last value computation instructions appear after the outer loop. It also
; checks to see if the inner loop is completely unrolled.
;
;int name(uint8_t *pix1, int i_stride_pix1, uint8_t *pix2, int i_stride_pix2) {
;  int i_sum = 0;
;  for (int y = 0; y < 8; y++) {
;    for (int x = 0; x < 16; x++) {
;      i_sum += abs(pix1[x] - pix2[x]);
;    }
;    pix1 += i_stride_pix1;
;    pix2 += i_stride_pix2;
;  }
;  return i_sum;
;}

; RUN: opt -S -enable-nested-blob-vec -enable-blob-coeff-vec -vplan-disable-verification -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -VPlanDriverHIR -hir-cg -print-after=VPlanDriverHIR < %s 2>&1 | FileCheck %s

; CHECK: BEGIN REGION { modified }
; CHECK-NEXT: %result.vector = insertelement zeroinitializer, %i_sum.027, 0;
; CHECK-NEXT: <{{[0-9]+}}>
; CHECK-NEXT: + DO i1
; CHECK-NOT: + DO i2
; CHECK: + END LOOP
; CHECK-NEXT: <{{[0-9]+}}>
; CHECK-NEXT: %rdx.shuf = shufflevector %result.vector

; Function Attrs: norecurse nounwind readonly uwtable
define dso_local i32 @name(i8* nocapture readonly %pix1, i32 %i_stride_pix1, i8* nocapture readonly %pix2, i32 %i_stride_pix2) local_unnamed_addr #0 {
entry:
  %idx.ext = sext i32 %i_stride_pix1 to i64
  %idx.ext8 = sext i32 %i_stride_pix2 to i64
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond.cleanup3, %entry
  %y.028 = phi i32 [ 0, %entry ], [ %inc11, %for.cond.cleanup3 ]
  %i_sum.027 = phi i32 [ 0, %entry ], [ %add.lcssa, %for.cond.cleanup3 ]
  %pix1.addr.026 = phi i8* [ %pix1, %entry ], [ %add.ptr, %for.cond.cleanup3 ]
  %pix2.addr.025 = phi i8* [ %pix2, %entry ], [ %add.ptr9, %for.cond.cleanup3 ]
  br label %for.body4

for.cond.cleanup:                                 ; preds = %for.cond.cleanup3
  %add.lcssa.lcssa = phi i32 [ %add.lcssa, %for.cond.cleanup3 ]
  ret i32 %add.lcssa.lcssa

for.cond.cleanup3:                                ; preds = %for.body4
  %add.lcssa = phi i32 [ %add, %for.body4 ]
  %add.ptr = getelementptr inbounds i8, i8* %pix1.addr.026, i64 %idx.ext
  %add.ptr9 = getelementptr inbounds i8, i8* %pix2.addr.025, i64 %idx.ext8
  %inc11 = add nuw nsw i32 %y.028, 1
  %exitcond29 = icmp eq i32 %inc11, 8
  br i1 %exitcond29, label %for.cond.cleanup, label %for.cond1.preheader

for.body4:                                        ; preds = %for.body4, %for.cond1.preheader
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body4 ]
  %i_sum.123 = phi i32 [ %i_sum.027, %for.cond1.preheader ], [ %add, %for.body4 ]
  %arrayidx = getelementptr inbounds i8, i8* %pix1.addr.026, i64 %indvars.iv
  %0 = load i8, i8* %arrayidx, align 1
  %conv = zext i8 %0 to i32
  %arrayidx6 = getelementptr inbounds i8, i8* %pix2.addr.025, i64 %indvars.iv
  %1 = load i8, i8* %arrayidx6, align 1
  %conv7 = zext i8 %1 to i32
  %sub = sub nsw i32 %conv, %conv7
  %ispos = icmp sgt i32 %sub, -1
  %neg = sub nsw i32 0, %sub
  %2 = select i1 %ispos, i32 %sub, i32 %neg
  %add = add nsw i32 %2, %i_sum.123
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 16
  br i1 %exitcond, label %for.cond.cleanup3, label %for.body4
}

attributes #0 = { norecurse nounwind readonly uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
