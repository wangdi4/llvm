; RUN: opt -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -dtrans-inline-heuristics -intel-libirc-allowed -inline -inline-report=0xe807 < %s -S 2>&1 | FileCheck --check-prefix=CHECK-IR %s
; RUN: opt -passes='cgscc(inline)' -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -dtrans-inline-heuristics -intel-libirc-allowed -inline-report=0xe807 < %s -S 2>&1 | FileCheck --check-prefix=CHECK-IR %s

; Checks that mc_chroma() is NOT inlined because it
; - has 2 consecutive AND instructions with 0x07 on their operands;
; - has 2 loops where each loop's UB traces back to a formal;
; - is a leaf function;
;
; As a result, mc_chroma() is prefered for multiversioning than inlining.
;

; CHECK-IR: -> mc_chroma {{\[\[}}Callsite preferred for multiversioning{{\]\]}}

source_filename = "test.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #0 {
entry:
  %dst = alloca [1000 x i8], align 16
  %src = alloca [1000 x i8], align 16
  %dst_stride = alloca i32, align 4
  %i_dst_stride = alloca i32, align 4
  %i_src_stride = alloca i32, align 4
  %mvx = alloca i32, align 4
  %mvy = alloca i32, align 4
  %i_width = alloca i32, align 4
  %i_height = alloca i32, align 4
  %0 = getelementptr inbounds [1000 x i8], [1000 x i8]* %dst, i64 0, i64 0
  call void @llvm.lifetime.start.p0i8(i64 1000, i8* nonnull %0) #3
  %1 = getelementptr inbounds [1000 x i8], [1000 x i8]* %src, i64 0, i64 0
  call void @llvm.lifetime.start.p0i8(i64 1000, i8* nonnull %1) #3
  %dst_stride.0.dst_stride.0..sroa_cast = bitcast i32* %dst_stride to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %dst_stride.0.dst_stride.0..sroa_cast)
  store volatile i32 1, i32* %dst_stride, align 4, !tbaa !3
  %i_dst_stride.0.i_dst_stride.0..sroa_cast = bitcast i32* %i_dst_stride to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %i_dst_stride.0.i_dst_stride.0..sroa_cast)
  store volatile i32 1, i32* %i_dst_stride, align 4, !tbaa !3
  %i_src_stride.0.i_src_stride.0..sroa_cast = bitcast i32* %i_src_stride to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %i_src_stride.0.i_src_stride.0..sroa_cast)
  store volatile i32 1, i32* %i_src_stride, align 4, !tbaa !3
  %mvx.0.mvx.0..sroa_cast = bitcast i32* %mvx to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %mvx.0.mvx.0..sroa_cast)
  store volatile i32 1, i32* %mvx, align 4, !tbaa !3
  %mvy.0.mvy.0..sroa_cast = bitcast i32* %mvy to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %mvy.0.mvy.0..sroa_cast)
  store volatile i32 2, i32* %mvy, align 4, !tbaa !3
  %i_width.0.i_width.0..sroa_cast = bitcast i32* %i_width to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %i_width.0.i_width.0..sroa_cast)
  store volatile i32 16, i32* %i_width, align 4, !tbaa !3
  %i_height.0.i_height.0..sroa_cast = bitcast i32* %i_height to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %i_height.0.i_height.0..sroa_cast)
  store volatile i32 8, i32* %i_height, align 4, !tbaa !3
  %i_dst_stride.0.i_dst_stride.0. = load volatile i32, i32* %i_dst_stride, align 4, !tbaa !3
  %i_src_stride.0.i_src_stride.0. = load volatile i32, i32* %i_src_stride, align 4, !tbaa !3
  %mvx.0.mvx.0. = load volatile i32, i32* %mvx, align 4, !tbaa !3
  %mvy.0.mvy.0. = load volatile i32, i32* %mvy, align 4, !tbaa !3
  %i_width.0.i_width.0. = load volatile i32, i32* %i_width, align 4, !tbaa !3
  %i_height.0.i_height.0. = load volatile i32, i32* %i_height, align 4, !tbaa !3
  call fastcc void @mc_chroma(i8* nonnull %0, i32 %i_dst_stride.0.i_dst_stride.0., i8* nonnull %1, i32 %i_src_stride.0.i_src_stride.0., i32 %mvx.0.mvx.0., i32 %mvy.0.mvy.0., i32 %i_width.0.i_width.0., i32 %i_height.0.i_height.0.)
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %i_height.0.i_height.0..sroa_cast)
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %i_width.0.i_width.0..sroa_cast)
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %mvy.0.mvy.0..sroa_cast)
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %mvx.0.mvx.0..sroa_cast)
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %i_src_stride.0.i_src_stride.0..sroa_cast)
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %i_dst_stride.0.i_dst_stride.0..sroa_cast)
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %dst_stride.0.dst_stride.0..sroa_cast)
  call void @llvm.lifetime.end.p0i8(i64 1000, i8* nonnull %1) #3
  call void @llvm.lifetime.end.p0i8(i64 1000, i8* nonnull %0) #3
  ret i32 0
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: norecurse nounwind uwtable
define internal fastcc void @mc_chroma(i8* nocapture %dst, i32 %i_dst_stride, i8* nocapture readonly %src, i32 %i_src_stride, i32 %mvx, i32 %mvy, i32 %i_width, i32 %i_height) unnamed_addr #2 {
entry:
  %and = and i32 %mvx, 7
  %and1 = and i32 %mvy, 7
  %sub = sub nsw i32 8, %and
  %sub2 = sub nsw i32 8, %and1
  %shr = ashr i32 %mvy, 3
  %mul8 = mul nsw i32 %shr, %i_src_stride
  %shr9 = ashr i32 %mvx, 3
  %add = add nsw i32 %mul8, %shr9
  %idx.ext = sext i32 %add to i64
  %add.ptr = getelementptr inbounds i8, i8* %src, i64 %idx.ext, !intel-tbaa !7
  %idxprom = sext i32 %i_src_stride to i64
  %cmp84 = icmp sgt i32 %i_height, 0
  br i1 %cmp84, label %for.cond10.preheader.lr.ph, label %for.cond.cleanup

for.cond10.preheader.lr.ph:                       ; preds = %entry
  %cmp1181 = icmp sgt i32 %i_width, 0
  %idx.ext39 = sext i32 %i_dst_stride to i64
  br i1 %cmp1181, label %for.cond10.preheader.us.preheader, label %for.cond.cleanup

for.cond10.preheader.us.preheader:                ; preds = %for.cond10.preheader.lr.ph
  %wide.trip.count = sext i32 %i_width to i64
  br label %for.cond10.preheader.us

for.cond10.preheader.us:                          ; preds = %for.cond10.for.cond.cleanup12_crit_edge.us, %for.cond10.preheader.us.preheader
  %srcp.088.us.pn = phi i8* [ %srcp.088.us, %for.cond10.for.cond.cleanup12_crit_edge.us ], [ %add.ptr, %for.cond10.preheader.us.preheader ]
  %y.087.us = phi i32 [ %inc44.us, %for.cond10.for.cond.cleanup12_crit_edge.us ], [ 0, %for.cond10.preheader.us.preheader ]
  %dst.addr.086.us = phi i8* [ %add.ptr40.us, %for.cond10.for.cond.cleanup12_crit_edge.us ], [ %dst, %for.cond10.preheader.us.preheader ]
  %srcp.088.us = getelementptr inbounds i8, i8* %srcp.088.us.pn, i64 %idxprom
  br label %for.body13.us

for.body13.us:                                    ; preds = %for.body13.us, %for.cond10.preheader.us
  %indvars.iv = phi i64 [ 0, %for.cond10.preheader.us ], [ %indvars.iv.next, %for.body13.us ]
  %arrayidx15.us = getelementptr inbounds i8, i8* %srcp.088.us.pn, i64 %indvars.iv
  %0 = load i8, i8* %arrayidx15.us, align 1, !tbaa !7
  %conv.us = zext i8 %0 to i32
  %mul16.us = mul nuw nsw i32 %sub, %conv.us
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %arrayidx19.us = getelementptr inbounds i8, i8* %srcp.088.us.pn, i64 %indvars.iv.next
  %1 = load i8, i8* %arrayidx19.us, align 1, !tbaa !7
  %conv20.us = zext i8 %1 to i32
  %mul21.us = mul nuw nsw i32 %and, %conv20.us
  %arrayidx24.us = getelementptr inbounds i8, i8* %srcp.088.us, i64 %indvars.iv
  %2 = load i8, i8* %arrayidx24.us, align 1, !tbaa !7
  %conv25.us = zext i8 %2 to i32
  %mul26.us = mul nuw nsw i32 %sub, %conv25.us
  %arrayidx30.us = getelementptr inbounds i8, i8* %srcp.088.us, i64 %indvars.iv.next
  %3 = load i8, i8* %arrayidx30.us, align 1, !tbaa !7
  %conv31.us = zext i8 %3 to i32
  %mul32.us = mul nuw nsw i32 %and, %conv31.us
  %reass.add.us = add nuw nsw i32 %mul32.us, %mul26.us
  %reass.mul.us = mul nuw nsw i32 %reass.add.us, %and1
  %reass.add79.us = add nuw nsw i32 %mul21.us, %mul16.us
  %reass.mul80.us = mul nuw nsw i32 %reass.add79.us, %sub2
  %add33.us = add nuw nsw i32 %reass.mul80.us, 32
  %add34.us = add nuw nsw i32 %add33.us, %reass.mul.us
  %4 = lshr i32 %add34.us, 6
  %conv36.us = trunc i32 %4 to i8
  %arrayidx38.us = getelementptr inbounds i8, i8* %dst.addr.086.us, i64 %indvars.iv
  store i8 %conv36.us, i8* %arrayidx38.us, align 1, !tbaa !7
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond10.for.cond.cleanup12_crit_edge.us, label %for.body13.us

for.cond10.for.cond.cleanup12_crit_edge.us:       ; preds = %for.body13.us
  %add.ptr40.us = getelementptr inbounds i8, i8* %dst.addr.086.us, i64 %idx.ext39, !intel-tbaa !7
  %inc44.us = add nuw nsw i32 %y.087.us, 1
  %exitcond90 = icmp eq i32 %inc44.us, %i_height
  br i1 %exitcond90, label %for.cond.cleanup, label %for.cond10.preheader.us

for.cond.cleanup:                                 ; preds = %for.cond10.for.cond.cleanup12_crit_edge.us, %for.cond10.preheader.lr.ph, %entry
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, !"ThinLTO", i32 0}
!2 = !{!"clang version 8.0.0"}
!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = !{!5, !5, i64 0}

;Source code:
;
;#include <stdint.h>
;#include <stdio.h>
;#include <string.h>
;
;typedef struct x264_weight_type {
;  /* aligning the first member is a gcc hack to force the struct to be
;     * 16 byte aligned, as well as force sizeof(struct) to be a multiple of 16 */
;  int16_t cachea[8];
;  int16_t cacheb[8];
;  int32_t i_denom;
;  int32_t i_scale;
;  int32_t i_offset;
;  //weight_fn_t *weightfn;
;  int *weightfn;
;} x264_weight_t;
;
;/* full chroma mc (ie until 1/8 pixel)*/
;static void mc_chroma(uint8_t *dst, int i_dst_stride,
;                      uint8_t *src, int i_src_stride,
;                      int mvx, int mvy,
;                      int i_width, int i_height) {
;  uint8_t *srcp;
;
;  int d8x = mvx & 0x07;
;  int d8y = mvy & 0x07;
;  int cA = (8 - d8x) * (8 - d8y);
;  int cB = d8x * (8 - d8y);
;  int cC = (8 - d8x) * d8y;
;  int cD = d8x * d8y;
;
;  src += (mvy >> 3) * i_src_stride + (mvx >> 3);
;  srcp = &src[i_src_stride];
;
;  for (int y = 0; y < i_height; y++) {
;    for (int x = 0; x < i_width; x++)
;      dst[x] = (cA * src[x] + cB * src[x + 1] + cC * srcp[x] + cD * srcp[x + 1] + 32) >> 6;
;    dst += i_dst_stride;
;    src = srcp;
;    srcp += i_src_stride;
;  }
;}
;
;int main(void) {
;  volatile uint8_t dst[1000];
;  volatile uint8_t src[1000];
;  volatile uint8_t src4[4];
;  volatile int dst_stride = 1;
;  volatile int i_dst_stride = 1;
;  volatile int i_src_stride = 1;
;  volatile x264_weight_t weight_t;
;  volatile int mvx = 1, mvy = 2;
;  volatile uint8_t *RV = 0;
;  volatile int i_width = 16;
;  volatile int i_height = 8;
;
;  mc_chroma(dst, i_dst_stride,
;            src, i_src_stride,
;            mvx, mvy,
;            i_width, i_height);
;
;  return 0;
;}
;
