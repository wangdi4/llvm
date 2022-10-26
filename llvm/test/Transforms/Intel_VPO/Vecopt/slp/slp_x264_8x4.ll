; RUN: opt < %s -basic-aa -slp-vectorizer -enable-intel-advanced-opts -slp-multinode -S -mtriple=x86_64 -mcpu=skylake-avx512 | FileCheck %s

; This is a test for PSLP. It checks whether the 8x4 loop in x264 gets vectorized.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

@i_pix1 = local_unnamed_addr global i32 4, align 4
@i_pix2 = local_unnamed_addr global i32 4, align 4
@pix1 = common global [8 x i8] zeroinitializer, align 1
@pix2 = common global [8 x i8] zeroinitializer, align 1
@res = common local_unnamed_addr global i32 0, align 4

; Function Attrs: noinline nounwind readonly uwtable
define void @x264_pixel_satd_8x4(i8* nocapture readonly %pix1, i32 %i_pix1, i8* nocapture readonly %pix2, i32 %i_pix2) local_unnamed_addr #0 {

; Make sure that we have vectorized from the store all the way to the 4 loads.
; CHECK: [[VEC_LOAD1:%.*]] = load <4 x i8>, <4 x i8>*
; CHECK: [[VEC_LOAD2:%.*]] = load <4 x i8>, <4 x i8>*
; CHECK: [[VEC_LOAD3:%.*]] = load <4 x i8>, <4 x i8>*
; CHECK: [[VEC_LOAD4:%.*]] = load <4 x i8>, <4 x i8>*
; CHECK: store <4 x i32> [[Hmm:%.*]], <4 x i32>*

entry:
  %tmp = alloca [4 x [4 x i32]], align 16
  %0 = bitcast [4 x [4 x i32]]* %tmp to i8*
  ; call void @llvm.lifetime.start.p0i8(i64 64, i8* nonnull %0) #3
  %idx.ext = sext i32 %i_pix1 to i64
  %idx.ext63 = sext i32 %i_pix2 to i64
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv204 = phi i64 [ 0, %entry ], [ %indvars.iv.next205, %for.body ]
  %pix1.addr.0203 = phi i8* [ %pix1, %entry ], [ %add.ptr, %for.body ]
  %pix2.addr.0202 = phi i8* [ %pix2, %entry ], [ %add.ptr64, %for.body ]
  %1 = load i8, i8* %pix1.addr.0203, align 1, !tbaa !2
  %conv = zext i8 %1 to i32
  %2 = load i8, i8* %pix2.addr.0202, align 1, !tbaa !2
  %conv2 = zext i8 %2 to i32
  %sub = sub nsw i32 %conv, %conv2
  %arrayidx3 = getelementptr inbounds i8, i8* %pix1.addr.0203, i64 4
  %3 = load i8, i8* %arrayidx3, align 1, !tbaa !2
  %conv4 = zext i8 %3 to i32
  %arrayidx5 = getelementptr inbounds i8, i8* %pix2.addr.0202, i64 4
  %4 = load i8, i8* %arrayidx5, align 1, !tbaa !2
  %conv6 = zext i8 %4 to i32
  %sub7 = sub nsw i32 %conv4, %conv6
  %shl = shl nsw i32 %sub7, 16
  %add = add nsw i32 %shl, %sub
  %arrayidx8 = getelementptr inbounds i8, i8* %pix1.addr.0203, i64 1
  %5 = load i8, i8* %arrayidx8, align 1, !tbaa !2
  %conv9 = zext i8 %5 to i32
  %arrayidx10 = getelementptr inbounds i8, i8* %pix2.addr.0202, i64 1
  %6 = load i8, i8* %arrayidx10, align 1, !tbaa !2
  %conv11 = zext i8 %6 to i32
  %sub12 = sub nsw i32 %conv9, %conv11
  %arrayidx13 = getelementptr inbounds i8, i8* %pix1.addr.0203, i64 5
  %7 = load i8, i8* %arrayidx13, align 1, !tbaa !2
  %conv14 = zext i8 %7 to i32
  %arrayidx15 = getelementptr inbounds i8, i8* %pix2.addr.0202, i64 5
  %8 = load i8, i8* %arrayidx15, align 1, !tbaa !2
  %conv16 = zext i8 %8 to i32
  %sub17 = sub nsw i32 %conv14, %conv16
  %shl18 = shl nsw i32 %sub17, 16
  %add19 = add nsw i32 %shl18, %sub12
  %arrayidx20 = getelementptr inbounds i8, i8* %pix1.addr.0203, i64 2
  %9 = load i8, i8* %arrayidx20, align 1, !tbaa !2
  %conv21 = zext i8 %9 to i32
  %arrayidx22 = getelementptr inbounds i8, i8* %pix2.addr.0202, i64 2
  %10 = load i8, i8* %arrayidx22, align 1, !tbaa !2
  %conv23 = zext i8 %10 to i32
  %sub24 = sub nsw i32 %conv21, %conv23
  %arrayidx25 = getelementptr inbounds i8, i8* %pix1.addr.0203, i64 6
  %11 = load i8, i8* %arrayidx25, align 1, !tbaa !2
  %conv26 = zext i8 %11 to i32
  %arrayidx27 = getelementptr inbounds i8, i8* %pix2.addr.0202, i64 6
  %12 = load i8, i8* %arrayidx27, align 1, !tbaa !2
  %conv28 = zext i8 %12 to i32
  %sub29 = sub nsw i32 %conv26, %conv28
  %shl30 = shl nsw i32 %sub29, 16
  %add31 = add nsw i32 %shl30, %sub24
  %arrayidx32 = getelementptr inbounds i8, i8* %pix1.addr.0203, i64 3
  %13 = load i8, i8* %arrayidx32, align 1, !tbaa !2
  %conv33 = zext i8 %13 to i32
  %arrayidx34 = getelementptr inbounds i8, i8* %pix2.addr.0202, i64 3
  %14 = load i8, i8* %arrayidx34, align 1, !tbaa !2
  %conv35 = zext i8 %14 to i32
  %sub36 = sub nsw i32 %conv33, %conv35
  %arrayidx37 = getelementptr inbounds i8, i8* %pix1.addr.0203, i64 7
  %15 = load i8, i8* %arrayidx37, align 1, !tbaa !2
  %conv38 = zext i8 %15 to i32
  %arrayidx39 = getelementptr inbounds i8, i8* %pix2.addr.0202, i64 7
  %16 = load i8, i8* %arrayidx39, align 1, !tbaa !2
  %conv40 = zext i8 %16 to i32
  %sub41 = sub nsw i32 %conv38, %conv40
  %shl42 = shl nsw i32 %sub41, 16
  %add43 = add nsw i32 %shl42, %sub36
  %add44 = add nsw i32 %add19, %add
  %sub45 = sub nsw i32 %add, %add19
  %add46 = add nsw i32 %add43, %add31
  %sub47 = sub nsw i32 %add31, %add43
  %add48 = add nsw i32 %add46, %add44
  %arrayidx50 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %tmp, i64 0, i64 %indvars.iv204, i64 0
  store i32 %add48, i32* %arrayidx50, align 16, !tbaa !5
  %sub51 = sub nsw i32 %add44, %add46
  %arrayidx54 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %tmp, i64 0, i64 %indvars.iv204, i64 2
  store i32 %sub51, i32* %arrayidx54, align 8, !tbaa !5
  %add55 = add nsw i32 %sub47, %sub45
  %arrayidx58 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %tmp, i64 0, i64 %indvars.iv204, i64 1
  store i32 %add55, i32* %arrayidx58, align 4, !tbaa !5
  %sub59 = sub nsw i32 %sub45, %sub47
  %arrayidx62 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %tmp, i64 0, i64 %indvars.iv204, i64 3
  store i32 %sub59, i32* %arrayidx62, align 4, !tbaa !5
  %indvars.iv.next205 = add nuw nsw i64 %indvars.iv204, 1
  %add.ptr = getelementptr inbounds i8, i8* %pix1.addr.0203, i64 %idx.ext
  %add.ptr64 = getelementptr inbounds i8, i8* %pix2.addr.0202, i64 %idx.ext63
  %exitcond206 = icmp eq i64 %indvars.iv.next205, 4
  br i1 %exitcond206, label %exit, label %for.body

exit:
  ret void
}
!2 = !{!3, !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
!5 = !{!6, !8, i64 0}
!6 = !{!"array@_ZTSA4_A4_j", !7, i64 0}
!7 = !{!"array@_ZTSA4_j", !8, i64 0}
!8 = !{!"int", !3, i64 0}
!9 = !{!8, !8, i64 0}

