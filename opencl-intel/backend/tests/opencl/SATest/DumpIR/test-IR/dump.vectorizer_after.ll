; ModuleID = 'Program'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"
target triple = "i686-pc-win32"

define void @intel_median(<4 x i8> addrspace(1)* nocapture %pSrc, i32 addrspace(1)* nocapture %pDst, i32 %iImageWidth, i32 %iImageHeight) nounwind {
entry:
  %iPixels = alloca [9 x <4 x i32>], align 16
  %cmp10 = icmp sgt i32 %iImageWidth, 0
  br i1 %cmp10, label %for.cond1.preheader.lr.ph, label %for.end79

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %call = call i32 @get_global_id(i32 0) nounwind readnone
  %add = add i32 %call, 2
  %mul74 = mul nsw i32 %add, %iImageWidth
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.end66, %for.cond1.preheader.lr.ph
  %x.011 = phi i32 [ 0, %for.cond1.preheader.lr.ph ], [ %inc78, %for.end66 ]
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.cond1.preheader
  %iRow.02 = phi i32 [ -1, %for.cond1.preheader ], [ %inc28, %for.body3 ]
  %iPixelCount.01 = phi i32 [ 0, %for.cond1.preheader ], [ %inc27, %for.body3 ]
  %add4 = add i32 %add, %iRow.02
  %mul = mul nsw i32 %add4, %iImageWidth
  %add5 = add nsw i32 %mul, %x.011
  %sub = add nsw i32 %add5, -1
  %arrayidx = getelementptr inbounds <4 x i8> addrspace(1)* %pSrc, i32 %sub
  %0 = load <4 x i8> addrspace(1)* %arrayidx, align 4
  %1 = extractelement <4 x i8> %0, i32 0
  %conv = zext i8 %1 to i32
  %arrayidx6 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 %iPixelCount.01
  %2 = load <4 x i32>* %arrayidx6, align 16
  %3 = insertelement <4 x i32> %2, i32 %conv, i32 0
  %4 = extractelement <4 x i8> %0, i32 1
  %conv7 = zext i8 %4 to i32
  %5 = insertelement <4 x i32> %3, i32 %conv7, i32 1
  %6 = extractelement <4 x i8> %0, i32 2
  %conv9 = zext i8 %6 to i32
  %7 = insertelement <4 x i32> %5, i32 %conv9, i32 2
  store <4 x i32> %7, <4 x i32>* %arrayidx6, align 16
  %inc = add nsw i32 %iPixelCount.01, 1
  %arrayidx11 = getelementptr inbounds <4 x i8> addrspace(1)* %pSrc, i32 %add5
  %8 = load <4 x i8> addrspace(1)* %arrayidx11, align 4
  %9 = extractelement <4 x i8> %8, i32 0
  %conv12 = zext i8 %9 to i32
  %arrayidx13 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 %inc
  %10 = load <4 x i32>* %arrayidx13, align 16
  %11 = insertelement <4 x i32> %10, i32 %conv12, i32 0
  %12 = extractelement <4 x i8> %8, i32 1
  %conv14 = zext i8 %12 to i32
  %13 = insertelement <4 x i32> %11, i32 %conv14, i32 1
  %14 = extractelement <4 x i8> %8, i32 2
  %conv16 = zext i8 %14 to i32
  %15 = insertelement <4 x i32> %13, i32 %conv16, i32 2
  store <4 x i32> %15, <4 x i32>* %arrayidx13, align 16
  %inc18 = add nsw i32 %iPixelCount.01, 2
  %add19 = add nsw i32 %add5, 1
  %arrayidx20 = getelementptr inbounds <4 x i8> addrspace(1)* %pSrc, i32 %add19
  %16 = load <4 x i8> addrspace(1)* %arrayidx20, align 4
  %17 = extractelement <4 x i8> %16, i32 0
  %conv21 = zext i8 %17 to i32
  %arrayidx22 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 %inc18
  %18 = load <4 x i32>* %arrayidx22, align 16
  %19 = insertelement <4 x i32> %18, i32 %conv21, i32 0
  %20 = extractelement <4 x i8> %16, i32 1
  %conv23 = zext i8 %20 to i32
  %21 = insertelement <4 x i32> %19, i32 %conv23, i32 1
  %22 = extractelement <4 x i8> %16, i32 2
  %conv25 = zext i8 %22 to i32
  %23 = insertelement <4 x i32> %21, i32 %conv25, i32 2
  store <4 x i32> %23, <4 x i32>* %arrayidx22, align 16
  %inc27 = add nsw i32 %iPixelCount.01, 3
  %inc28 = add nsw i32 %iRow.02, 1
  %exitcond = icmp eq i32 %inc28, 2
  br i1 %exitcond, label %for.cond29.loopexit, label %for.body3

for.cond29.loopexit:                              ; preds = %for.body3
  %arrayidx38 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 0
  br label %for.cond34.preheader

for.cond34.preheader:                             ; preds = %for.cond34.preheader, %for.cond29.loopexit
  %iSearch.09 = phi i32 [ 0, %for.cond29.loopexit ], [ %inc65, %for.cond34.preheader ]
  %iYes.08 = phi <4 x i32> [ <i32 128, i32 128, i32 128, i32 128>, %for.cond29.loopexit ], [ %shr, %for.cond34.preheader ]
  %iMin.07 = phi <4 x i32> [ zeroinitializer, %for.cond29.loopexit ], [ %or, %for.cond34.preheader ]
  %iMax.06 = phi <4 x i32> [ <i32 255, i32 255, i32 255, i32 255>, %for.cond29.loopexit ], [ %or62, %for.cond34.preheader ]
  %24 = load <4 x i32>* %arrayidx38, align 16
  %cmp39 = icmp slt <4 x i32> %iYes.08, %24
  %sext = sext <4 x i1> %cmp39 to <4 x i32>
  %arrayidx42 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 1
  %25 = load <4 x i32>* %arrayidx42, align 16
  %cmp43 = icmp slt <4 x i32> %iYes.08, %25
  %sext44 = sext <4 x i1> %cmp43 to <4 x i32>
  %add45 = add <4 x i32> %sext, %sext44
  %arrayidx47 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 2
  %26 = load <4 x i32>* %arrayidx47, align 16
  %cmp48 = icmp slt <4 x i32> %iYes.08, %26
  %sext49 = sext <4 x i1> %cmp48 to <4 x i32>
  %add50 = add <4 x i32> %add45, %sext49
  %arrayidx38.1 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 3
  %27 = load <4 x i32>* %arrayidx38.1, align 16
  %cmp39.1 = icmp slt <4 x i32> %iYes.08, %27
  %sext.1 = sext <4 x i1> %cmp39.1 to <4 x i32>
  %add40.1 = add <4 x i32> %add50, %sext.1
  %arrayidx42.1 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 4
  %28 = load <4 x i32>* %arrayidx42.1, align 16
  %cmp43.1 = icmp slt <4 x i32> %iYes.08, %28
  %sext44.1 = sext <4 x i1> %cmp43.1 to <4 x i32>
  %add45.1 = add <4 x i32> %add40.1, %sext44.1
  %arrayidx47.1 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 5
  %29 = load <4 x i32>* %arrayidx47.1, align 16
  %cmp48.1 = icmp slt <4 x i32> %iYes.08, %29
  %sext49.1 = sext <4 x i1> %cmp48.1 to <4 x i32>
  %add50.1 = add <4 x i32> %add45.1, %sext49.1
  %arrayidx38.2 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 6
  %30 = load <4 x i32>* %arrayidx38.2, align 16
  %cmp39.2 = icmp slt <4 x i32> %iYes.08, %30
  %sext.2 = sext <4 x i1> %cmp39.2 to <4 x i32>
  %add40.2 = add <4 x i32> %add50.1, %sext.2
  %arrayidx42.2 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 7
  %31 = load <4 x i32>* %arrayidx42.2, align 16
  %cmp43.2 = icmp slt <4 x i32> %iYes.08, %31
  %sext44.2 = sext <4 x i1> %cmp43.2 to <4 x i32>
  %add45.2 = add <4 x i32> %add40.2, %sext44.2
  %arrayidx47.2 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 8
  %32 = load <4 x i32>* %arrayidx47.2, align 16
  %cmp48.2 = icmp slt <4 x i32> %iYes.08, %32
  %sext49.2 = sext <4 x i1> %cmp48.2 to <4 x i32>
  %add50.2 = add <4 x i32> %add45.2, %sext49.2
  %sub55 = sub <4 x i32> zeroinitializer, %add50.2
  %cmp56 = icmp sgt <4 x i32> %sub55, <i32 4, i32 4, i32 4, i32 4>
  %sext57 = sext <4 x i1> %cmp56 to <4 x i32>
  %and = and <4 x i32> %iYes.08, %sext57
  %neg = xor <4 x i32> %sext57, <i32 -1, i32 -1, i32 -1, i32 -1>
  %and58 = and <4 x i32> %iMin.07, %neg
  %or = or <4 x i32> %and, %and58
  %and60 = and <4 x i32> %iYes.08, %neg
  %and61 = and <4 x i32> %iMax.06, %sext57
  %or62 = or <4 x i32> %and60, %and61
  %add63 = add <4 x i32> %or62, %or
  %shr = ashr <4 x i32> %add63, <i32 1, i32 1, i32 1, i32 1>
  %inc65 = add nsw i32 %iSearch.09, 1
  %exitcond13 = icmp eq i32 %inc65, 8
  br i1 %exitcond13, label %for.end66, label %for.cond34.preheader

for.end66:                                        ; preds = %for.cond34.preheader
  %33 = extractelement <4 x i32> %shr, i32 0
  %and67 = and i32 %33, 255
  %34 = extractelement <4 x i32> %shr, i32 1
  %shl = shl i32 %34, 8
  %and68 = and i32 %shl, 65280
  %35 = extractelement <4 x i32> %shr, i32 2
  %shl70 = shl i32 %35, 16
  %and71 = and i32 %shl70, 16711680
  %or69 = or i32 %and71, %and67
  %or72 = or i32 %or69, %and68
  %add75 = add nsw i32 %x.011, %mul74
  %arrayidx76 = getelementptr inbounds i32 addrspace(1)* %pDst, i32 %add75
  store i32 %or72, i32 addrspace(1)* %arrayidx76, align 4
  %inc78 = add nsw i32 %x.011, 1
  %exitcond14 = icmp eq i32 %inc78, %iImageWidth
  br i1 %exitcond14, label %for.end79.loopexit, label %for.cond1.preheader

for.end79.loopexit:                               ; preds = %for.end66
  br label %for.end79

for.end79:                                        ; preds = %for.end79.loopexit, %entry
  ret void
}

declare i32 @get_global_id(i32) nounwind readnone

define void @intel_median_scalar(i8 addrspace(1)* nocapture %pSrc, i32 addrspace(1)* nocapture %pDst, i32 %iImageWidth, i32 %iImageHeight) nounwind {
entry:
  %iResult = alloca [4 x i32], align 4
  %iPixels = alloca [9 x i32], align 4
  %call = call i32 @get_global_id(i32 0) nounwind readnone
  %cmp11 = icmp sgt i32 %iImageWidth, 0
  br i1 %cmp11, label %for.cond1.preheader.lr.ph, label %for.end82

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %add = add i32 %call, 2
  %arrayidx69 = getelementptr inbounds [4 x i32]* %iResult, i32 0, i32 0
  %arrayidx70 = getelementptr inbounds [4 x i32]* %iResult, i32 0, i32 1
  %arrayidx72 = getelementptr inbounds [4 x i32]* %iResult, i32 0, i32 2
  %mul77 = mul nsw i32 %add, %iImageWidth
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.end68, %for.cond1.preheader.lr.ph
  %x.012 = phi i32 [ 0, %for.cond1.preheader.lr.ph ], [ %inc81, %for.end68 ]
  br label %for.cond4.preheader

for.cond4.preheader:                              ; preds = %for.end64, %for.cond1.preheader
  %ch.010 = phi i32 [ 0, %for.cond1.preheader ], [ %inc67, %for.end64 ]
  %mul9 = add i32 %ch.010, -4
  %mul19 = add i32 %ch.010, 4
  %add7 = add i32 %call, 1
  %mul = mul nsw i32 %add7, %iImageWidth
  %add8 = add nsw i32 %mul, %x.012
  %sub = shl i32 %add8, 2
  %add10 = add i32 %mul9, %sub
  %arrayidx = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %add10
  %0 = load i8 addrspace(1)* %arrayidx, align 1
  %conv = zext i8 %0 to i32
  %arrayidx11 = getelementptr inbounds [9 x i32]* %iPixels, i32 0, i32 0
  store i32 %conv, i32* %arrayidx11, align 4
  %add13 = add nsw i32 %sub, %ch.010
  %arrayidx14 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %add13
  %1 = load i8 addrspace(1)* %arrayidx14, align 1
  %conv15 = zext i8 %1 to i32
  %arrayidx16 = getelementptr inbounds [9 x i32]* %iPixels, i32 0, i32 1
  store i32 %conv15, i32* %arrayidx16, align 4
  %add20 = add i32 %mul19, %sub
  %arrayidx21 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %add20
  %2 = load i8 addrspace(1)* %arrayidx21, align 1
  %conv22 = zext i8 %2 to i32
  %arrayidx23 = getelementptr inbounds [9 x i32]* %iPixels, i32 0, i32 2
  store i32 %conv22, i32* %arrayidx23, align 4
  %add8.1 = add nsw i32 %mul77, %x.012
  %sub.1 = shl i32 %add8.1, 2
  %add10.1 = add i32 %mul9, %sub.1
  %arrayidx.1 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %add10.1
  %3 = load i8 addrspace(1)* %arrayidx.1, align 1
  %conv.1 = zext i8 %3 to i32
  %arrayidx11.1 = getelementptr inbounds [9 x i32]* %iPixels, i32 0, i32 3
  store i32 %conv.1, i32* %arrayidx11.1, align 4
  %add13.1 = add nsw i32 %sub.1, %ch.010
  %arrayidx14.1 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %add13.1
  %4 = load i8 addrspace(1)* %arrayidx14.1, align 1
  %conv15.1 = zext i8 %4 to i32
  %arrayidx16.1 = getelementptr inbounds [9 x i32]* %iPixels, i32 0, i32 4
  store i32 %conv15.1, i32* %arrayidx16.1, align 4
  %add20.1 = add i32 %mul19, %sub.1
  %arrayidx21.1 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %add20.1
  %5 = load i8 addrspace(1)* %arrayidx21.1, align 1
  %conv22.1 = zext i8 %5 to i32
  %arrayidx23.1 = getelementptr inbounds [9 x i32]* %iPixels, i32 0, i32 5
  store i32 %conv22.1, i32* %arrayidx23.1, align 4
  %add7.2 = add i32 %call, 3
  %mul.2 = mul nsw i32 %add7.2, %iImageWidth
  %add8.2 = add nsw i32 %mul.2, %x.012
  %sub.2 = shl i32 %add8.2, 2
  %add10.2 = add i32 %mul9, %sub.2
  %arrayidx.2 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %add10.2
  %6 = load i8 addrspace(1)* %arrayidx.2, align 1
  %conv.2 = zext i8 %6 to i32
  %arrayidx11.2 = getelementptr inbounds [9 x i32]* %iPixels, i32 0, i32 6
  store i32 %conv.2, i32* %arrayidx11.2, align 4
  %add13.2 = add nsw i32 %sub.2, %ch.010
  %arrayidx14.2 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %add13.2
  %7 = load i8 addrspace(1)* %arrayidx14.2, align 1
  %conv15.2 = zext i8 %7 to i32
  %arrayidx16.2 = getelementptr inbounds [9 x i32]* %iPixels, i32 0, i32 7
  store i32 %conv15.2, i32* %arrayidx16.2, align 4
  %add20.2 = add i32 %mul19, %sub.2
  %arrayidx21.2 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %add20.2
  %8 = load i8 addrspace(1)* %arrayidx21.2, align 1
  %conv22.2 = zext i8 %8 to i32
  %arrayidx23.2 = getelementptr inbounds [9 x i32]* %iPixels, i32 0, i32 8
  store i32 %conv22.2, i32* %arrayidx23.2, align 4
  br label %for.cond31.preheader

for.cond31.preheader:                             ; preds = %for.cond31.preheader, %for.cond4.preheader
  %iSearch.09 = phi i32 [ 0, %for.cond4.preheader ], [ %inc63, %for.cond31.preheader ]
  %iMax.08 = phi i32 [ 255, %for.cond4.preheader ], [ %cond60, %for.cond31.preheader ]
  %iMin.07 = phi i32 [ 0, %for.cond4.preheader ], [ %cond, %for.cond31.preheader ]
  %iYes.06 = phi i32 [ 128, %for.cond4.preheader ], [ %shr, %for.cond31.preheader ]
  %9 = load i32* %arrayidx11, align 4
  %cmp36 = icmp slt i32 %iYes.06, %9
  %conv37 = zext i1 %cmp36 to i32
  %10 = load i32* %arrayidx16, align 4
  %cmp41 = icmp slt i32 %iYes.06, %10
  %conv42 = zext i1 %cmp41 to i32
  %11 = load i32* %arrayidx23, align 4
  %cmp46 = icmp slt i32 %iYes.06, %11
  %conv47 = zext i1 %cmp46 to i32
  %add43 = add i32 %conv37, %conv42
  %add48 = add i32 %add43, %conv47
  %12 = load i32* %arrayidx11.1, align 4
  %cmp36.1 = icmp slt i32 %iYes.06, %12
  %conv37.1 = zext i1 %cmp36.1 to i32
  %13 = load i32* %arrayidx16.1, align 4
  %cmp41.1 = icmp slt i32 %iYes.06, %13
  %conv42.1 = zext i1 %cmp41.1 to i32
  %14 = load i32* %arrayidx23.1, align 4
  %cmp46.1 = icmp slt i32 %iYes.06, %14
  %conv47.1 = zext i1 %cmp46.1 to i32
  %add38.1 = add i32 %conv37.1, %add48
  %add43.1 = add i32 %add38.1, %conv42.1
  %add48.1 = add i32 %add43.1, %conv47.1
  %15 = load i32* %arrayidx11.2, align 4
  %cmp36.2 = icmp slt i32 %iYes.06, %15
  %conv37.2 = zext i1 %cmp36.2 to i32
  %16 = load i32* %arrayidx16.2, align 4
  %cmp41.2 = icmp slt i32 %iYes.06, %16
  %conv42.2 = zext i1 %cmp41.2 to i32
  %cmp46.2 = icmp slt i32 %iYes.06, %conv22.2
  %conv47.2 = zext i1 %cmp46.2 to i32
  %add38.2 = add i32 %conv37.2, %add48.1
  %add43.2 = add i32 %add38.2, %conv42.2
  %add48.2 = add i32 %add43.2, %conv47.2
  %cmp53 = icmp sgt i32 %add48.2, 4
  %cond = select i1 %cmp53, i32 %iYes.06, i32 %iMin.07
  %cmp55 = icmp slt i32 %add48.2, 5
  %cond60 = select i1 %cmp55, i32 %iYes.06, i32 %iMax.08
  %add61 = add nsw i32 %cond60, %cond
  %shr = ashr i32 %add61, 1
  %inc63 = add nsw i32 %iSearch.09, 1
  %exitcond = icmp eq i32 %inc63, 8
  br i1 %exitcond, label %for.end64, label %for.cond31.preheader

for.end64:                                        ; preds = %for.cond31.preheader
  %arrayidx65 = getelementptr inbounds [4 x i32]* %iResult, i32 0, i32 %ch.010
  store i32 %shr, i32* %arrayidx65, align 4
  %inc67 = add nsw i32 %ch.010, 1
  %exitcond13 = icmp eq i32 %inc67, 3
  br i1 %exitcond13, label %for.end68, label %for.cond4.preheader

for.end68:                                        ; preds = %for.end64
  %17 = load i32* %arrayidx69, align 4
  %and = and i32 %17, 255
  %18 = load i32* %arrayidx70, align 4
  %shl = shl i32 %18, 8
  %and71 = and i32 %shl, 65280
  %19 = load i32* %arrayidx72, align 4
  %shl73 = shl i32 %19, 16
  %and74 = and i32 %shl73, 16711680
  %or = or i32 %and71, %and
  %or75 = or i32 %or, %and74
  %add78 = add nsw i32 %x.012, %mul77
  %arrayidx79 = getelementptr inbounds i32 addrspace(1)* %pDst, i32 %add78
  store i32 %or75, i32 addrspace(1)* %arrayidx79, align 4
  %inc81 = add nsw i32 %x.012, 1
  %exitcond14 = icmp eq i32 %inc81, %iImageWidth
  br i1 %exitcond14, label %for.end82.loopexit, label %for.cond1.preheader

for.end82.loopexit:                               ; preds = %for.end68
  br label %for.end82

for.end82:                                        ; preds = %for.end82.loopexit, %entry
  ret void
}

define void @__Vectorized_.intel_median_scalar(i8 addrspace(1)* nocapture %pSrc, i32 addrspace(1)* nocapture %pDst, i32 %iImageWidth, i32 %iImageHeight) nounwind {
entry:
  %temp = insertelement <4 x i32> undef, i32 %iImageWidth, i32 0
  %vector = shufflevector <4 x i32> %temp, <4 x i32> undef, <4 x i32> zeroinitializer
  %0 = alloca [4 x i32], align 4
  %1 = alloca [4 x i32], align 4
  %2 = alloca [4 x i32], align 4
  %3 = alloca [4 x i32], align 4
  %call = call i32 @get_global_id(i32 0) nounwind readnone
  %broadcast1 = insertelement <4 x i32> undef, i32 %call, i32 0
  %broadcast2 = shufflevector <4 x i32> %broadcast1, <4 x i32> undef, <4 x i32> zeroinitializer
  %cmp11 = icmp sgt i32 %iImageWidth, 0
  br i1 %cmp11, label %for.cond1.preheader.lr.ph, label %for.end82

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %add25 = add <4 x i32> %broadcast2, <i32 2, i32 3, i32 4, i32 5>
  %4 = getelementptr inbounds [4 x i32]* %0, i32 0, i32 0
  %5 = getelementptr inbounds [4 x i32]* %1, i32 0, i32 0
  %6 = getelementptr inbounds [4 x i32]* %2, i32 0, i32 0
  %7 = getelementptr inbounds [4 x i32]* %3, i32 0, i32 0
  %8 = getelementptr inbounds [4 x i32]* %0, i32 0, i32 1
  %9 = getelementptr inbounds [4 x i32]* %1, i32 0, i32 1
  %10 = getelementptr inbounds [4 x i32]* %2, i32 0, i32 1
  %11 = getelementptr inbounds [4 x i32]* %3, i32 0, i32 1
  %12 = getelementptr inbounds [4 x i32]* %0, i32 0, i32 2
  %13 = getelementptr inbounds [4 x i32]* %1, i32 0, i32 2
  %14 = getelementptr inbounds [4 x i32]* %2, i32 0, i32 2
  %15 = getelementptr inbounds [4 x i32]* %3, i32 0, i32 2
  %mul7726 = mul nsw <4 x i32> %add25, %vector
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.end68, %for.cond1.preheader.lr.ph
  %x.012 = phi i32 [ 0, %for.cond1.preheader.lr.ph ], [ %inc81, %for.end68 ]
  %temp29 = insertelement <4 x i32> undef, i32 %x.012, i32 0
  %vector30 = shufflevector <4 x i32> %temp29, <4 x i32> undef, <4 x i32> zeroinitializer
  br label %for.cond4.preheader

for.cond4.preheader:                              ; preds = %for.end64, %for.cond1.preheader
  %ch.010 = phi i32 [ 0, %for.cond1.preheader ], [ %inc67, %for.end64 ]
  %temp39 = insertelement <4 x i32> undef, i32 %ch.010, i32 0
  %vector40 = shufflevector <4 x i32> %temp39, <4 x i32> undef, <4 x i32> zeroinitializer
  %mul9 = add i32 %ch.010, -4
  %temp33 = insertelement <4 x i32> undef, i32 %mul9, i32 0
  %vector34 = shufflevector <4 x i32> %temp33, <4 x i32> undef, <4 x i32> zeroinitializer
  %mul19 = add i32 %ch.010, 4
  %temp46 = insertelement <4 x i32> undef, i32 %mul19, i32 0
  %vector47 = shufflevector <4 x i32> %temp46, <4 x i32> undef, <4 x i32> zeroinitializer
  %add727 = add <4 x i32> %broadcast2, <i32 1, i32 2, i32 3, i32 4>
  %mul28 = mul nsw <4 x i32> %add727, %vector
  %add831 = add nsw <4 x i32> %mul28, %vector30
  %sub32 = shl <4 x i32> %add831, <i32 2, i32 2, i32 2, i32 2>
  %add1035 = add <4 x i32> %vector34, %sub32
  %extract = extractelement <4 x i32> %add1035, i32 0
  %extract36 = extractelement <4 x i32> %add1035, i32 1
  %extract37 = extractelement <4 x i32> %add1035, i32 2
  %extract38 = extractelement <4 x i32> %add1035, i32 3
  %16 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract
  %17 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract36
  %18 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract37
  %19 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract38
  %20 = load i8 addrspace(1)* %16, align 1
  %21 = load i8 addrspace(1)* %17, align 1
  %22 = load i8 addrspace(1)* %18, align 1
  %23 = load i8 addrspace(1)* %19, align 1
  %temp.vect95 = insertelement <4 x i8> undef, i8 %20, i32 0
  %temp.vect96 = insertelement <4 x i8> %temp.vect95, i8 %21, i32 1
  %temp.vect97 = insertelement <4 x i8> %temp.vect96, i8 %22, i32 2
  %temp.vect98 = insertelement <4 x i8> %temp.vect97, i8 %23, i32 3
  %add1341 = add nsw <4 x i32> %sub32, %vector40
  %extract42 = extractelement <4 x i32> %add1341, i32 0
  %extract43 = extractelement <4 x i32> %add1341, i32 1
  %extract44 = extractelement <4 x i32> %add1341, i32 2
  %extract45 = extractelement <4 x i32> %add1341, i32 3
  %24 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract42
  %25 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract43
  %26 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract44
  %27 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract45
  %28 = load i8 addrspace(1)* %24, align 1
  %29 = load i8 addrspace(1)* %25, align 1
  %30 = load i8 addrspace(1)* %26, align 1
  %31 = load i8 addrspace(1)* %27, align 1
  %temp.vect100 = insertelement <4 x i8> undef, i8 %28, i32 0
  %temp.vect101 = insertelement <4 x i8> %temp.vect100, i8 %29, i32 1
  %temp.vect102 = insertelement <4 x i8> %temp.vect101, i8 %30, i32 2
  %temp.vect103 = insertelement <4 x i8> %temp.vect102, i8 %31, i32 3
  %add2048 = add <4 x i32> %vector47, %sub32
  %extract49 = extractelement <4 x i32> %add2048, i32 0
  %extract50 = extractelement <4 x i32> %add2048, i32 1
  %extract51 = extractelement <4 x i32> %add2048, i32 2
  %extract52 = extractelement <4 x i32> %add2048, i32 3
  %32 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract49
  %33 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract50
  %34 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract51
  %35 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract52
  %36 = load i8 addrspace(1)* %32, align 1
  %37 = load i8 addrspace(1)* %33, align 1
  %38 = load i8 addrspace(1)* %34, align 1
  %39 = load i8 addrspace(1)* %35, align 1
  %temp.vect105 = insertelement <4 x i8> undef, i8 %36, i32 0
  %temp.vect106 = insertelement <4 x i8> %temp.vect105, i8 %37, i32 1
  %temp.vect107 = insertelement <4 x i8> %temp.vect106, i8 %38, i32 2
  %temp.vect108 = insertelement <4 x i8> %temp.vect107, i8 %39, i32 3
  %add8.153 = add nsw <4 x i32> %mul7726, %vector30
  %sub.154 = shl <4 x i32> %add8.153, <i32 2, i32 2, i32 2, i32 2>
  %add10.155 = add <4 x i32> %vector34, %sub.154
  %extract56 = extractelement <4 x i32> %add10.155, i32 0
  %extract57 = extractelement <4 x i32> %add10.155, i32 1
  %extract58 = extractelement <4 x i32> %add10.155, i32 2
  %extract59 = extractelement <4 x i32> %add10.155, i32 3
  %40 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract56
  %41 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract57
  %42 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract58
  %43 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract59
  %44 = load i8 addrspace(1)* %40, align 1
  %45 = load i8 addrspace(1)* %41, align 1
  %46 = load i8 addrspace(1)* %42, align 1
  %47 = load i8 addrspace(1)* %43, align 1
  %temp.vect112 = insertelement <4 x i8> undef, i8 %44, i32 0
  %temp.vect113 = insertelement <4 x i8> %temp.vect112, i8 %45, i32 1
  %temp.vect114 = insertelement <4 x i8> %temp.vect113, i8 %46, i32 2
  %temp.vect115 = insertelement <4 x i8> %temp.vect114, i8 %47, i32 3
  %add13.160 = add nsw <4 x i32> %sub.154, %vector40
  %extract61 = extractelement <4 x i32> %add13.160, i32 0
  %extract62 = extractelement <4 x i32> %add13.160, i32 1
  %extract63 = extractelement <4 x i32> %add13.160, i32 2
  %extract64 = extractelement <4 x i32> %add13.160, i32 3
  %48 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract61
  %49 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract62
  %50 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract63
  %51 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract64
  %52 = load i8 addrspace(1)* %48, align 1
  %53 = load i8 addrspace(1)* %49, align 1
  %54 = load i8 addrspace(1)* %50, align 1
  %55 = load i8 addrspace(1)* %51, align 1
  %temp.vect117 = insertelement <4 x i8> undef, i8 %52, i32 0
  %temp.vect118 = insertelement <4 x i8> %temp.vect117, i8 %53, i32 1
  %temp.vect119 = insertelement <4 x i8> %temp.vect118, i8 %54, i32 2
  %temp.vect120 = insertelement <4 x i8> %temp.vect119, i8 %55, i32 3
  %add20.165 = add <4 x i32> %vector47, %sub.154
  %extract66 = extractelement <4 x i32> %add20.165, i32 0
  %extract67 = extractelement <4 x i32> %add20.165, i32 1
  %extract68 = extractelement <4 x i32> %add20.165, i32 2
  %extract69 = extractelement <4 x i32> %add20.165, i32 3
  %56 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract66
  %57 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract67
  %58 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract68
  %59 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract69
  %60 = load i8 addrspace(1)* %56, align 1
  %61 = load i8 addrspace(1)* %57, align 1
  %62 = load i8 addrspace(1)* %58, align 1
  %63 = load i8 addrspace(1)* %59, align 1
  %temp.vect122 = insertelement <4 x i8> undef, i8 %60, i32 0
  %temp.vect123 = insertelement <4 x i8> %temp.vect122, i8 %61, i32 1
  %temp.vect124 = insertelement <4 x i8> %temp.vect123, i8 %62, i32 2
  %temp.vect125 = insertelement <4 x i8> %temp.vect124, i8 %63, i32 3
  %add7.270 = add <4 x i32> %broadcast2, <i32 3, i32 4, i32 5, i32 6>
  %mul.271 = mul nsw <4 x i32> %add7.270, %vector
  %add8.272 = add nsw <4 x i32> %mul.271, %vector30
  %sub.273 = shl <4 x i32> %add8.272, <i32 2, i32 2, i32 2, i32 2>
  %add10.274 = add <4 x i32> %vector34, %sub.273
  %extract75 = extractelement <4 x i32> %add10.274, i32 0
  %extract76 = extractelement <4 x i32> %add10.274, i32 1
  %extract77 = extractelement <4 x i32> %add10.274, i32 2
  %extract78 = extractelement <4 x i32> %add10.274, i32 3
  %64 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract75
  %65 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract76
  %66 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract77
  %67 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract78
  %68 = load i8 addrspace(1)* %64, align 1
  %69 = load i8 addrspace(1)* %65, align 1
  %70 = load i8 addrspace(1)* %66, align 1
  %71 = load i8 addrspace(1)* %67, align 1
  %temp.vect130 = insertelement <4 x i8> undef, i8 %68, i32 0
  %temp.vect131 = insertelement <4 x i8> %temp.vect130, i8 %69, i32 1
  %temp.vect132 = insertelement <4 x i8> %temp.vect131, i8 %70, i32 2
  %temp.vect133 = insertelement <4 x i8> %temp.vect132, i8 %71, i32 3
  %add13.279 = add nsw <4 x i32> %sub.273, %vector40
  %extract80 = extractelement <4 x i32> %add13.279, i32 0
  %extract81 = extractelement <4 x i32> %add13.279, i32 1
  %extract82 = extractelement <4 x i32> %add13.279, i32 2
  %extract83 = extractelement <4 x i32> %add13.279, i32 3
  %72 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract80
  %73 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract81
  %74 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract82
  %75 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract83
  %76 = load i8 addrspace(1)* %72, align 1
  %77 = load i8 addrspace(1)* %73, align 1
  %78 = load i8 addrspace(1)* %74, align 1
  %79 = load i8 addrspace(1)* %75, align 1
  %temp.vect135 = insertelement <4 x i8> undef, i8 %76, i32 0
  %temp.vect136 = insertelement <4 x i8> %temp.vect135, i8 %77, i32 1
  %temp.vect137 = insertelement <4 x i8> %temp.vect136, i8 %78, i32 2
  %temp.vect138 = insertelement <4 x i8> %temp.vect137, i8 %79, i32 3
  %add20.284 = add <4 x i32> %vector47, %sub.273
  %extract85 = extractelement <4 x i32> %add20.284, i32 0
  %extract86 = extractelement <4 x i32> %add20.284, i32 1
  %extract87 = extractelement <4 x i32> %add20.284, i32 2
  %extract88 = extractelement <4 x i32> %add20.284, i32 3
  %80 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract85
  %81 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract86
  %82 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract87
  %83 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract88
  %84 = load i8 addrspace(1)* %80, align 1
  %85 = load i8 addrspace(1)* %81, align 1
  %86 = load i8 addrspace(1)* %82, align 1
  %87 = load i8 addrspace(1)* %83, align 1
  %temp.vect = insertelement <4 x i8> undef, i8 %84, i32 0
  %temp.vect89 = insertelement <4 x i8> %temp.vect, i8 %85, i32 1
  %temp.vect90 = insertelement <4 x i8> %temp.vect89, i8 %86, i32 2
  %temp.vect91 = insertelement <4 x i8> %temp.vect90, i8 %87, i32 3
  %conv22.292 = zext <4 x i8> %temp.vect91 to <4 x i32>
  br label %for.cond31.preheader

for.cond31.preheader:                             ; preds = %for.cond31.preheader, %for.cond4.preheader
  %iSearch.09 = phi i32 [ 0, %for.cond4.preheader ], [ %inc63, %for.cond31.preheader ]
  %vectorPHI = phi <4 x i32> [ <i32 255, i32 255, i32 255, i32 255>, %for.cond4.preheader ], [ %116, %for.cond31.preheader ]
  %vectorPHI93 = phi <4 x i32> [ zeroinitializer, %for.cond4.preheader ], [ %110, %for.cond31.preheader ]
  %vectorPHI94 = phi <4 x i32> [ <i32 128, i32 128, i32 128, i32 128>, %for.cond4.preheader ], [ %shr147, %for.cond31.preheader ]
  %88 = zext <4 x i8> %temp.vect98 to <4 x i32>
  %89 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %88, <4 x i32> %vectorPHI94)
  %msb = and <4 x i32> %89, <i32 1, i32 1, i32 1, i32 1>
  %90 = zext <4 x i8> %temp.vect103 to <4 x i32>
  %91 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %90, <4 x i32> %vectorPHI94)
  %msb184 = and <4 x i32> %91, <i32 1, i32 1, i32 1, i32 1>
  %92 = zext <4 x i8> %temp.vect108 to <4 x i32>
  %93 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %92, <4 x i32> %vectorPHI94)
  %msb186 = and <4 x i32> %93, <i32 1, i32 1, i32 1, i32 1>
  %add43110 = add <4 x i32> %msb, %msb184
  %add48111 = add <4 x i32> %add43110, %msb186
  %94 = zext <4 x i8> %temp.vect115 to <4 x i32>
  %95 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %94, <4 x i32> %vectorPHI94)
  %msb188 = and <4 x i32> %95, <i32 1, i32 1, i32 1, i32 1>
  %96 = zext <4 x i8> %temp.vect120 to <4 x i32>
  %97 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %96, <4 x i32> %vectorPHI94)
  %msb190 = and <4 x i32> %97, <i32 1, i32 1, i32 1, i32 1>
  %98 = zext <4 x i8> %temp.vect125 to <4 x i32>
  %99 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %98, <4 x i32> %vectorPHI94)
  %msb192 = and <4 x i32> %99, <i32 1, i32 1, i32 1, i32 1>
  %add38.1127 = add <4 x i32> %msb188, %add48111
  %add43.1128 = add <4 x i32> %add38.1127, %msb190
  %add48.1129 = add <4 x i32> %add43.1128, %msb192
  %100 = zext <4 x i8> %temp.vect133 to <4 x i32>
  %101 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %100, <4 x i32> %vectorPHI94)
  %msb194 = and <4 x i32> %101, <i32 1, i32 1, i32 1, i32 1>
  %102 = zext <4 x i8> %temp.vect138 to <4 x i32>
  %103 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %102, <4 x i32> %vectorPHI94)
  %msb196 = and <4 x i32> %103, <i32 1, i32 1, i32 1, i32 1>
  %104 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %conv22.292, <4 x i32> %vectorPHI94)
  %msb198 = and <4 x i32> %104, <i32 1, i32 1, i32 1, i32 1>
  %add38.2141 = add <4 x i32> %msb194, %add48.1129
  %add43.2142 = add <4 x i32> %add38.2141, %msb196
  %add48.2143 = add <4 x i32> %add43.2142, %msb198
  %105 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %add48.2143, <4 x i32> <i32 4, i32 4, i32 4, i32 4>)
  %106 = bitcast <4 x i32> %vectorPHI94 to <4 x float>
  %107 = bitcast <4 x i32> %vectorPHI93 to <4 x float>
  %108 = bitcast <4 x i32> %105 to <4 x float>
  %109 = call <4 x float> @llvm.x86.sse41.blendvps(<4 x float> %107, <4 x float> %106, <4 x float> %108)
  %110 = bitcast <4 x float> %109 to <4 x i32>
  %111 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> <i32 5, i32 5, i32 5, i32 5>, <4 x i32> %add48.2143)
  %112 = bitcast <4 x i32> %vectorPHI94 to <4 x float>
  %113 = bitcast <4 x i32> %vectorPHI to <4 x float>
  %114 = bitcast <4 x i32> %111 to <4 x float>
  %115 = call <4 x float> @llvm.x86.sse41.blendvps(<4 x float> %113, <4 x float> %112, <4 x float> %114)
  %116 = bitcast <4 x float> %115 to <4 x i32>
  %add61146 = add nsw <4 x i32> %116, %110
  %shr147 = ashr <4 x i32> %add61146, <i32 1, i32 1, i32 1, i32 1>
  %inc63 = add nsw i32 %iSearch.09, 1
  %exitcond = icmp eq i32 %inc63, 8
  br i1 %exitcond, label %for.end64, label %for.cond31.preheader

for.end64:                                        ; preds = %for.cond31.preheader
  %extract151 = extractelement <4 x i32> %shr147, i32 3
  %extract150 = extractelement <4 x i32> %shr147, i32 2
  %extract149 = extractelement <4 x i32> %shr147, i32 1
  %extract148 = extractelement <4 x i32> %shr147, i32 0
  %117 = getelementptr inbounds [4 x i32]* %0, i32 0, i32 %ch.010
  %118 = getelementptr inbounds [4 x i32]* %1, i32 0, i32 %ch.010
  %119 = getelementptr inbounds [4 x i32]* %2, i32 0, i32 %ch.010
  %120 = getelementptr inbounds [4 x i32]* %3, i32 0, i32 %ch.010
  store i32 %extract148, i32* %117, align 4
  store i32 %extract149, i32* %118, align 4
  store i32 %extract150, i32* %119, align 4
  store i32 %extract151, i32* %120, align 4
  %inc67 = add nsw i32 %ch.010, 1
  %exitcond13 = icmp eq i32 %inc67, 3
  br i1 %exitcond13, label %for.end68, label %for.cond4.preheader

for.end68:                                        ; preds = %for.end64
  %121 = load i32* %4, align 4
  %122 = load i32* %5, align 4
  %123 = load i32* %6, align 4
  %124 = load i32* %7, align 4
  %temp.vect152 = insertelement <4 x i32> undef, i32 %121, i32 0
  %temp.vect153 = insertelement <4 x i32> %temp.vect152, i32 %122, i32 1
  %temp.vect154 = insertelement <4 x i32> %temp.vect153, i32 %123, i32 2
  %temp.vect155 = insertelement <4 x i32> %temp.vect154, i32 %124, i32 3
  %and156 = and <4 x i32> %temp.vect155, <i32 255, i32 255, i32 255, i32 255>
  %125 = load i32* %8, align 4
  %126 = load i32* %9, align 4
  %127 = load i32* %10, align 4
  %128 = load i32* %11, align 4
  %temp.vect157 = insertelement <4 x i32> undef, i32 %125, i32 0
  %temp.vect158 = insertelement <4 x i32> %temp.vect157, i32 %126, i32 1
  %temp.vect159 = insertelement <4 x i32> %temp.vect158, i32 %127, i32 2
  %temp.vect160 = insertelement <4 x i32> %temp.vect159, i32 %128, i32 3
  %shl161 = shl <4 x i32> %temp.vect160, <i32 8, i32 8, i32 8, i32 8>
  %and71162 = and <4 x i32> %shl161, <i32 65280, i32 65280, i32 65280, i32 65280>
  %129 = load i32* %12, align 4
  %130 = load i32* %13, align 4
  %131 = load i32* %14, align 4
  %132 = load i32* %15, align 4
  %temp.vect163 = insertelement <4 x i32> undef, i32 %129, i32 0
  %temp.vect164 = insertelement <4 x i32> %temp.vect163, i32 %130, i32 1
  %temp.vect165 = insertelement <4 x i32> %temp.vect164, i32 %131, i32 2
  %temp.vect166 = insertelement <4 x i32> %temp.vect165, i32 %132, i32 3
  %shl73167 = shl <4 x i32> %temp.vect166, <i32 16, i32 16, i32 16, i32 16>
  %and74168 = and <4 x i32> %shl73167, <i32 16711680, i32 16711680, i32 16711680, i32 16711680>
  %or169 = or <4 x i32> %and71162, %and156
  %or75170 = or <4 x i32> %or169, %and74168
  %extract176 = extractelement <4 x i32> %or75170, i32 0
  %extract177 = extractelement <4 x i32> %or75170, i32 1
  %extract178 = extractelement <4 x i32> %or75170, i32 2
  %extract179 = extractelement <4 x i32> %or75170, i32 3
  %add78171 = add nsw <4 x i32> %vector30, %mul7726
  %extract172 = extractelement <4 x i32> %add78171, i32 0
  %extract173 = extractelement <4 x i32> %add78171, i32 1
  %extract174 = extractelement <4 x i32> %add78171, i32 2
  %extract175 = extractelement <4 x i32> %add78171, i32 3
  %133 = getelementptr inbounds i32 addrspace(1)* %pDst, i32 %extract172
  %134 = getelementptr inbounds i32 addrspace(1)* %pDst, i32 %extract173
  %135 = getelementptr inbounds i32 addrspace(1)* %pDst, i32 %extract174
  %136 = getelementptr inbounds i32 addrspace(1)* %pDst, i32 %extract175
  store i32 %extract176, i32 addrspace(1)* %133, align 4
  store i32 %extract177, i32 addrspace(1)* %134, align 4
  store i32 %extract178, i32 addrspace(1)* %135, align 4
  store i32 %extract179, i32 addrspace(1)* %136, align 4
  %inc81 = add nsw i32 %x.012, 1
  %exitcond14 = icmp eq i32 %inc81, %iImageWidth
  br i1 %exitcond14, label %for.end82, label %for.cond1.preheader

for.end82:                                        ; preds = %for.end68, %entry
  ret void
}

define <8 x i32> @local.avx256.pcmpeq.d(<8 x i32>, <8 x i32>) {
entry:
  %ALow = shufflevector <8 x i32> %0, <8 x i32> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %BLow = shufflevector <8 x i32> %1, <8 x i32> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %AHigh = shufflevector <8 x i32> %0, <8 x i32> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  %BHigh = shufflevector <8 x i32> %1, <8 x i32> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  %callLow = call <4 x i32> @llvm.x86.sse2.pcmpeq.d(<4 x i32> %ALow, <4 x i32> %BLow)
  %callLow1 = call <4 x i32> @llvm.x86.sse2.pcmpeq.d(<4 x i32> %AHigh, <4 x i32> %BHigh)
  %join = shufflevector <4 x i32> %callLow, <4 x i32> %callLow1, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  ret <8 x i32> %join
}

declare <4 x i32> @llvm.x86.sse2.pcmpeq.d(<4 x i32>, <4 x i32>) nounwind readnone

define <8 x i32> @local.avx256.pcmpgt.d(<8 x i32>, <8 x i32>) {
entry:
  %ALow = shufflevector <8 x i32> %0, <8 x i32> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %BLow = shufflevector <8 x i32> %1, <8 x i32> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  %AHigh = shufflevector <8 x i32> %0, <8 x i32> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  %BHigh = shufflevector <8 x i32> %1, <8 x i32> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  %callLow = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %ALow, <4 x i32> %BLow)
  %callLow1 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %AHigh, <4 x i32> %BHigh)
  %join = shufflevector <4 x i32> %callLow, <4 x i32> %callLow1, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  ret <8 x i32> %join
}

declare <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32>, <4 x i32>) nounwind readnone

declare i1 @allOne(i1)

declare i1 @allZero(i1)

declare <4 x float> @llvm.x86.sse41.blendvps(<4 x float>, <4 x float>, <4 x float>) nounwind readnone

!opencl.kernels = !{!0, !1}
!opencl.cl_kernel_arg_info = !{!2, !8}
!opencl.build.options = !{!10}

!0 = metadata !{void (<4 x i8> addrspace(1)*, i32 addrspace(1)*, i32, i32)* @intel_median}
!1 = metadata !{void (i8 addrspace(1)*, i32 addrspace(1)*, i32, i32)* @intel_median_scalar}
!2 = metadata !{metadata !"cl_kernel_arg_info", void (<4 x i8> addrspace(1)*, i32 addrspace(1)*, i32, i32)* @intel_median, metadata !3, metadata !4, metadata !5, metadata !6, metadata !7}
!3 = metadata !{i32 1, i32 1, i32 0, i32 0}
!4 = metadata !{i32 3, i32 3, i32 3, i32 3}
!5 = metadata !{metadata !"uchar4*", metadata !"unsigned int*", metadata !"int", metadata !"int"}
!6 = metadata !{i32 0, i32 0, i32 0, i32 0}
!7 = metadata !{metadata !"pSrc", metadata !"pDst", metadata !"iImageWidth", metadata !"iImageHeight"}
!8 = metadata !{metadata !"cl_kernel_arg_info", void (i8 addrspace(1)*, i32 addrspace(1)*, i32, i32)* @intel_median_scalar, metadata !3, metadata !4, metadata !9, metadata !6, metadata !7}
!9 = metadata !{metadata !"uchar*", metadata !"unsigned int*", metadata !"int", metadata !"int"}
!10 = metadata !{metadata !"-cl-kernel-arg-info"}
