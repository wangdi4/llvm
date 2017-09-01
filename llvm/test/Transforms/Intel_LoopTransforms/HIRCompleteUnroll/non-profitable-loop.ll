; RUN: opt -hir-ssa-deconstruction -hir-post-vec-complete-unroll -print-before=hir-post-vec-complete-unroll -print-after=hir-post-vec-complete-unroll 2>&1 < %s | FileCheck %s

; Verify that this loop with lots of non-linear blobs is not unrolled.

; HIR -
; + DO i1 = 0, 7, 1   <DO_LOOP>
; |   %0 = (%data)[8 * i1];
; |   %1 = (%data)[8 * i1 + 7];
; |   %2 = (%data)[8 * i1 + 1];
; |   %3 = (%data)[8 * i1 + 6];
; |   %4 = (%data)[8 * i1 + 2];
; |   %5 = (%data)[8 * i1 + 5];
; |   %6 = (%data)[8 * i1 + 3];
; |   %7 = (%data)[8 * i1 + 4];
; |   (%data)[8 * i1] = 4 * (%2 + %3 + %4 + %5 + %0 + %1 + %6 + %7);
; |   (%data)[8 * i1 + 4] = 4 * ((-1 * (%2 + %3 + %4 + %5)) + %0 + %1 + %6 + %7);
; |   %shr = 4433 * (%2 + %3) + 10703 * (%0 + %1) + -10703 * (%6 + %7) + -4433 * (%4 + %5) + 1024  >>  11;
; |   (%data)[8 * i1 + 2] = %shr;
; |   %shr39 = -10704 * (%2 + %3) + 4433 * (%0 + %1) + -4433 * (%6 + %7) + 10704 * (%4 + %5) + 1024  >>  11;
; |   (%data)[8 * i1 + 6] = %shr39;
; |   %shr60 = 2260 * %0 + -2260 * %1 + -6436 * %2 + 6436 * %3 + 9633 * %4 + -9633 * %5 + -11363 * %6 + 11363 * %7 + 1024  >>  11;
; |   (%data)[8 * i1 + 7] = %shr60;
; |   %shr65 = 6437 * %0 + -6437 * %1 + -11362 * %2 + 11362 * %3 + 2261 * %4 + -2261 * %5 + 9633 * %6 + -9633 * %7 + 1024  >>  11;
; |   (%data)[8 * i1 + 5] = %shr65;
; |   %shr70 = 9633 * %0 + -9633 * %1 + -2259 * %2 + 2259 * %3 + -11362 * %4 + 11362 * %5 + -6436 * %6 + 6436 * %7 + 1024  >>  11;
; |   (%data)[8 * i1 + 3] = %shr70;
; |   %shr75 = 11363 * %0 + -11363 * %1 + 9633 * %2 + -9633 * %3 + 6437 * %4 + -6437 * %5 + 2260 * %6 + -2260 * %7 + 1024  >>  11;
; |   (%data)[8 * i1 + 1] = %shr75;
; + END LOOP


; CHECK: Dump Before HIR PostVec Complete Unroll
; CHECK: DO i1


; CHECK: Dump After HIR PostVec Complete Unroll
; CHECK: DO i1


; Function Attrs: norecurse nounwind uwtable
define void @jpeg_fdct_islow(i32* nocapture %data) #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %ctr.0329 = phi i32 [ 7, %entry ], [ %dec, %for.body ]
  %dataptr.0328 = phi i32* [ %data, %entry ], [ %add.ptr, %for.body ]
  %0 = load i32, i32* %dataptr.0328, align 4
  %arrayidx1 = getelementptr inbounds i32, i32* %dataptr.0328, i64 7
  %1 = load i32, i32* %arrayidx1, align 4
  %add = add nsw i32 %1, %0
  %sub = sub nsw i32 %0, %1
  %arrayidx4 = getelementptr inbounds i32, i32* %dataptr.0328, i64 1
  %2 = load i32, i32* %arrayidx4, align 4
  %arrayidx5 = getelementptr inbounds i32, i32* %dataptr.0328, i64 6
  %3 = load i32, i32* %arrayidx5, align 4
  %add6 = add nsw i32 %3, %2
  %sub9 = sub nsw i32 %2, %3
  %arrayidx10 = getelementptr inbounds i32, i32* %dataptr.0328, i64 2
  %4 = load i32, i32* %arrayidx10, align 4
  %arrayidx11 = getelementptr inbounds i32, i32* %dataptr.0328, i64 5
  %5 = load i32, i32* %arrayidx11, align 4
  %add12 = add nsw i32 %5, %4
  %sub15 = sub nsw i32 %4, %5
  %arrayidx16 = getelementptr inbounds i32, i32* %dataptr.0328, i64 3
  %6 = load i32, i32* %arrayidx16, align 4
  %arrayidx17 = getelementptr inbounds i32, i32* %dataptr.0328, i64 4
  %7 = load i32, i32* %arrayidx17, align 4
  %add18 = add nsw i32 %7, %6
  %sub21 = sub nsw i32 %6, %7
  %add22 = add nsw i32 %add18, %add
  %sub23 = sub nsw i32 %add, %add18
  %add24 = add nsw i32 %add12, %add6
  %sub25 = sub nsw i32 %add6, %add12
  %add26 = add nsw i32 %add22, %add24
  %shl = shl i32 %add26, 2
  store i32 %shl, i32* %dataptr.0328, align 4
  %sub28 = sub nsw i32 %add22, %add24
  %shl29 = shl i32 %sub28, 2
  store i32 %shl29, i32* %arrayidx17, align 4
  %add31 = add nsw i32 %sub23, %sub25
  %mul = mul nsw i32 %add31, 4433
  %mul32 = mul nsw i32 %sub23, 6270
  %add33 = add i32 %mul32, 1024
  %add34 = add i32 %add33, %mul
  %shr = ashr i32 %add34, 11
  store i32 %shr, i32* %arrayidx10, align 4
  %mul36 = mul nsw i32 %sub25, -15137
  %add37 = add i32 %mul36, 1024
  %add38 = add i32 %add37, %mul
  %shr39 = ashr i32 %add38, 11
  store i32 %shr39, i32* %arrayidx5, align 4
  %add41 = add nsw i32 %sub21, %sub
  %add42 = add nsw i32 %sub15, %sub9
  %add43 = add nsw i32 %sub21, %sub9
  %add44 = add nsw i32 %sub15, %sub
  %add45 = add nsw i32 %add43, %add44
  %mul46 = mul nsw i32 %add45, 9633
  %mul47 = mul nsw i32 %sub21, 2446
  %mul48 = mul nsw i32 %sub15, 16819
  %mul49 = mul nsw i32 %sub9, 25172
  %mul50 = mul nsw i32 %sub, 12299
  %mul51 = mul nsw i32 %add41, -7373
  %mul52 = mul nsw i32 %add42, -20995
  %mul53 = mul nsw i32 %add43, -16069
  %mul54 = mul nsw i32 %add44, -3196
  %add55 = add nsw i32 %mul46, %mul53
  %add56 = add nsw i32 %mul46, %mul54
  %add57 = add i32 %mul47, 1024
  %add58 = add i32 %add57, %mul51
  %add59 = add i32 %add58, %add55
  %shr60 = ashr i32 %add59, 11
  store i32 %shr60, i32* %arrayidx1, align 4
  %add62 = add i32 %mul48, 1024
  %add63 = add i32 %add62, %mul52
  %add64 = add i32 %add63, %add56
  %shr65 = ashr i32 %add64, 11
  store i32 %shr65, i32* %arrayidx11, align 4
  %add67 = add i32 %mul49, 1024
  %add68 = add i32 %add67, %mul52
  %add69 = add i32 %add68, %add55
  %shr70 = ashr i32 %add69, 11
  store i32 %shr70, i32* %arrayidx16, align 4
  %add72 = add i32 %mul50, 1024
  %add73 = add i32 %add72, %mul51
  %add74 = add i32 %add73, %add56
  %shr75 = ashr i32 %add74, 11
  store i32 %shr75, i32* %arrayidx4, align 4
  %add.ptr = getelementptr inbounds i32, i32* %dataptr.0328, i64 8
  %dec = add nsw i32 %ctr.0329, -1
  %cmp = icmp sgt i32 %ctr.0329, 0
  br i1 %cmp, label %for.body, label %for.end166

for.end166:                                       ; preds = %for.body79
  ret void
}

