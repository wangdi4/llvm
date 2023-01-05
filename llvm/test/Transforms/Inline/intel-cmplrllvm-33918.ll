; INTEL_FEATURE_SW_ADVANCED
; RUN: opt -passes='cgscc(inline)' -inlined-alloca-merging-npm=false -S < %s 2>&1 | FileCheck --check-prefixes=CHECK,CHECK-OFF %s
; RUN: opt -passes='cgscc(inline)' -inlined-alloca-merging-npm=true -S < %s 2>&1 | FileCheck --check-prefixes=CHECK,CHECK-ON %s

; Check enabling and disabling of alloca merging in the inliner

; CHECK: define i32 @x264_pixel_satd_16x16
; CHECK: alloca [4 x [4 x i32]], align 16
; CHECK-OFF-NEXT: alloca [4 x [4 x i32]], align 16
; CHECK-ON-NOT: alloca [4 x [4 x i32]], align 16

define external i32 @x264_pixel_satd_8x4(i8* nocapture noundef readonly %pix1, i32 noundef %i_pix1, i8* nocapture noundef readonly %pix2, i32 noundef %i_pix2) #3 #0 {
entry:
  %tmp = alloca [4 x [4 x i32]], align 16
  %0 = bitcast [4 x [4 x i32]]* %tmp to i8*
  %idx.ext = sext i32 %i_pix1 to i64
  %idx.ext63 = sext i32 %i_pix2 to i64
  br label %for.body

for.cond66.preheader:                             ; preds = %for.body
  %arrayidx72 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %tmp, i64 0, i64 0
  %arrayidx75 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %tmp, i64 0, i64 1
  %arrayidx88 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %tmp, i64 0, i64 2
  %arrayidx91 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %tmp, i64 0, i64 3
  br label %for.body70

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %pix1.addr.0201 = phi i8* [ %pix1, %entry ], [ %add.ptr, %for.body ]
  %pix2.addr.0200 = phi i8* [ %pix2, %entry ], [ %add.ptr64, %for.body ]
  %1 = load i8, i8* %pix1.addr.0201, align 1
  %conv = zext i8 %1 to i32
  %2 = load i8, i8* %pix2.addr.0200, align 1
  %conv2 = zext i8 %2 to i32
  %sub = sub nsw i32 %conv, %conv2
  %arrayidx3 = getelementptr inbounds i8, i8* %pix1.addr.0201, i64 4
  %3 = load i8, i8* %arrayidx3, align 1
  %conv4 = zext i8 %3 to i32
  %arrayidx5 = getelementptr inbounds i8, i8* %pix2.addr.0200, i64 4
  %4 = load i8, i8* %arrayidx5, align 1
  %conv6 = zext i8 %4 to i32
  %sub7 = sub nsw i32 %conv4, %conv6
  %shl = shl nsw i32 %sub7, 16
  %add = add nsw i32 %shl, %sub
  %arrayidx8 = getelementptr inbounds i8, i8* %pix1.addr.0201, i64 1
  %5 = load i8, i8* %arrayidx8, align 1
  %conv9 = zext i8 %5 to i32
  %arrayidx10 = getelementptr inbounds i8, i8* %pix2.addr.0200, i64 1
  %6 = load i8, i8* %arrayidx10, align 1
  %conv11 = zext i8 %6 to i32
  %sub12 = sub nsw i32 %conv9, %conv11
  %arrayidx13 = getelementptr inbounds i8, i8* %pix1.addr.0201, i64 5
  %7 = load i8, i8* %arrayidx13, align 1
  %conv14 = zext i8 %7 to i32
  %arrayidx15 = getelementptr inbounds i8, i8* %pix2.addr.0200, i64 5
  %8 = load i8, i8* %arrayidx15, align 1
  %conv16 = zext i8 %8 to i32
  %sub17 = sub nsw i32 %conv14, %conv16
  %shl18 = shl nsw i32 %sub17, 16
  %add19 = add nsw i32 %shl18, %sub12
  %arrayidx20 = getelementptr inbounds i8, i8* %pix1.addr.0201, i64 2
  %9 = load i8, i8* %arrayidx20, align 1
  %conv21 = zext i8 %9 to i32
  %arrayidx22 = getelementptr inbounds i8, i8* %pix2.addr.0200, i64 2
  %10 = load i8, i8* %arrayidx22, align 1
  %conv23 = zext i8 %10 to i32
  %sub24 = sub nsw i32 %conv21, %conv23
  %arrayidx25 = getelementptr inbounds i8, i8* %pix1.addr.0201, i64 6
  %11 = load i8, i8* %arrayidx25, align 1
  %conv26 = zext i8 %11 to i32
  %arrayidx27 = getelementptr inbounds i8, i8* %pix2.addr.0200, i64 6
  %12 = load i8, i8* %arrayidx27, align 1
  %conv28 = zext i8 %12 to i32
  %sub29 = sub nsw i32 %conv26, %conv28
  %shl30 = shl nsw i32 %sub29, 16
  %add31 = add nsw i32 %shl30, %sub24
  %arrayidx32 = getelementptr inbounds i8, i8* %pix1.addr.0201, i64 3
  %13 = load i8, i8* %arrayidx32, align 1
  %conv33 = zext i8 %13 to i32
  %arrayidx34 = getelementptr inbounds i8, i8* %pix2.addr.0200, i64 3
  %14 = load i8, i8* %arrayidx34, align 1
  %conv35 = zext i8 %14 to i32
  %sub36 = sub nsw i32 %conv33, %conv35
  %arrayidx37 = getelementptr inbounds i8, i8* %pix1.addr.0201, i64 7
  %15 = load i8, i8* %arrayidx37, align 1
  %conv38 = zext i8 %15 to i32
  %arrayidx39 = getelementptr inbounds i8, i8* %pix2.addr.0200, i64 7
  %16 = load i8, i8* %arrayidx39, align 1
  %conv40 = zext i8 %16 to i32
  %sub41 = sub nsw i32 %conv38, %conv40
  %shl42 = shl nsw i32 %sub41, 16
  %add43 = add nsw i32 %shl42, %sub36
  %add44 = add nsw i32 %add19, %add
  %sub45 = sub nsw i32 %add, %add19
  %add46 = add nsw i32 %add43, %add31
  %sub47 = sub nsw i32 %add31, %add43
  %add48 = add nsw i32 %add46, %add44
  %arrayidx49 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %tmp, i64 0, i64 %indvars.iv
  %arrayidx50 = getelementptr inbounds [4 x i32], [4 x i32]* %arrayidx49, i64 0, i64 0
  store i32 %add48, i32* %arrayidx50, align 16
  %sub51 = sub nsw i32 %add44, %add46
  %arrayidx54 = getelementptr inbounds [4 x i32], [4 x i32]* %arrayidx49, i64 0, i64 2
  store i32 %sub51, i32* %arrayidx54, align 8
  %add55 = add nsw i32 %sub47, %sub45
  %arrayidx58 = getelementptr inbounds [4 x i32], [4 x i32]* %arrayidx49, i64 0, i64 1
  store i32 %add55, i32* %arrayidx58, align 4
  %sub59 = sub nsw i32 %sub45, %sub47
  %arrayidx62 = getelementptr inbounds [4 x i32], [4 x i32]* %arrayidx49, i64 0, i64 3
  store i32 %sub59, i32* %arrayidx62, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %add.ptr = getelementptr inbounds i8, i8* %pix1.addr.0201, i64 %idx.ext
  %add.ptr64 = getelementptr inbounds i8, i8* %pix2.addr.0200, i64 %idx.ext63
  %exitcond.not = icmp eq i64 %indvars.iv.next, 4
  br i1 %exitcond.not, label %for.cond66.preheader, label %for.body

for.cond.cleanup69:                               ; preds = %for.body70
  %conv118 = and i32 %add113, 65535
  %shr = lshr i32 %add113, 16
  %add119 = add nuw nsw i32 %conv118, %shr
  %shr120 = lshr i32 %add119, 1
  ret i32 %shr120

for.body70:                                       ; preds = %for.cond66.preheader, %for.body70
  %indvars.iv204 = phi i64 [ 0, %for.cond66.preheader ], [ %indvars.iv.next205, %for.body70 ]
  %sum.0202 = phi i32 [ 0, %for.cond66.preheader ], [ %add113, %for.body70 ]
  %arrayidx74 = getelementptr inbounds [4 x i32], [4 x i32]* %arrayidx72, i64 0, i64 %indvars.iv204
  %17 = load i32, i32* %arrayidx74, align 4
  %arrayidx77 = getelementptr inbounds [4 x i32], [4 x i32]* %arrayidx75, i64 0, i64 %indvars.iv204
  %18 = load i32, i32* %arrayidx77, align 4
  %add78 = add i32 %18, %17
  %sub86 = sub i32 %17, %18
  %arrayidx90 = getelementptr inbounds [4 x i32], [4 x i32]* %arrayidx88, i64 0, i64 %indvars.iv204
  %19 = load i32, i32* %arrayidx90, align 4
  %arrayidx93 = getelementptr inbounds [4 x i32], [4 x i32]* %arrayidx91, i64 0, i64 %indvars.iv204
  %20 = load i32, i32* %arrayidx93, align 4
  %add94 = add i32 %20, %19
  %sub102 = sub i32 %19, %20
  %add103 = add nsw i32 %add94, %add78
  %sub104 = sub nsw i32 %add78, %add94
  %add105 = add nsw i32 %sub102, %sub86
  %sub106 = sub nsw i32 %sub86, %sub102
  %shr.i = lshr i32 %add103, 15
  %and.i = and i32 %shr.i, 65537
  %mul.i = mul nuw i32 %and.i, 65535
  %add.i = add i32 %mul.i, %add103
  %xor.i = xor i32 %add.i, %mul.i
  %shr.i184 = lshr i32 %add105, 15
  %and.i185 = and i32 %shr.i184, 65537
  %mul.i186 = mul nuw i32 %and.i185, 65535
  %add.i187 = add i32 %mul.i186, %add105
  %xor.i188 = xor i32 %add.i187, %mul.i186
  %shr.i189 = lshr i32 %sub104, 15
  %and.i190 = and i32 %shr.i189, 65537
  %mul.i191 = mul nuw i32 %and.i190, 65535
  %add.i192 = add i32 %mul.i191, %sub104
  %xor.i193 = xor i32 %add.i192, %mul.i191
  %shr.i194 = lshr i32 %sub106, 15
  %and.i195 = and i32 %shr.i194, 65537
  %mul.i196 = mul nuw i32 %and.i195, 65535
  %add.i197 = add i32 %mul.i196, %sub106
  %xor.i198 = xor i32 %add.i197, %mul.i196
  %add108 = add i32 %xor.i188, %sum.0202
  %add110 = add i32 %add108, %xor.i
  %add112 = add i32 %add110, %xor.i193
  %add113 = add i32 %add112, %xor.i198
  %indvars.iv.next205 = add nuw nsw i64 %indvars.iv204, 1
  %exitcond206.not = icmp eq i64 %indvars.iv.next205, 4
  br i1 %exitcond206.not, label %for.cond.cleanup69, label %for.body70
}

define external i32 @x264_pixel_satd_16x16(i8* noundef %pix1, i32 noundef %i_pix1, i8* noundef %pix2, i32 noundef %i_pix2) #0 {
entry:
  %call = call i32 @x264_pixel_satd_8x4(i8* noundef %pix1, i32 noundef %i_pix1, i8* noundef %pix2, i32 noundef %i_pix2)
  %mul = shl nsw i32 %i_pix1, 2
  %idx.ext = sext i32 %mul to i64
  %add.ptr = getelementptr inbounds i8, i8* %pix1, i64 %idx.ext
  %mul1 = shl nsw i32 %i_pix2, 2
  %idx.ext2 = sext i32 %mul1 to i64
  %add.ptr3 = getelementptr inbounds i8, i8* %pix2, i64 %idx.ext2
  %call4 = call i32 @x264_pixel_satd_8x4(i8* noundef %add.ptr, i32 noundef %i_pix1, i8* noundef %add.ptr3, i32 noundef %i_pix2)
  %add = add nsw i32 %call, %call4
  %add.ptr5 = getelementptr inbounds i8, i8* %pix1, i64 8
  %add.ptr6 = getelementptr inbounds i8, i8* %pix2, i64 8
  %call7 = call i32 @x264_pixel_satd_8x4(i8* noundef nonnull %add.ptr5, i32 noundef %i_pix1, i8* noundef nonnull %add.ptr6, i32 noundef %i_pix2)
  %add.ptr11 = getelementptr inbounds i8, i8* %add.ptr5, i64 %idx.ext
  %add.ptr15 = getelementptr inbounds i8, i8* %add.ptr6, i64 %idx.ext2
  %call16 = call i32 @x264_pixel_satd_8x4(i8* noundef nonnull %add.ptr11, i32 noundef %i_pix1, i8* noundef nonnull %add.ptr15, i32 noundef %i_pix2)
  %add17 = add nsw i32 %call7, %call16
  %add18 = add nsw i32 %add, %add17
  %mul19 = shl nsw i32 %i_pix1, 3
  %idx.ext20 = sext i32 %mul19 to i64
  %add.ptr21 = getelementptr inbounds i8, i8* %pix1, i64 %idx.ext20
  %mul22 = shl nsw i32 %i_pix2, 3
  %idx.ext23 = sext i32 %mul22 to i64
  %add.ptr24 = getelementptr inbounds i8, i8* %pix2, i64 %idx.ext23
  %call25 = call i32 @x264_pixel_satd_8x4(i8* noundef %add.ptr21, i32 noundef %i_pix1, i8* noundef %add.ptr24, i32 noundef %i_pix2)
  %mul26 = mul nsw i32 %i_pix1, 12
  %idx.ext27 = sext i32 %mul26 to i64
  %add.ptr28 = getelementptr inbounds i8, i8* %pix1, i64 %idx.ext27
  %mul29 = mul nsw i32 %i_pix2, 12
  %idx.ext30 = sext i32 %mul29 to i64
  %add.ptr31 = getelementptr inbounds i8, i8* %pix2, i64 %idx.ext30
  %call32 = call i32 @x264_pixel_satd_8x4(i8* noundef %add.ptr28, i32 noundef %i_pix1, i8* noundef %add.ptr31, i32 noundef %i_pix2)
  %add33 = add nsw i32 %call25, %call32
  %add34 = add nsw i32 %add18, %add33
  %add.ptr38 = getelementptr inbounds i8, i8* %add.ptr5, i64 %idx.ext20
  %add.ptr42 = getelementptr inbounds i8, i8* %add.ptr6, i64 %idx.ext23
  %call43 = call i32 @x264_pixel_satd_8x4(i8* noundef nonnull %add.ptr38, i32 noundef %i_pix1, i8* noundef nonnull %add.ptr42, i32 noundef %i_pix2)
  %add.ptr47 = getelementptr inbounds i8, i8* %add.ptr5, i64 %idx.ext27
  %add.ptr51 = getelementptr inbounds i8, i8* %add.ptr6, i64 %idx.ext30
  %call52 = call i32 @x264_pixel_satd_8x4(i8* noundef nonnull %add.ptr47, i32 noundef %i_pix1, i8* noundef nonnull %add.ptr51, i32 noundef %i_pix2)
  %add53 = add nsw i32 %call43, %call52
  %add54 = add nsw i32 %add34, %add53
  ret i32 %add54
}

attributes #0 = { alwaysinline }
; end INTEL_FEATURE_SW_ADVANCED
