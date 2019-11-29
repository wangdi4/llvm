; RUN: opt -scoped-noalias -hir-ssa-deconstruction -disable-output -hir-runtime-dd -hir-vec-dir-insert -print-after=hir-vec-dir-insert -hir-details < %s 2>&1 | FileCheck %s

; HIR:
; BEGIN REGION { }
;      + DO i1 = 0, %i_height + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>
;      |   + DO i2 = 0, sext.i32.i64(%i_width) + -1, 1   <DO_LOOP>
;      |   |   %0 = (%src1)[sext.i32.i64(%i_src1_stride) * i1 + i2];
;      |   |   %1 = (%src2)[sext.i32.i64(%i_src2_stride) * i1 + i2];
;      |   |   (%dst)[sext.i32.i64(%i_dst_stride) * i1 + i2] = (zext.i8.i32(%1) + zext.i8.i32(%0) + 1)/u2;
;      |   + END LOOP
;      + END LOOP
; END REGION

; CHECK: After

; CHECK-DAG: = &((%src1)[sext.i32.i64(%i_width) + (sext.i32.i64((-1 + %i_height)) * smax(0, sext.i32.i64(%i_src1_stride))) + -1]) >=u &((%dst)[(sext.i32.i64((-1 + %i_height)) * smin(0, sext.i32.i64(%i_dst_stride)))]);
; CHECK-DAG: = &((%dst)[sext.i32.i64(%i_width) + (sext.i32.i64((-1 + %i_height)) * smax(0, sext.i32.i64(%i_dst_stride))) + -1]) >=u &((%src1)[(sext.i32.i64((-1 + %i_height)) * smin(0, sext.i32.i64(%i_src1_stride)))]);
; CHECK: %mv.and = %mv.test  &&  %mv.test2;
; CHECK-DAG: = &((%dst)[sext.i32.i64(%i_width) + (sext.i32.i64((-1 + %i_height)) * smax(0, sext.i32.i64(%i_dst_stride))) + -1]) >=u &((%src2)[(sext.i32.i64((-1 + %i_height)) * smin(0, sext.i32.i64(%i_src2_stride)))]);
; CHECK-DAG: = &((%src2)[sext.i32.i64(%i_width) + (sext.i32.i64((-1 + %i_height)) * smax(0, sext.i32.i64(%i_src2_stride))) + -1]) >=u &((%dst)[(sext.i32.i64((-1 + %i_height)) * smin(0, sext.i32.i64(%i_dst_stride)))]);
; CHECK: %mv.and5 = %mv.test3  &&  %mv.test4;
; CHECK: if (%mv.and == 0 && %mv.and5 == 0)

; Verify that unmodified loops are marked to do not vectorize
; CHECK: Loop metadata: No
; CHECK: DO
; Verify that the inner loop has vec-directives
; CHECK: @llvm.directive.region.entry
; CHECK: Loop metadata: No
; CHECK: DO
; CHECK: END LOOP
; CHECK: @llvm.directive.region.exit
; CHECK: END LOOP
; CHECK: Loop metadata: !llvm.loop
; CHECK: DO
; CHECK: Loop metadata: !llvm.loop
; CHECK: DO
; CHECK: END LOOP
; CHECK: END LOOP

;Module Before HIR; ModuleID = 'pixel_avg.c'
source_filename = "pixel_avg.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @pixel_avg(i8* nocapture %dst, i32 %i_dst_stride, i8* nocapture readonly %src1, i32 %i_src1_stride, i8* nocapture readonly %src2, i32 %i_src2_stride, i32 %i_width, i32 %i_height) local_unnamed_addr #0 {
entry:
  %cmp31 = icmp sgt i32 %i_height, 0
  br i1 %cmp31, label %for.cond1.preheader.lr.ph, label %for.cond.cleanup

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %cmp229 = icmp sgt i32 %i_width, 0
  %idx.ext = sext i32 %i_dst_stride to i64
  %idx.ext12 = sext i32 %i_src1_stride to i64
  %idx.ext14 = sext i32 %i_src2_stride to i64
  %wide.trip.count = sext i32 %i_width to i64
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond.cleanup3, %for.cond1.preheader.lr.ph
  %y.035 = phi i32 [ 0, %for.cond1.preheader.lr.ph ], [ %inc17, %for.cond.cleanup3 ]
  %dst.addr.034 = phi i8* [ %dst, %for.cond1.preheader.lr.ph ], [ %add.ptr, %for.cond.cleanup3 ]
  %src1.addr.033 = phi i8* [ %src1, %for.cond1.preheader.lr.ph ], [ %add.ptr13, %for.cond.cleanup3 ]
  %src2.addr.032 = phi i8* [ %src2, %for.cond1.preheader.lr.ph ], [ %add.ptr15, %for.cond.cleanup3 ]
  br i1 %cmp229, label %for.body4.preheader, label %for.cond.cleanup3

for.body4.preheader:                              ; preds = %for.cond1.preheader
  br label %for.body4

for.cond.cleanup.loopexit:                        ; preds = %for.cond.cleanup3
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.cond.cleanup3.loopexit:                       ; preds = %for.body4
  br label %for.cond.cleanup3

for.cond.cleanup3:                                ; preds = %for.cond.cleanup3.loopexit, %for.cond1.preheader
  %add.ptr = getelementptr inbounds i8, i8* %dst.addr.034, i64 %idx.ext
  %add.ptr13 = getelementptr inbounds i8, i8* %src1.addr.033, i64 %idx.ext12
  %add.ptr15 = getelementptr inbounds i8, i8* %src2.addr.032, i64 %idx.ext14
  %inc17 = add nuw nsw i32 %y.035, 1
  %exitcond36 = icmp eq i32 %inc17, %i_height
  br i1 %exitcond36, label %for.cond.cleanup.loopexit, label %for.cond1.preheader

for.body4:                                        ; preds = %for.body4.preheader, %for.body4
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body4 ], [ 0, %for.body4.preheader ]
  %arrayidx = getelementptr inbounds i8, i8* %src1.addr.033, i64 %indvars.iv
  %0 = load i8, i8* %arrayidx, align 1
  %conv = zext i8 %0 to i32
  %arrayidx6 = getelementptr inbounds i8, i8* %src2.addr.032, i64 %indvars.iv
  %1 = load i8, i8* %arrayidx6, align 1
  %conv7 = zext i8 %1 to i32
  %add = add nuw nsw i32 %conv, 1
  %add8 = add nuw nsw i32 %add, %conv7
  %2 = lshr i32 %add8, 1
  %conv9 = trunc i32 %2 to i8
  %arrayidx11 = getelementptr inbounds i8, i8* %dst.addr.034, i64 %indvars.iv
  store i8 %conv9, i8* %arrayidx11, align 1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup3.loopexit, label %for.body4
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }


