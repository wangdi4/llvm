; RUN: opt -hir-ssa-deconstruction -disable-output -hir-runtime-dd -print-after=hir-runtime-dd < %s 2>&1 | FileCheck %s

; Verify that struct references are supported by RTDD.

; BEGIN REGION { }
; + DO i1 = 0, %i_height + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>
; |   + DO i2 = 0, sext.i32.i64(%i_width) + -1, 1   <DO_LOOP>
; |   |   %1 = (%src)[sext.i32.i64(%i_src_stride) * i1 + i2 + -1];
; |   |   %2 = (%weight)[0].0;
; |   |   %3 = (%weight)[0].2;
; |   |   (%src)[sext.i32.i64(%i_src_stride) * i1 + i2] = -1 * smax(-256, (-1 + (-1 * smax(0, ((zext.i8.i32(%1) * %2) + %3))))) + -1;
; |   + END LOOP
; + END LOOP
; END REGION

; CHECK: %mv.cast = bitcast.i32*.i8*(&((%weight)[0].0));
; CHECK: %mv.cast2 = bitcast.i32*.i8*(&((%weight)[0].2));
; CHECK: %mv.test = &((%src)[sext.i32.i64(%i_width) + (sext.i32.i64((-1 + %i_height)) * smax(0, sext.i32.i64(%i_src_stride))) + -1]) >=u %mv.cast;
; CHECK: %mv.test3 = %mv.cast2 >=u &((%src)[(sext.i32.i64((-1 + %i_height)) * (-1 + (-1 * smax(-1, (-1 + (-1 * sext.i32.i64(%i_src_stride))))))) + -1]);
; CHECK: %mv.and = %mv.test  &&  %mv.test3;
; CHECK: if (%mv.and == 0)

;Module Before HIR; ModuleID = 'mc_weight.c'
source_filename = "mc_weight.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.x264_weight_t = type { i32, i32, i32 }

; Function Attrs: norecurse nounwind uwtable
define void @mc_weight(i8* nocapture readnone %dst, i32 %i_dst_stride, i8* nocapture %src, i32 %i_src_stride, %struct.x264_weight_t* nocapture readonly %weight, i32 %i_width, i32 %i_height) local_unnamed_addr #0 {
entry:
  %cmp23 = icmp sgt i32 %i_height, 0
  br i1 %cmp23, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %entry
  %cmp221 = icmp sgt i32 %i_width, 0
  %i_scale = getelementptr inbounds %struct.x264_weight_t, %struct.x264_weight_t* %weight, i64 0, i32 0
  %i_offset = getelementptr inbounds %struct.x264_weight_t, %struct.x264_weight_t* %weight, i64 0, i32 2
  %idx.ext9 = sext i32 %i_src_stride to i64
  %wide.trip.count = sext i32 %i_width to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.cond.cleanup3
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.cond.cleanup3, %for.body.lr.ph
  %y.025 = phi i32 [ 0, %for.body.lr.ph ], [ %inc8, %for.cond.cleanup3 ]
  %src.addr.024 = phi i8* [ %src, %for.body.lr.ph ], [ %add.ptr10, %for.cond.cleanup3 ]
  br i1 %cmp221, label %for.body4.lr.ph, label %for.cond.cleanup3

for.body4.lr.ph:                                  ; preds = %for.body
  br label %for.body4

for.cond.cleanup3.loopexit:                       ; preds = %for.body4
  br label %for.cond.cleanup3

for.cond.cleanup3:                                ; preds = %for.cond.cleanup3.loopexit, %for.body
  %inc8 = add nuw nsw i32 %y.025, 1
  %add.ptr10 = getelementptr inbounds i8, i8* %src.addr.024, i64 %idx.ext9
  %exitcond27 = icmp eq i32 %inc8, %i_height
  br i1 %exitcond27, label %for.cond.cleanup.loopexit, label %for.body

for.body4:                                        ; preds = %for.body4, %for.body4.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body4.lr.ph ], [ %indvars.iv.next, %for.body4 ]
  %0 = add nsw i64 %indvars.iv, -1
  %arrayidx = getelementptr inbounds i8, i8* %src.addr.024, i64 %0
  %1 = load i8, i8* %arrayidx, align 1
  %conv = zext i8 %1 to i32
  %2 = load i32, i32* %i_scale, align 4
  %mul = mul nsw i32 %2, %conv
  %3 = load i32, i32* %i_offset, align 4
  %add = add nsw i32 %mul, %3
  %4 = icmp sgt i32 %add, 0
  %5 = select i1 %4, i32 %add, i32 0
  %6 = icmp slt i32 %5, 255
  %7 = select i1 %6, i32 %5, i32 255
  %conv.i = trunc i32 %7 to i8
  %arrayidx6 = getelementptr inbounds i8, i8* %src.addr.024, i64 %indvars.iv
  store i8 %conv.i, i8* %arrayidx6, align 1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup3.loopexit, label %for.body4
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }


