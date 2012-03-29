; ModuleID = 'Program'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"
target triple = "i686-pc-win32"

define void @intel_median(<4 x i8> addrspace(1)* nocapture %pSrc, i32 addrspace(1)* nocapture %pDst, i32 %iImageWidth, i32 %iImageHeight) nounwind {
entry:
  %iPixels = alloca [9 x <4 x i32>], align 16
  %call = call i32 @get_global_id(i32 0) nounwind readnone
  %add = add i32 %call, 2
  %mul74 = mul nsw i32 %add, %iImageWidth
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.end66, %entry
  %0 = phi <4 x i32> [ undef, %entry ], [ %.pre, %for.end66 ]
  %x.011 = phi i32 [ 0, %entry ], [ %inc78, %for.end66 ]
  br label %for.body3

for.body3:                                        ; preds = %for.body3.for.body3_crit_edge, %for.cond1.preheader
  %1 = phi <4 x i32> [ %0, %for.cond1.preheader ], [ %.pre23, %for.body3.for.body3_crit_edge ]
  %iRow.02 = phi i32 [ -1, %for.cond1.preheader ], [ %inc28, %for.body3.for.body3_crit_edge ]
  %iPixelCount.01 = phi i32 [ 0, %for.cond1.preheader ], [ %inc27, %for.body3.for.body3_crit_edge ]
  %add4 = add i32 %add, %iRow.02
  %mul = mul nsw i32 %add4, %iImageWidth
  %add5 = add nsw i32 %mul, %x.011
  %sub = add nsw i32 %add5, -1
  %arrayidx = getelementptr inbounds <4 x i8> addrspace(1)* %pSrc, i32 %sub
  %2 = load <4 x i8> addrspace(1)* %arrayidx, align 4
  %3 = extractelement <4 x i8> %2, i32 0
  %conv = zext i8 %3 to i32
  %arrayidx6 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 %iPixelCount.01
  %4 = insertelement <4 x i32> %1, i32 %conv, i32 0
  %5 = extractelement <4 x i8> %2, i32 1
  %conv7 = zext i8 %5 to i32
  %6 = insertelement <4 x i32> %4, i32 %conv7, i32 1
  %7 = extractelement <4 x i8> %2, i32 2
  %conv9 = zext i8 %7 to i32
  %8 = insertelement <4 x i32> %6, i32 %conv9, i32 2
  store <4 x i32> %8, <4 x i32>* %arrayidx6, align 16
  %inc = add nsw i32 %iPixelCount.01, 1
  %arrayidx11 = getelementptr inbounds <4 x i8> addrspace(1)* %pSrc, i32 %add5
  %9 = load <4 x i8> addrspace(1)* %arrayidx11, align 4
  %10 = extractelement <4 x i8> %9, i32 0
  %conv12 = zext i8 %10 to i32
  %arrayidx13 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 %inc
  %11 = load <4 x i32>* %arrayidx13, align 16
  %12 = insertelement <4 x i32> %11, i32 %conv12, i32 0
  %13 = extractelement <4 x i8> %9, i32 1
  %conv14 = zext i8 %13 to i32
  %14 = insertelement <4 x i32> %12, i32 %conv14, i32 1
  %15 = extractelement <4 x i8> %9, i32 2
  %conv16 = zext i8 %15 to i32
  %16 = insertelement <4 x i32> %14, i32 %conv16, i32 2
  store <4 x i32> %16, <4 x i32>* %arrayidx13, align 16
  %inc18 = add nsw i32 %iPixelCount.01, 2
  %add19 = add nsw i32 %add5, 1
  %arrayidx20 = getelementptr inbounds <4 x i8> addrspace(1)* %pSrc, i32 %add19
  %17 = load <4 x i8> addrspace(1)* %arrayidx20, align 4
  %18 = extractelement <4 x i8> %17, i32 0
  %conv21 = zext i8 %18 to i32
  %arrayidx22 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 %inc18
  %19 = load <4 x i32>* %arrayidx22, align 16
  %20 = insertelement <4 x i32> %19, i32 %conv21, i32 0
  %21 = extractelement <4 x i8> %17, i32 1
  %conv23 = zext i8 %21 to i32
  %22 = insertelement <4 x i32> %20, i32 %conv23, i32 1
  %23 = extractelement <4 x i8> %17, i32 2
  %conv25 = zext i8 %23 to i32
  %24 = insertelement <4 x i32> %22, i32 %conv25, i32 2
  store <4 x i32> %24, <4 x i32>* %arrayidx22, align 16
  %inc27 = add nsw i32 %iPixelCount.01, 3
  %inc28 = add nsw i32 %iRow.02, 1
  %exitcond = icmp eq i32 %inc28, 2
  br i1 %exitcond, label %for.cond29.loopexit, label %for.body3.for.body3_crit_edge

for.body3.for.body3_crit_edge:                    ; preds = %for.body3
  %arrayidx6.phi.trans.insert = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 %inc27
  %.pre23 = load <4 x i32>* %arrayidx6.phi.trans.insert, align 16
  br label %for.body3

for.cond29.loopexit:                              ; preds = %for.body3
  %arrayidx38 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 0
  %.pre = load <4 x i32>* %arrayidx38, align 16
  %arrayidx42.phi.trans.insert = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 1
  %.pre15 = load <4 x i32>* %arrayidx42.phi.trans.insert, align 16
  %arrayidx47.phi.trans.insert = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 2
  %.pre16 = load <4 x i32>* %arrayidx47.phi.trans.insert, align 16
  %arrayidx38.1.phi.trans.insert = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 3
  %.pre17 = load <4 x i32>* %arrayidx38.1.phi.trans.insert, align 16
  %arrayidx42.1.phi.trans.insert = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 4
  %.pre18 = load <4 x i32>* %arrayidx42.1.phi.trans.insert, align 16
  %arrayidx47.1.phi.trans.insert = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 5
  %.pre19 = load <4 x i32>* %arrayidx47.1.phi.trans.insert, align 16
  %arrayidx38.2.phi.trans.insert = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 6
  %.pre20 = load <4 x i32>* %arrayidx38.2.phi.trans.insert, align 16
  %arrayidx42.2.phi.trans.insert = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 7
  %.pre21 = load <4 x i32>* %arrayidx42.2.phi.trans.insert, align 16
  %arrayidx47.2.phi.trans.insert = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 8
  %.pre22 = load <4 x i32>* %arrayidx47.2.phi.trans.insert, align 16
  br label %for.cond34.preheader

for.cond34.preheader:                             ; preds = %for.cond34.preheader, %for.cond29.loopexit
  %iSearch.09 = phi i32 [ 0, %for.cond29.loopexit ], [ %inc65, %for.cond34.preheader ]
  %iYes.08 = phi <4 x i32> [ <i32 128, i32 128, i32 128, i32 128>, %for.cond29.loopexit ], [ %shr, %for.cond34.preheader ]
  %iMin.07 = phi <4 x i32> [ zeroinitializer, %for.cond29.loopexit ], [ %or, %for.cond34.preheader ]
  %iMax.06 = phi <4 x i32> [ <i32 255, i32 255, i32 255, i32 255>, %for.cond29.loopexit ], [ %or62, %for.cond34.preheader ]
  %cmp39 = icmp slt <4 x i32> %iYes.08, %.pre
  %sext = sext <4 x i1> %cmp39 to <4 x i32>
  %cmp43 = icmp slt <4 x i32> %iYes.08, %.pre15
  %sext44 = sext <4 x i1> %cmp43 to <4 x i32>
  %add45 = add <4 x i32> %sext, %sext44
  %cmp48 = icmp slt <4 x i32> %iYes.08, %.pre16
  %sext49 = sext <4 x i1> %cmp48 to <4 x i32>
  %add50 = add <4 x i32> %add45, %sext49
  %cmp39.1 = icmp slt <4 x i32> %iYes.08, %.pre17
  %sext.1 = sext <4 x i1> %cmp39.1 to <4 x i32>
  %add40.1 = add <4 x i32> %add50, %sext.1
  %cmp43.1 = icmp slt <4 x i32> %iYes.08, %.pre18
  %sext44.1 = sext <4 x i1> %cmp43.1 to <4 x i32>
  %add45.1 = add <4 x i32> %add40.1, %sext44.1
  %cmp48.1 = icmp slt <4 x i32> %iYes.08, %.pre19
  %sext49.1 = sext <4 x i1> %cmp48.1 to <4 x i32>
  %add50.1 = add <4 x i32> %add45.1, %sext49.1
  %cmp39.2 = icmp slt <4 x i32> %iYes.08, %.pre20
  %sext.2 = sext <4 x i1> %cmp39.2 to <4 x i32>
  %add40.2 = add <4 x i32> %add50.1, %sext.2
  %cmp43.2 = icmp slt <4 x i32> %iYes.08, %.pre21
  %sext44.2 = sext <4 x i1> %cmp43.2 to <4 x i32>
  %add45.2 = add <4 x i32> %add40.2, %sext44.2
  %cmp48.2 = icmp slt <4 x i32> %iYes.08, %.pre22
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
  %25 = extractelement <4 x i32> %shr, i32 0
  %and67 = and i32 %25, 255
  %26 = extractelement <4 x i32> %shr, i32 1
  %shl = shl i32 %26, 8
  %and68 = and i32 %shl, 65280
  %27 = extractelement <4 x i32> %shr, i32 2
  %shl70 = shl i32 %27, 16
  %and71 = and i32 %shl70, 16711680
  %or69 = or i32 %and71, %and67
  %or72 = or i32 %or69, %and68
  %add75 = add nsw i32 %x.011, %mul74
  %arrayidx76 = getelementptr inbounds i32 addrspace(1)* %pDst, i32 %add75
  store i32 %or72, i32 addrspace(1)* %arrayidx76, align 4
  %inc78 = add nsw i32 %x.011, 1
  %exitcond14 = icmp eq i32 %inc78, %iImageWidth
  br i1 %exitcond14, label %for.end79, label %for.cond1.preheader

for.end79:                                        ; preds = %for.end66
  ret void
}

declare i32 @get_global_id(i32) nounwind readnone

define void @intel_median_scalar(i8 addrspace(1)* nocapture %pSrc, i32 addrspace(1)* nocapture %pDst, i32 %iImageWidth, i32 %iImageHeight) nounwind {
entry:
  %iResult = alloca [4 x i32], align 4
  %iPixels = alloca [9 x i32], align 4
  %call = call i32 @get_global_id(i32 0) nounwind readnone
  %add = add i32 %call, 2
  %arrayidx69 = getelementptr inbounds [4 x i32]* %iResult, i32 0, i32 0
  %arrayidx70 = getelementptr inbounds [4 x i32]* %iResult, i32 0, i32 1
  %arrayidx72 = getelementptr inbounds [4 x i32]* %iResult, i32 0, i32 2
  %mul77 = mul nsw i32 %add, %iImageWidth
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.end68, %entry
  %x.012 = phi i32 [ 0, %entry ], [ %inc81, %for.end68 ]
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
  %cmp36 = icmp slt i32 %iYes.06, %conv
  %conv37 = zext i1 %cmp36 to i32
  %cmp41 = icmp slt i32 %iYes.06, %conv15
  %conv42 = zext i1 %cmp41 to i32
  %cmp46 = icmp slt i32 %iYes.06, %conv22
  %conv47 = zext i1 %cmp46 to i32
  %add43 = add i32 %conv37, %conv42
  %add48 = add i32 %add43, %conv47
  %cmp36.1 = icmp slt i32 %iYes.06, %conv.1
  %conv37.1 = zext i1 %cmp36.1 to i32
  %cmp41.1 = icmp slt i32 %iYes.06, %conv15.1
  %conv42.1 = zext i1 %cmp41.1 to i32
  %cmp46.1 = icmp slt i32 %iYes.06, %conv22.1
  %conv47.1 = zext i1 %cmp46.1 to i32
  %add38.1 = add i32 %conv37.1, %add48
  %add43.1 = add i32 %add38.1, %conv42.1
  %add48.1 = add i32 %add43.1, %conv47.1
  %cmp36.2 = icmp slt i32 %iYes.06, %conv.2
  %conv37.2 = zext i1 %cmp36.2 to i32
  %cmp41.2 = icmp slt i32 %iYes.06, %conv15.2
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
  %9 = load i32* %arrayidx69, align 4
  %and = and i32 %9, 255
  %10 = load i32* %arrayidx70, align 4
  %shl = shl i32 %10, 8
  %and71 = and i32 %shl, 65280
  %11 = load i32* %arrayidx72, align 4
  %shl73 = shl i32 %11, 16
  %and74 = and i32 %shl73, 16711680
  %or = or i32 %and71, %and
  %or75 = or i32 %or, %and74
  %add78 = add nsw i32 %x.012, %mul77
  %arrayidx79 = getelementptr inbounds i32 addrspace(1)* %pDst, i32 %add78
  store i32 %or75, i32 addrspace(1)* %arrayidx79, align 4
  %inc81 = add nsw i32 %x.012, 1
  %exitcond14 = icmp eq i32 %inc81, %iImageWidth
  br i1 %exitcond14, label %for.end82, label %for.cond1.preheader

for.end82:                                        ; preds = %for.end68
  ret void
}

define [7 x i32] @WG.boundaries.intel_median(<4 x i8> addrspace(1)*, i32 addrspace(1)*, i32, i32) {
entry:
  %4 = call i32 @get_local_size(i32 0)
  %5 = call i32 @get_base_global_id.(i32 0)
  %6 = call i32 @get_local_size(i32 1)
  %7 = call i32 @get_base_global_id.(i32 1)
  %8 = call i32 @get_local_size(i32 2)
  %9 = call i32 @get_base_global_id.(i32 2)
  %10 = icmp sgt i32 %2, 0
  %11 = and i1 true, %10
  %zext_cast = zext i1 %11 to i32
  %12 = insertvalue [7 x i32] undef, i32 %4, 2
  %13 = insertvalue [7 x i32] %12, i32 %5, 1
  %14 = insertvalue [7 x i32] %13, i32 %6, 4
  %15 = insertvalue [7 x i32] %14, i32 %7, 3
  %16 = insertvalue [7 x i32] %15, i32 %8, 6
  %17 = insertvalue [7 x i32] %16, i32 %9, 5
  %18 = insertvalue [7 x i32] %17, i32 %zext_cast, 0
  ret [7 x i32] %18
}

declare i32 @get_local_size(i32)

declare i32 @get_base_global_id.(i32)

define [7 x i32] @WG.boundaries.intel_median_scalar(i8 addrspace(1)*, i32 addrspace(1)*, i32, i32) {
entry:
  %4 = call i32 @get_local_size(i32 0)
  %5 = call i32 @get_base_global_id.(i32 0)
  %6 = call i32 @get_local_size(i32 1)
  %7 = call i32 @get_base_global_id.(i32 1)
  %8 = call i32 @get_local_size(i32 2)
  %9 = call i32 @get_base_global_id.(i32 2)
  %10 = icmp sgt i32 %2, 0
  %11 = and i1 true, %10
  %zext_cast = zext i1 %11 to i32
  %12 = insertvalue [7 x i32] undef, i32 %4, 2
  %13 = insertvalue [7 x i32] %12, i32 %5, 1
  %14 = insertvalue [7 x i32] %13, i32 %6, 4
  %15 = insertvalue [7 x i32] %14, i32 %7, 3
  %16 = insertvalue [7 x i32] %15, i32 %8, 6
  %17 = insertvalue [7 x i32] %16, i32 %9, 5
  %18 = insertvalue [7 x i32] %17, i32 %zext_cast, 0
  ret [7 x i32] %18
}

define void @__Vectorized_.intel_median(<4 x i8> addrspace(1)* nocapture %pSrc, i32 addrspace(1)* nocapture %pDst, i32 %iImageWidth, i32 %iImageHeight) nounwind {
entry:
  %temp = insertelement <4 x i32> undef, i32 %iImageWidth, i32 0
  %vector = shufflevector <4 x i32> %temp, <4 x i32> undef, <4 x i32> zeroinitializer
  %0 = alloca [9 x <4 x i32>], align 16
  %1 = alloca [9 x <4 x i32>], align 16
  %2 = alloca [9 x <4 x i32>], align 16
  %3 = alloca [9 x <4 x i32>], align 16
  %call = call i32 @get_global_id(i32 0) nounwind readnone
  %broadcast1 = insertelement <4 x i32> undef, i32 %call, i32 0
  %broadcast2 = shufflevector <4 x i32> %broadcast1, <4 x i32> undef, <4 x i32> zeroinitializer
  %add235 = add <4 x i32> %broadcast2, <i32 2, i32 3, i32 4, i32 5>
  %mul74236 = mul nsw <4 x i32> %add235, %vector
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.end66, %entry
  %vectorPHI = phi <4 x i32> [ undef, %entry ], [ %shuffleMerge333, %for.end66 ]
  %x.011 = phi i32 [ 0, %entry ], [ %inc78, %for.end66 ]
  %temp242 = insertelement <4 x i32> undef, i32 %x.011, i32 0
  %vector243 = shufflevector <4 x i32> %temp242, <4 x i32> undef, <4 x i32> zeroinitializer
  br label %for.body3

for.body3:                                        ; preds = %for.body3.for.body3_crit_edge, %for.cond1.preheader
  %vectorPHI237 = phi <4 x i32> [ %vectorPHI, %for.cond1.preheader ], [ %shuffleMerge321, %for.body3.for.body3_crit_edge ]
  %iRow.02 = phi i32 [ -1, %for.cond1.preheader ], [ %inc28, %for.body3.for.body3_crit_edge ]
  %iPixelCount.01 = phi i32 [ 0, %for.cond1.preheader ], [ %inc27, %for.body3.for.body3_crit_edge ]
  %temp238 = insertelement <4 x i32> undef, i32 %iRow.02, i32 0
  %vector239 = shufflevector <4 x i32> %temp238, <4 x i32> undef, <4 x i32> zeroinitializer
  %add4240 = add <4 x i32> %add235, %vector239
  %mul241 = mul nsw <4 x i32> %add4240, %vector
  %add5244 = add nsw <4 x i32> %mul241, %vector243
  %extract264 = extractelement <4 x i32> %add5244, i32 0
  %extract265 = extractelement <4 x i32> %add5244, i32 1
  %extract266 = extractelement <4 x i32> %add5244, i32 2
  %extract267 = extractelement <4 x i32> %add5244, i32 3
  %sub245 = add nsw <4 x i32> %add5244, <i32 -1, i32 -1, i32 -1, i32 -1>
  %extract = extractelement <4 x i32> %sub245, i32 0
  %extract246 = extractelement <4 x i32> %sub245, i32 1
  %extract247 = extractelement <4 x i32> %sub245, i32 2
  %extract248 = extractelement <4 x i32> %sub245, i32 3
  %4 = getelementptr inbounds <4 x i8> addrspace(1)* %pSrc, i32 %extract
  %5 = getelementptr inbounds <4 x i8> addrspace(1)* %pSrc, i32 %extract246
  %6 = getelementptr inbounds <4 x i8> addrspace(1)* %pSrc, i32 %extract247
  %7 = getelementptr inbounds <4 x i8> addrspace(1)* %pSrc, i32 %extract248
  %8 = load <4 x i8> addrspace(1)* %4, align 4
  %9 = load <4 x i8> addrspace(1)* %5, align 4
  %10 = load <4 x i8> addrspace(1)* %6, align 4
  %11 = load <4 x i8> addrspace(1)* %7, align 4
  %shuffle0 = shufflevector <4 x i8> %8, <4 x i8> %9, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffle1 = shufflevector <4 x i8> %10, <4 x i8> %11, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffleMerge = shufflevector <4 x i8> %shuffle0, <4 x i8> %shuffle1, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffle0249 = shufflevector <4 x i8> %8, <4 x i8> %9, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffle1250 = shufflevector <4 x i8> %10, <4 x i8> %11, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffleMerge251 = shufflevector <4 x i8> %shuffle0249, <4 x i8> %shuffle1250, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle0252 = shufflevector <4 x i8> %8, <4 x i8> %9, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffle1253 = shufflevector <4 x i8> %10, <4 x i8> %11, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffleMerge254 = shufflevector <4 x i8> %shuffle0252, <4 x i8> %shuffle1253, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %conv255 = zext <4 x i8> %shuffleMerge to <4 x i32>
  %12 = getelementptr inbounds [9 x <4 x i32>]* %0, i32 0, i32 %iPixelCount.01
  %13 = getelementptr inbounds [9 x <4 x i32>]* %1, i32 0, i32 %iPixelCount.01
  %14 = getelementptr inbounds [9 x <4 x i32>]* %2, i32 0, i32 %iPixelCount.01
  %15 = getelementptr inbounds [9 x <4 x i32>]* %3, i32 0, i32 %iPixelCount.01
  %conv7256 = zext <4 x i8> %shuffleMerge251 to <4 x i32>
  %conv9257 = zext <4 x i8> %shuffleMerge254 to <4 x i32>
  %shuf_transpL = shufflevector <4 x i32> %conv255, <4 x i32> %conv9257, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuf_transpL258 = shufflevector <4 x i32> %conv7256, <4 x i32> %vectorPHI237, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuf_transpH = shufflevector <4 x i32> %conv255, <4 x i32> %conv9257, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuf_transpH259 = shufflevector <4 x i32> %conv7256, <4 x i32> %vectorPHI237, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuf_transpL260 = shufflevector <4 x i32> %shuf_transpL, <4 x i32> %shuf_transpL258, <4 x i32> <i32 0, i32 4, i32 2, i32 6>
  %shuf_transpH261 = shufflevector <4 x i32> %shuf_transpL, <4 x i32> %shuf_transpL258, <4 x i32> <i32 1, i32 5, i32 3, i32 7>
  %shuf_transpL262 = shufflevector <4 x i32> %shuf_transpH, <4 x i32> %shuf_transpH259, <4 x i32> <i32 0, i32 4, i32 2, i32 6>
  %shuf_transpH263 = shufflevector <4 x i32> %shuf_transpH, <4 x i32> %shuf_transpH259, <4 x i32> <i32 1, i32 5, i32 3, i32 7>
  store <4 x i32> %shuf_transpL260, <4 x i32>* %12, align 16
  store <4 x i32> %shuf_transpH261, <4 x i32>* %13, align 16
  store <4 x i32> %shuf_transpL262, <4 x i32>* %14, align 16
  store <4 x i32> %shuf_transpH263, <4 x i32>* %15, align 16
  %inc = add nsw i32 %iPixelCount.01, 1
  %16 = getelementptr inbounds <4 x i8> addrspace(1)* %pSrc, i32 %extract264
  %17 = getelementptr inbounds <4 x i8> addrspace(1)* %pSrc, i32 %extract265
  %18 = getelementptr inbounds <4 x i8> addrspace(1)* %pSrc, i32 %extract266
  %19 = getelementptr inbounds <4 x i8> addrspace(1)* %pSrc, i32 %extract267
  %20 = load <4 x i8> addrspace(1)* %16, align 4
  %21 = load <4 x i8> addrspace(1)* %17, align 4
  %22 = load <4 x i8> addrspace(1)* %18, align 4
  %23 = load <4 x i8> addrspace(1)* %19, align 4
  %shuffle0268 = shufflevector <4 x i8> %20, <4 x i8> %21, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffle1269 = shufflevector <4 x i8> %22, <4 x i8> %23, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffleMerge270 = shufflevector <4 x i8> %shuffle0268, <4 x i8> %shuffle1269, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffle0271 = shufflevector <4 x i8> %20, <4 x i8> %21, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffle1272 = shufflevector <4 x i8> %22, <4 x i8> %23, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffleMerge273 = shufflevector <4 x i8> %shuffle0271, <4 x i8> %shuffle1272, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle0274 = shufflevector <4 x i8> %20, <4 x i8> %21, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffle1275 = shufflevector <4 x i8> %22, <4 x i8> %23, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffleMerge276 = shufflevector <4 x i8> %shuffle0274, <4 x i8> %shuffle1275, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %conv12277 = zext <4 x i8> %shuffleMerge270 to <4 x i32>
  %24 = getelementptr inbounds [9 x <4 x i32>]* %0, i32 0, i32 %inc
  %25 = getelementptr inbounds [9 x <4 x i32>]* %1, i32 0, i32 %inc
  %26 = getelementptr inbounds [9 x <4 x i32>]* %2, i32 0, i32 %inc
  %27 = getelementptr inbounds [9 x <4 x i32>]* %3, i32 0, i32 %inc
  %28 = load <4 x i32>* %24, align 16
  %29 = load <4 x i32>* %25, align 16
  %30 = load <4 x i32>* %26, align 16
  %31 = load <4 x i32>* %27, align 16
  %shuffle0278 = shufflevector <4 x i32> %28, <4 x i32> %29, <4 x i32> <i32 undef, i32 3, i32 undef, i32 7>
  %shuffle1279 = shufflevector <4 x i32> %30, <4 x i32> %31, <4 x i32> <i32 undef, i32 3, i32 undef, i32 7>
  %shuffleMerge280 = shufflevector <4 x i32> %shuffle0278, <4 x i32> %shuffle1279, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %conv14281 = zext <4 x i8> %shuffleMerge273 to <4 x i32>
  %conv16282 = zext <4 x i8> %shuffleMerge276 to <4 x i32>
  %shuf_transpL283 = shufflevector <4 x i32> %conv12277, <4 x i32> %conv16282, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuf_transpL284 = shufflevector <4 x i32> %conv14281, <4 x i32> %shuffleMerge280, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuf_transpH285 = shufflevector <4 x i32> %conv12277, <4 x i32> %conv16282, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuf_transpH286 = shufflevector <4 x i32> %conv14281, <4 x i32> %shuffleMerge280, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuf_transpL287 = shufflevector <4 x i32> %shuf_transpL283, <4 x i32> %shuf_transpL284, <4 x i32> <i32 0, i32 4, i32 2, i32 6>
  %shuf_transpH288 = shufflevector <4 x i32> %shuf_transpL283, <4 x i32> %shuf_transpL284, <4 x i32> <i32 1, i32 5, i32 3, i32 7>
  %shuf_transpL289 = shufflevector <4 x i32> %shuf_transpH285, <4 x i32> %shuf_transpH286, <4 x i32> <i32 0, i32 4, i32 2, i32 6>
  %shuf_transpH290 = shufflevector <4 x i32> %shuf_transpH285, <4 x i32> %shuf_transpH286, <4 x i32> <i32 1, i32 5, i32 3, i32 7>
  store <4 x i32> %shuf_transpL287, <4 x i32>* %24, align 16
  store <4 x i32> %shuf_transpH288, <4 x i32>* %25, align 16
  store <4 x i32> %shuf_transpL289, <4 x i32>* %26, align 16
  store <4 x i32> %shuf_transpH290, <4 x i32>* %27, align 16
  %inc18 = add nsw i32 %iPixelCount.01, 2
  %add19291 = add nsw <4 x i32> %add5244, <i32 1, i32 1, i32 1, i32 1>
  %extract292 = extractelement <4 x i32> %add19291, i32 0
  %extract293 = extractelement <4 x i32> %add19291, i32 1
  %extract294 = extractelement <4 x i32> %add19291, i32 2
  %extract295 = extractelement <4 x i32> %add19291, i32 3
  %32 = getelementptr inbounds <4 x i8> addrspace(1)* %pSrc, i32 %extract292
  %33 = getelementptr inbounds <4 x i8> addrspace(1)* %pSrc, i32 %extract293
  %34 = getelementptr inbounds <4 x i8> addrspace(1)* %pSrc, i32 %extract294
  %35 = getelementptr inbounds <4 x i8> addrspace(1)* %pSrc, i32 %extract295
  %36 = load <4 x i8> addrspace(1)* %32, align 4
  %37 = load <4 x i8> addrspace(1)* %33, align 4
  %38 = load <4 x i8> addrspace(1)* %34, align 4
  %39 = load <4 x i8> addrspace(1)* %35, align 4
  %shuffle0296 = shufflevector <4 x i8> %36, <4 x i8> %37, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffle1297 = shufflevector <4 x i8> %38, <4 x i8> %39, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffleMerge298 = shufflevector <4 x i8> %shuffle0296, <4 x i8> %shuffle1297, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffle0299 = shufflevector <4 x i8> %36, <4 x i8> %37, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffle1300 = shufflevector <4 x i8> %38, <4 x i8> %39, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffleMerge301 = shufflevector <4 x i8> %shuffle0299, <4 x i8> %shuffle1300, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle0302 = shufflevector <4 x i8> %36, <4 x i8> %37, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffle1303 = shufflevector <4 x i8> %38, <4 x i8> %39, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffleMerge304 = shufflevector <4 x i8> %shuffle0302, <4 x i8> %shuffle1303, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %conv21305 = zext <4 x i8> %shuffleMerge298 to <4 x i32>
  %40 = getelementptr inbounds [9 x <4 x i32>]* %0, i32 0, i32 %inc18
  %41 = getelementptr inbounds [9 x <4 x i32>]* %1, i32 0, i32 %inc18
  %42 = getelementptr inbounds [9 x <4 x i32>]* %2, i32 0, i32 %inc18
  %43 = getelementptr inbounds [9 x <4 x i32>]* %3, i32 0, i32 %inc18
  %44 = load <4 x i32>* %40, align 16
  %45 = load <4 x i32>* %41, align 16
  %46 = load <4 x i32>* %42, align 16
  %47 = load <4 x i32>* %43, align 16
  %shuffle0306 = shufflevector <4 x i32> %44, <4 x i32> %45, <4 x i32> <i32 undef, i32 3, i32 undef, i32 7>
  %shuffle1307 = shufflevector <4 x i32> %46, <4 x i32> %47, <4 x i32> <i32 undef, i32 3, i32 undef, i32 7>
  %shuffleMerge308 = shufflevector <4 x i32> %shuffle0306, <4 x i32> %shuffle1307, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %conv23309 = zext <4 x i8> %shuffleMerge301 to <4 x i32>
  %conv25310 = zext <4 x i8> %shuffleMerge304 to <4 x i32>
  %shuf_transpL311 = shufflevector <4 x i32> %conv21305, <4 x i32> %conv25310, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuf_transpL312 = shufflevector <4 x i32> %conv23309, <4 x i32> %shuffleMerge308, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuf_transpH313 = shufflevector <4 x i32> %conv21305, <4 x i32> %conv25310, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuf_transpH314 = shufflevector <4 x i32> %conv23309, <4 x i32> %shuffleMerge308, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuf_transpL315 = shufflevector <4 x i32> %shuf_transpL311, <4 x i32> %shuf_transpL312, <4 x i32> <i32 0, i32 4, i32 2, i32 6>
  %shuf_transpH316 = shufflevector <4 x i32> %shuf_transpL311, <4 x i32> %shuf_transpL312, <4 x i32> <i32 1, i32 5, i32 3, i32 7>
  %shuf_transpL317 = shufflevector <4 x i32> %shuf_transpH313, <4 x i32> %shuf_transpH314, <4 x i32> <i32 0, i32 4, i32 2, i32 6>
  %shuf_transpH318 = shufflevector <4 x i32> %shuf_transpH313, <4 x i32> %shuf_transpH314, <4 x i32> <i32 1, i32 5, i32 3, i32 7>
  store <4 x i32> %shuf_transpL315, <4 x i32>* %40, align 16
  store <4 x i32> %shuf_transpH316, <4 x i32>* %41, align 16
  store <4 x i32> %shuf_transpL317, <4 x i32>* %42, align 16
  store <4 x i32> %shuf_transpH318, <4 x i32>* %43, align 16
  %inc27 = add nsw i32 %iPixelCount.01, 3
  %inc28 = add nsw i32 %iRow.02, 1
  %exitcond = icmp eq i32 %inc28, 2
  br i1 %exitcond, label %for.cond29.loopexit, label %for.body3.for.body3_crit_edge

for.body3.for.body3_crit_edge:                    ; preds = %for.body3
  %48 = getelementptr inbounds [9 x <4 x i32>]* %0, i32 0, i32 %inc27
  %49 = getelementptr inbounds [9 x <4 x i32>]* %1, i32 0, i32 %inc27
  %50 = getelementptr inbounds [9 x <4 x i32>]* %2, i32 0, i32 %inc27
  %51 = getelementptr inbounds [9 x <4 x i32>]* %3, i32 0, i32 %inc27
  %52 = load <4 x i32>* %48, align 16
  %53 = load <4 x i32>* %49, align 16
  %54 = load <4 x i32>* %50, align 16
  %55 = load <4 x i32>* %51, align 16
  %shuffle0319 = shufflevector <4 x i32> %52, <4 x i32> %53, <4 x i32> <i32 undef, i32 3, i32 undef, i32 7>
  %shuffle1320 = shufflevector <4 x i32> %54, <4 x i32> %55, <4 x i32> <i32 undef, i32 3, i32 undef, i32 7>
  %shuffleMerge321 = shufflevector <4 x i32> %shuffle0319, <4 x i32> %shuffle1320, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  br label %for.body3

for.cond29.loopexit:                              ; preds = %for.body3
  %56 = getelementptr inbounds [9 x <4 x i32>]* %0, i32 0, i32 0
  %57 = getelementptr inbounds [9 x <4 x i32>]* %1, i32 0, i32 0
  %58 = getelementptr inbounds [9 x <4 x i32>]* %2, i32 0, i32 0
  %59 = getelementptr inbounds [9 x <4 x i32>]* %3, i32 0, i32 0
  %60 = load <4 x i32>* %56, align 16
  %61 = load <4 x i32>* %57, align 16
  %62 = load <4 x i32>* %58, align 16
  %63 = load <4 x i32>* %59, align 16
  %shuffle0322 = shufflevector <4 x i32> %60, <4 x i32> %61, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffle1323 = shufflevector <4 x i32> %62, <4 x i32> %63, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffleMerge324 = shufflevector <4 x i32> %shuffle0322, <4 x i32> %shuffle1323, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffle0325 = shufflevector <4 x i32> %60, <4 x i32> %61, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffle1326 = shufflevector <4 x i32> %62, <4 x i32> %63, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffleMerge327 = shufflevector <4 x i32> %shuffle0325, <4 x i32> %shuffle1326, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle0328 = shufflevector <4 x i32> %60, <4 x i32> %61, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffle1329 = shufflevector <4 x i32> %62, <4 x i32> %63, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffleMerge330 = shufflevector <4 x i32> %shuffle0328, <4 x i32> %shuffle1329, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffle0331 = shufflevector <4 x i32> %60, <4 x i32> %61, <4 x i32> <i32 undef, i32 3, i32 undef, i32 7>
  %shuffle1332 = shufflevector <4 x i32> %62, <4 x i32> %63, <4 x i32> <i32 undef, i32 3, i32 undef, i32 7>
  %shuffleMerge333 = shufflevector <4 x i32> %shuffle0331, <4 x i32> %shuffle1332, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %64 = getelementptr inbounds [9 x <4 x i32>]* %0, i32 0, i32 1
  %65 = getelementptr inbounds [9 x <4 x i32>]* %1, i32 0, i32 1
  %66 = getelementptr inbounds [9 x <4 x i32>]* %2, i32 0, i32 1
  %67 = getelementptr inbounds [9 x <4 x i32>]* %3, i32 0, i32 1
  %68 = load <4 x i32>* %64, align 16
  %69 = load <4 x i32>* %65, align 16
  %70 = load <4 x i32>* %66, align 16
  %71 = load <4 x i32>* %67, align 16
  %shuffle0334 = shufflevector <4 x i32> %68, <4 x i32> %69, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffle1335 = shufflevector <4 x i32> %70, <4 x i32> %71, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffleMerge336 = shufflevector <4 x i32> %shuffle0334, <4 x i32> %shuffle1335, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffle0337 = shufflevector <4 x i32> %68, <4 x i32> %69, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffle1338 = shufflevector <4 x i32> %70, <4 x i32> %71, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffleMerge339 = shufflevector <4 x i32> %shuffle0337, <4 x i32> %shuffle1338, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle0340 = shufflevector <4 x i32> %68, <4 x i32> %69, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffle1341 = shufflevector <4 x i32> %70, <4 x i32> %71, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffleMerge342 = shufflevector <4 x i32> %shuffle0340, <4 x i32> %shuffle1341, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %72 = getelementptr inbounds [9 x <4 x i32>]* %0, i32 0, i32 2
  %73 = getelementptr inbounds [9 x <4 x i32>]* %1, i32 0, i32 2
  %74 = getelementptr inbounds [9 x <4 x i32>]* %2, i32 0, i32 2
  %75 = getelementptr inbounds [9 x <4 x i32>]* %3, i32 0, i32 2
  %76 = load <4 x i32>* %72, align 16
  %77 = load <4 x i32>* %73, align 16
  %78 = load <4 x i32>* %74, align 16
  %79 = load <4 x i32>* %75, align 16
  %shuffle0346 = shufflevector <4 x i32> %76, <4 x i32> %77, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffle1347 = shufflevector <4 x i32> %78, <4 x i32> %79, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffleMerge348 = shufflevector <4 x i32> %shuffle0346, <4 x i32> %shuffle1347, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffle0349 = shufflevector <4 x i32> %76, <4 x i32> %77, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffle1350 = shufflevector <4 x i32> %78, <4 x i32> %79, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffleMerge351 = shufflevector <4 x i32> %shuffle0349, <4 x i32> %shuffle1350, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle0352 = shufflevector <4 x i32> %76, <4 x i32> %77, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffle1353 = shufflevector <4 x i32> %78, <4 x i32> %79, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffleMerge354 = shufflevector <4 x i32> %shuffle0352, <4 x i32> %shuffle1353, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %80 = getelementptr inbounds [9 x <4 x i32>]* %0, i32 0, i32 3
  %81 = getelementptr inbounds [9 x <4 x i32>]* %1, i32 0, i32 3
  %82 = getelementptr inbounds [9 x <4 x i32>]* %2, i32 0, i32 3
  %83 = getelementptr inbounds [9 x <4 x i32>]* %3, i32 0, i32 3
  %84 = load <4 x i32>* %80, align 16
  %85 = load <4 x i32>* %81, align 16
  %86 = load <4 x i32>* %82, align 16
  %87 = load <4 x i32>* %83, align 16
  %shuffle0358 = shufflevector <4 x i32> %84, <4 x i32> %85, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffle1359 = shufflevector <4 x i32> %86, <4 x i32> %87, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffleMerge360 = shufflevector <4 x i32> %shuffle0358, <4 x i32> %shuffle1359, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffle0361 = shufflevector <4 x i32> %84, <4 x i32> %85, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffle1362 = shufflevector <4 x i32> %86, <4 x i32> %87, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffleMerge363 = shufflevector <4 x i32> %shuffle0361, <4 x i32> %shuffle1362, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle0364 = shufflevector <4 x i32> %84, <4 x i32> %85, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffle1365 = shufflevector <4 x i32> %86, <4 x i32> %87, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffleMerge366 = shufflevector <4 x i32> %shuffle0364, <4 x i32> %shuffle1365, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %88 = getelementptr inbounds [9 x <4 x i32>]* %0, i32 0, i32 4
  %89 = getelementptr inbounds [9 x <4 x i32>]* %1, i32 0, i32 4
  %90 = getelementptr inbounds [9 x <4 x i32>]* %2, i32 0, i32 4
  %91 = getelementptr inbounds [9 x <4 x i32>]* %3, i32 0, i32 4
  %92 = load <4 x i32>* %88, align 16
  %93 = load <4 x i32>* %89, align 16
  %94 = load <4 x i32>* %90, align 16
  %95 = load <4 x i32>* %91, align 16
  %shuffle0370 = shufflevector <4 x i32> %92, <4 x i32> %93, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffle1371 = shufflevector <4 x i32> %94, <4 x i32> %95, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffleMerge372 = shufflevector <4 x i32> %shuffle0370, <4 x i32> %shuffle1371, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffle0373 = shufflevector <4 x i32> %92, <4 x i32> %93, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffle1374 = shufflevector <4 x i32> %94, <4 x i32> %95, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffleMerge375 = shufflevector <4 x i32> %shuffle0373, <4 x i32> %shuffle1374, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle0376 = shufflevector <4 x i32> %92, <4 x i32> %93, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffle1377 = shufflevector <4 x i32> %94, <4 x i32> %95, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffleMerge378 = shufflevector <4 x i32> %shuffle0376, <4 x i32> %shuffle1377, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %96 = getelementptr inbounds [9 x <4 x i32>]* %0, i32 0, i32 5
  %97 = getelementptr inbounds [9 x <4 x i32>]* %1, i32 0, i32 5
  %98 = getelementptr inbounds [9 x <4 x i32>]* %2, i32 0, i32 5
  %99 = getelementptr inbounds [9 x <4 x i32>]* %3, i32 0, i32 5
  %100 = load <4 x i32>* %96, align 16
  %101 = load <4 x i32>* %97, align 16
  %102 = load <4 x i32>* %98, align 16
  %103 = load <4 x i32>* %99, align 16
  %shuffle0382 = shufflevector <4 x i32> %100, <4 x i32> %101, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffle1383 = shufflevector <4 x i32> %102, <4 x i32> %103, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffleMerge384 = shufflevector <4 x i32> %shuffle0382, <4 x i32> %shuffle1383, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffle0385 = shufflevector <4 x i32> %100, <4 x i32> %101, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffle1386 = shufflevector <4 x i32> %102, <4 x i32> %103, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffleMerge387 = shufflevector <4 x i32> %shuffle0385, <4 x i32> %shuffle1386, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle0388 = shufflevector <4 x i32> %100, <4 x i32> %101, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffle1389 = shufflevector <4 x i32> %102, <4 x i32> %103, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffleMerge390 = shufflevector <4 x i32> %shuffle0388, <4 x i32> %shuffle1389, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %104 = getelementptr inbounds [9 x <4 x i32>]* %0, i32 0, i32 6
  %105 = getelementptr inbounds [9 x <4 x i32>]* %1, i32 0, i32 6
  %106 = getelementptr inbounds [9 x <4 x i32>]* %2, i32 0, i32 6
  %107 = getelementptr inbounds [9 x <4 x i32>]* %3, i32 0, i32 6
  %108 = load <4 x i32>* %104, align 16
  %109 = load <4 x i32>* %105, align 16
  %110 = load <4 x i32>* %106, align 16
  %111 = load <4 x i32>* %107, align 16
  %shuffle0394 = shufflevector <4 x i32> %108, <4 x i32> %109, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffle1395 = shufflevector <4 x i32> %110, <4 x i32> %111, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffleMerge396 = shufflevector <4 x i32> %shuffle0394, <4 x i32> %shuffle1395, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffle0397 = shufflevector <4 x i32> %108, <4 x i32> %109, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffle1398 = shufflevector <4 x i32> %110, <4 x i32> %111, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffleMerge399 = shufflevector <4 x i32> %shuffle0397, <4 x i32> %shuffle1398, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle0400 = shufflevector <4 x i32> %108, <4 x i32> %109, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffle1401 = shufflevector <4 x i32> %110, <4 x i32> %111, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffleMerge402 = shufflevector <4 x i32> %shuffle0400, <4 x i32> %shuffle1401, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %112 = getelementptr inbounds [9 x <4 x i32>]* %0, i32 0, i32 7
  %113 = getelementptr inbounds [9 x <4 x i32>]* %1, i32 0, i32 7
  %114 = getelementptr inbounds [9 x <4 x i32>]* %2, i32 0, i32 7
  %115 = getelementptr inbounds [9 x <4 x i32>]* %3, i32 0, i32 7
  %116 = load <4 x i32>* %112, align 16
  %117 = load <4 x i32>* %113, align 16
  %118 = load <4 x i32>* %114, align 16
  %119 = load <4 x i32>* %115, align 16
  %shuffle0406 = shufflevector <4 x i32> %116, <4 x i32> %117, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffle1407 = shufflevector <4 x i32> %118, <4 x i32> %119, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffleMerge408 = shufflevector <4 x i32> %shuffle0406, <4 x i32> %shuffle1407, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffle0409 = shufflevector <4 x i32> %116, <4 x i32> %117, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffle1410 = shufflevector <4 x i32> %118, <4 x i32> %119, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffleMerge411 = shufflevector <4 x i32> %shuffle0409, <4 x i32> %shuffle1410, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle0412 = shufflevector <4 x i32> %116, <4 x i32> %117, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffle1413 = shufflevector <4 x i32> %118, <4 x i32> %119, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffleMerge414 = shufflevector <4 x i32> %shuffle0412, <4 x i32> %shuffle1413, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %120 = getelementptr inbounds [9 x <4 x i32>]* %0, i32 0, i32 8
  %121 = getelementptr inbounds [9 x <4 x i32>]* %1, i32 0, i32 8
  %122 = getelementptr inbounds [9 x <4 x i32>]* %2, i32 0, i32 8
  %123 = getelementptr inbounds [9 x <4 x i32>]* %3, i32 0, i32 8
  %124 = load <4 x i32>* %120, align 16
  %125 = load <4 x i32>* %121, align 16
  %126 = load <4 x i32>* %122, align 16
  %127 = load <4 x i32>* %123, align 16
  %shuffle0418 = shufflevector <4 x i32> %124, <4 x i32> %125, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffle1419 = shufflevector <4 x i32> %126, <4 x i32> %127, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffleMerge420 = shufflevector <4 x i32> %shuffle0418, <4 x i32> %shuffle1419, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffle0421 = shufflevector <4 x i32> %124, <4 x i32> %125, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffle1422 = shufflevector <4 x i32> %126, <4 x i32> %127, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffleMerge423 = shufflevector <4 x i32> %shuffle0421, <4 x i32> %shuffle1422, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle0424 = shufflevector <4 x i32> %124, <4 x i32> %125, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffle1425 = shufflevector <4 x i32> %126, <4 x i32> %127, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffleMerge426 = shufflevector <4 x i32> %shuffle0424, <4 x i32> %shuffle1425, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  br label %for.cond34.preheader

for.cond34.preheader:                             ; preds = %for.cond34.preheader, %for.cond29.loopexit
  %iSearch.09 = phi i32 [ 0, %for.cond29.loopexit ], [ %inc65, %for.cond34.preheader ]
  %vectorPHI430 = phi <4 x i32> [ <i32 128, i32 128, i32 128, i32 128>, %for.cond29.loopexit ], [ %shr216550, %for.cond34.preheader ]
  %vectorPHI431 = phi <4 x i32> [ <i32 128, i32 128, i32 128, i32 128>, %for.cond29.loopexit ], [ %shr217551, %for.cond34.preheader ]
  %vectorPHI432 = phi <4 x i32> [ <i32 128, i32 128, i32 128, i32 128>, %for.cond29.loopexit ], [ %shr218552, %for.cond34.preheader ]
  %vectorPHI434 = phi <4 x i32> [ zeroinitializer, %for.cond29.loopexit ], [ %or196530, %for.cond34.preheader ]
  %vectorPHI435 = phi <4 x i32> [ zeroinitializer, %for.cond29.loopexit ], [ %or197531, %for.cond34.preheader ]
  %vectorPHI436 = phi <4 x i32> [ zeroinitializer, %for.cond29.loopexit ], [ %or198532, %for.cond34.preheader ]
  %vectorPHI438 = phi <4 x i32> [ <i32 255, i32 255, i32 255, i32 255>, %for.cond29.loopexit ], [ %or62208542, %for.cond34.preheader ]
  %vectorPHI439 = phi <4 x i32> [ <i32 255, i32 255, i32 255, i32 255>, %for.cond29.loopexit ], [ %or62209543, %for.cond34.preheader ]
  %vectorPHI440 = phi <4 x i32> [ <i32 255, i32 255, i32 255, i32 255>, %for.cond29.loopexit ], [ %or62210544, %for.cond34.preheader ]
  %cmp3936 = icmp sgt <4 x i32> %shuffleMerge324, %vectorPHI430
  %cmp3937 = icmp sgt <4 x i32> %shuffleMerge327, %vectorPHI431
  %cmp3938 = icmp sgt <4 x i32> %shuffleMerge330, %vectorPHI432
  %sext40442 = sext <4 x i1> %cmp3936 to <4 x i32>
  %sext41443 = sext <4 x i1> %cmp3937 to <4 x i32>
  %sext42444 = sext <4 x i1> %cmp3938 to <4 x i32>
  %cmp4348 = icmp sgt <4 x i32> %shuffleMerge336, %vectorPHI430
  %cmp4349 = icmp sgt <4 x i32> %shuffleMerge339, %vectorPHI431
  %cmp4350 = icmp sgt <4 x i32> %shuffleMerge342, %vectorPHI432
  %sext4452446 = sext <4 x i1> %cmp4348 to <4 x i32>
  %sext4453447 = sext <4 x i1> %cmp4349 to <4 x i32>
  %sext4454448 = sext <4 x i1> %cmp4350 to <4 x i32>
  %add4556450 = add <4 x i32> %sext40442, %sext4452446
  %add4557451 = add <4 x i32> %sext41443, %sext4453447
  %add4558452 = add <4 x i32> %sext42444, %sext4454448
  %cmp4864 = icmp sgt <4 x i32> %shuffleMerge348, %vectorPHI430
  %cmp4865 = icmp sgt <4 x i32> %shuffleMerge351, %vectorPHI431
  %cmp4866 = icmp sgt <4 x i32> %shuffleMerge354, %vectorPHI432
  %sext4968454 = sext <4 x i1> %cmp4864 to <4 x i32>
  %sext4969455 = sext <4 x i1> %cmp4865 to <4 x i32>
  %sext4970456 = sext <4 x i1> %cmp4866 to <4 x i32>
  %add5072458 = add <4 x i32> %add4556450, %sext4968454
  %add5073459 = add <4 x i32> %add4557451, %sext4969455
  %add5074460 = add <4 x i32> %add4558452, %sext4970456
  %cmp39.180 = icmp sgt <4 x i32> %shuffleMerge360, %vectorPHI430
  %cmp39.181 = icmp sgt <4 x i32> %shuffleMerge363, %vectorPHI431
  %cmp39.182 = icmp sgt <4 x i32> %shuffleMerge366, %vectorPHI432
  %sext.184462 = sext <4 x i1> %cmp39.180 to <4 x i32>
  %sext.185463 = sext <4 x i1> %cmp39.181 to <4 x i32>
  %sext.186464 = sext <4 x i1> %cmp39.182 to <4 x i32>
  %add40.188466 = add <4 x i32> %add5072458, %sext.184462
  %add40.189467 = add <4 x i32> %add5073459, %sext.185463
  %add40.190468 = add <4 x i32> %add5074460, %sext.186464
  %cmp43.196 = icmp sgt <4 x i32> %shuffleMerge372, %vectorPHI430
  %cmp43.197 = icmp sgt <4 x i32> %shuffleMerge375, %vectorPHI431
  %cmp43.198 = icmp sgt <4 x i32> %shuffleMerge378, %vectorPHI432
  %sext44.1100470 = sext <4 x i1> %cmp43.196 to <4 x i32>
  %sext44.1101471 = sext <4 x i1> %cmp43.197 to <4 x i32>
  %sext44.1102472 = sext <4 x i1> %cmp43.198 to <4 x i32>
  %add45.1104474 = add <4 x i32> %add40.188466, %sext44.1100470
  %add45.1105475 = add <4 x i32> %add40.189467, %sext44.1101471
  %add45.1106476 = add <4 x i32> %add40.190468, %sext44.1102472
  %cmp48.1112 = icmp sgt <4 x i32> %shuffleMerge384, %vectorPHI430
  %cmp48.1113 = icmp sgt <4 x i32> %shuffleMerge387, %vectorPHI431
  %cmp48.1114 = icmp sgt <4 x i32> %shuffleMerge390, %vectorPHI432
  %sext49.1116478 = sext <4 x i1> %cmp48.1112 to <4 x i32>
  %sext49.1117479 = sext <4 x i1> %cmp48.1113 to <4 x i32>
  %sext49.1118480 = sext <4 x i1> %cmp48.1114 to <4 x i32>
  %add50.1120482 = add <4 x i32> %add45.1104474, %sext49.1116478
  %add50.1121483 = add <4 x i32> %add45.1105475, %sext49.1117479
  %add50.1122484 = add <4 x i32> %add45.1106476, %sext49.1118480
  %cmp39.2128 = icmp sgt <4 x i32> %shuffleMerge396, %vectorPHI430
  %cmp39.2129 = icmp sgt <4 x i32> %shuffleMerge399, %vectorPHI431
  %cmp39.2130 = icmp sgt <4 x i32> %shuffleMerge402, %vectorPHI432
  %sext.2132486 = sext <4 x i1> %cmp39.2128 to <4 x i32>
  %sext.2133487 = sext <4 x i1> %cmp39.2129 to <4 x i32>
  %sext.2134488 = sext <4 x i1> %cmp39.2130 to <4 x i32>
  %add40.2136490 = add <4 x i32> %add50.1120482, %sext.2132486
  %add40.2137491 = add <4 x i32> %add50.1121483, %sext.2133487
  %add40.2138492 = add <4 x i32> %add50.1122484, %sext.2134488
  %cmp43.2144 = icmp sgt <4 x i32> %shuffleMerge408, %vectorPHI430
  %cmp43.2145 = icmp sgt <4 x i32> %shuffleMerge411, %vectorPHI431
  %cmp43.2146 = icmp sgt <4 x i32> %shuffleMerge414, %vectorPHI432
  %sext44.2148494 = sext <4 x i1> %cmp43.2144 to <4 x i32>
  %sext44.2149495 = sext <4 x i1> %cmp43.2145 to <4 x i32>
  %sext44.2150496 = sext <4 x i1> %cmp43.2146 to <4 x i32>
  %add45.2152498 = add <4 x i32> %add40.2136490, %sext44.2148494
  %add45.2153499 = add <4 x i32> %add40.2137491, %sext44.2149495
  %add45.2154500 = add <4 x i32> %add40.2138492, %sext44.2150496
  %cmp48.2160 = icmp sgt <4 x i32> %shuffleMerge420, %vectorPHI430
  %cmp48.2161 = icmp sgt <4 x i32> %shuffleMerge423, %vectorPHI431
  %cmp48.2162 = icmp sgt <4 x i32> %shuffleMerge426, %vectorPHI432
  %sext49.2164502 = sext <4 x i1> %cmp48.2160 to <4 x i32>
  %sext49.2165503 = sext <4 x i1> %cmp48.2161 to <4 x i32>
  %sext49.2166504 = sext <4 x i1> %cmp48.2162 to <4 x i32>
  %add50.2168506 = add <4 x i32> %add45.2152498, %sext49.2164502
  %add50.2169507 = add <4 x i32> %add45.2153499, %sext49.2165503
  %add50.2170508 = add <4 x i32> %add45.2154500, %sext49.2166504
  %sub55172510 = sub <4 x i32> zeroinitializer, %add50.2168506
  %sub55173511 = sub <4 x i32> zeroinitializer, %add50.2169507
  %sub55174512 = sub <4 x i32> zeroinitializer, %add50.2170508
  %cmp56176 = icmp sgt <4 x i32> %sub55172510, <i32 4, i32 4, i32 4, i32 4>
  %cmp56177 = icmp sgt <4 x i32> %sub55173511, <i32 4, i32 4, i32 4, i32 4>
  %cmp56178 = icmp sgt <4 x i32> %sub55174512, <i32 4, i32 4, i32 4, i32 4>
  %sext57180514 = sext <4 x i1> %cmp56176 to <4 x i32>
  %sext57181515 = sext <4 x i1> %cmp56177 to <4 x i32>
  %sext57182516 = sext <4 x i1> %cmp56178 to <4 x i32>
  %and184518 = and <4 x i32> %vectorPHI430, %sext57180514
  %and185519 = and <4 x i32> %vectorPHI431, %sext57181515
  %and186520 = and <4 x i32> %vectorPHI432, %sext57182516
  %neg188522 = xor <4 x i32> %sext57180514, <i32 -1, i32 -1, i32 -1, i32 -1>
  %neg189523 = xor <4 x i32> %sext57181515, <i32 -1, i32 -1, i32 -1, i32 -1>
  %neg190524 = xor <4 x i32> %sext57182516, <i32 -1, i32 -1, i32 -1, i32 -1>
  %and58192526 = and <4 x i32> %vectorPHI434, %neg188522
  %and58193527 = and <4 x i32> %vectorPHI435, %neg189523
  %and58194528 = and <4 x i32> %vectorPHI436, %neg190524
  %or196530 = or <4 x i32> %and184518, %and58192526
  %or197531 = or <4 x i32> %and185519, %and58193527
  %or198532 = or <4 x i32> %and186520, %and58194528
  %and60200534 = and <4 x i32> %vectorPHI430, %neg188522
  %and60201535 = and <4 x i32> %vectorPHI431, %neg189523
  %and60202536 = and <4 x i32> %vectorPHI432, %neg190524
  %and61204538 = and <4 x i32> %vectorPHI438, %sext57180514
  %and61205539 = and <4 x i32> %vectorPHI439, %sext57181515
  %and61206540 = and <4 x i32> %vectorPHI440, %sext57182516
  %or62208542 = or <4 x i32> %and60200534, %and61204538
  %or62209543 = or <4 x i32> %and60201535, %and61205539
  %or62210544 = or <4 x i32> %and60202536, %and61206540
  %add63212546 = add <4 x i32> %or62208542, %or196530
  %add63213547 = add <4 x i32> %or62209543, %or197531
  %add63214548 = add <4 x i32> %or62210544, %or198532
  %shr216550 = ashr <4 x i32> %add63212546, <i32 1, i32 1, i32 1, i32 1>
  %shr217551 = ashr <4 x i32> %add63213547, <i32 1, i32 1, i32 1, i32 1>
  %shr218552 = ashr <4 x i32> %add63214548, <i32 1, i32 1, i32 1, i32 1>
  %inc65 = add nsw i32 %iSearch.09, 1
  %exitcond13 = icmp eq i32 %inc65, 8
  br i1 %exitcond13, label %for.end66, label %for.cond34.preheader

for.end66:                                        ; preds = %for.cond34.preheader
  %and67554 = and <4 x i32> %shr216550, <i32 255, i32 255, i32 255, i32 255>
  %shl555 = shl <4 x i32> %shr217551, <i32 8, i32 8, i32 8, i32 8>
  %and68556 = and <4 x i32> %shl555, <i32 65280, i32 65280, i32 65280, i32 65280>
  %shl70557 = shl <4 x i32> %shr218552, <i32 16, i32 16, i32 16, i32 16>
  %and71558 = and <4 x i32> %shl70557, <i32 16711680, i32 16711680, i32 16711680, i32 16711680>
  %or69559 = or <4 x i32> %and71558, %and67554
  %or72560 = or <4 x i32> %or69559, %and68556
  %extract566 = extractelement <4 x i32> %or72560, i32 0
  %extract567 = extractelement <4 x i32> %or72560, i32 1
  %extract568 = extractelement <4 x i32> %or72560, i32 2
  %extract569 = extractelement <4 x i32> %or72560, i32 3
  %add75561 = add nsw <4 x i32> %vector243, %mul74236
  %extract562 = extractelement <4 x i32> %add75561, i32 0
  %extract563 = extractelement <4 x i32> %add75561, i32 1
  %extract564 = extractelement <4 x i32> %add75561, i32 2
  %extract565 = extractelement <4 x i32> %add75561, i32 3
  %128 = getelementptr inbounds i32 addrspace(1)* %pDst, i32 %extract562
  %129 = getelementptr inbounds i32 addrspace(1)* %pDst, i32 %extract563
  %130 = getelementptr inbounds i32 addrspace(1)* %pDst, i32 %extract564
  %131 = getelementptr inbounds i32 addrspace(1)* %pDst, i32 %extract565
  store i32 %extract566, i32 addrspace(1)* %128, align 4
  store i32 %extract567, i32 addrspace(1)* %129, align 4
  store i32 %extract568, i32 addrspace(1)* %130, align 4
  store i32 %extract569, i32 addrspace(1)* %131, align 4
  %inc78 = add nsw i32 %x.011, 1
  %exitcond14 = icmp eq i32 %inc78, %iImageWidth
  br i1 %exitcond14, label %for.end79, label %for.cond1.preheader

for.end79:                                        ; preds = %for.end66
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

for.cond1.preheader:                              ; preds = %for.end68, %entry
  %x.012 = phi i32 [ 0, %entry ], [ %inc81, %for.end68 ]
  %temp29 = insertelement <4 x i32> undef, i32 %x.012, i32 0
  %vector30 = shufflevector <4 x i32> %temp29, <4 x i32> undef, <4 x i32> zeroinitializer
  br label %for.cond4.preheader

for.cond4.preheader:                              ; preds = %for.end64, %for.cond1.preheader
  %ch.010 = phi i32 [ 0, %for.cond1.preheader ], [ %inc67, %for.end64 ]
  %temp44 = insertelement <4 x i32> undef, i32 %ch.010, i32 0
  %vector45 = shufflevector <4 x i32> %temp44, <4 x i32> undef, <4 x i32> zeroinitializer
  %mul9 = add i32 %ch.010, -4
  %temp33 = insertelement <4 x i32> undef, i32 %mul9, i32 0
  %vector34 = shufflevector <4 x i32> %temp33, <4 x i32> undef, <4 x i32> zeroinitializer
  %mul19 = add i32 %ch.010, 4
  %temp56 = insertelement <4 x i32> undef, i32 %mul19, i32 0
  %vector57 = shufflevector <4 x i32> %temp56, <4 x i32> undef, <4 x i32> zeroinitializer
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
  %temp.vect = insertelement <4 x i8> undef, i8 %20, i32 0
  %temp.vect39 = insertelement <4 x i8> %temp.vect, i8 %21, i32 1
  %temp.vect40 = insertelement <4 x i8> %temp.vect39, i8 %22, i32 2
  %temp.vect41 = insertelement <4 x i8> %temp.vect40, i8 %23, i32 3
  %conv43 = zext <4 x i8> %temp.vect41 to <4 x i32>
  %add1346 = add nsw <4 x i32> %sub32, %vector45
  %extract47 = extractelement <4 x i32> %add1346, i32 0
  %extract48 = extractelement <4 x i32> %add1346, i32 1
  %extract49 = extractelement <4 x i32> %add1346, i32 2
  %extract50 = extractelement <4 x i32> %add1346, i32 3
  %24 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract47
  %25 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract48
  %26 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract49
  %27 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract50
  %28 = load i8 addrspace(1)* %24, align 1
  %29 = load i8 addrspace(1)* %25, align 1
  %30 = load i8 addrspace(1)* %26, align 1
  %31 = load i8 addrspace(1)* %27, align 1
  %temp.vect51 = insertelement <4 x i8> undef, i8 %28, i32 0
  %temp.vect52 = insertelement <4 x i8> %temp.vect51, i8 %29, i32 1
  %temp.vect53 = insertelement <4 x i8> %temp.vect52, i8 %30, i32 2
  %temp.vect54 = insertelement <4 x i8> %temp.vect53, i8 %31, i32 3
  %conv1555 = zext <4 x i8> %temp.vect54 to <4 x i32>
  %add2058 = add <4 x i32> %vector57, %sub32
  %extract59 = extractelement <4 x i32> %add2058, i32 0
  %extract60 = extractelement <4 x i32> %add2058, i32 1
  %extract61 = extractelement <4 x i32> %add2058, i32 2
  %extract62 = extractelement <4 x i32> %add2058, i32 3
  %32 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract59
  %33 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract60
  %34 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract61
  %35 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract62
  %36 = load i8 addrspace(1)* %32, align 1
  %37 = load i8 addrspace(1)* %33, align 1
  %38 = load i8 addrspace(1)* %34, align 1
  %39 = load i8 addrspace(1)* %35, align 1
  %temp.vect63 = insertelement <4 x i8> undef, i8 %36, i32 0
  %temp.vect64 = insertelement <4 x i8> %temp.vect63, i8 %37, i32 1
  %temp.vect65 = insertelement <4 x i8> %temp.vect64, i8 %38, i32 2
  %temp.vect66 = insertelement <4 x i8> %temp.vect65, i8 %39, i32 3
  %conv2267 = zext <4 x i8> %temp.vect66 to <4 x i32>
  %add8.168 = add nsw <4 x i32> %mul7726, %vector30
  %sub.169 = shl <4 x i32> %add8.168, <i32 2, i32 2, i32 2, i32 2>
  %add10.170 = add <4 x i32> %vector34, %sub.169
  %extract71 = extractelement <4 x i32> %add10.170, i32 0
  %extract72 = extractelement <4 x i32> %add10.170, i32 1
  %extract73 = extractelement <4 x i32> %add10.170, i32 2
  %extract74 = extractelement <4 x i32> %add10.170, i32 3
  %40 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract71
  %41 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract72
  %42 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract73
  %43 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract74
  %44 = load i8 addrspace(1)* %40, align 1
  %45 = load i8 addrspace(1)* %41, align 1
  %46 = load i8 addrspace(1)* %42, align 1
  %47 = load i8 addrspace(1)* %43, align 1
  %temp.vect75 = insertelement <4 x i8> undef, i8 %44, i32 0
  %temp.vect76 = insertelement <4 x i8> %temp.vect75, i8 %45, i32 1
  %temp.vect77 = insertelement <4 x i8> %temp.vect76, i8 %46, i32 2
  %temp.vect78 = insertelement <4 x i8> %temp.vect77, i8 %47, i32 3
  %conv.179 = zext <4 x i8> %temp.vect78 to <4 x i32>
  %add13.180 = add nsw <4 x i32> %sub.169, %vector45
  %extract81 = extractelement <4 x i32> %add13.180, i32 0
  %extract82 = extractelement <4 x i32> %add13.180, i32 1
  %extract83 = extractelement <4 x i32> %add13.180, i32 2
  %extract84 = extractelement <4 x i32> %add13.180, i32 3
  %48 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract81
  %49 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract82
  %50 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract83
  %51 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract84
  %52 = load i8 addrspace(1)* %48, align 1
  %53 = load i8 addrspace(1)* %49, align 1
  %54 = load i8 addrspace(1)* %50, align 1
  %55 = load i8 addrspace(1)* %51, align 1
  %temp.vect85 = insertelement <4 x i8> undef, i8 %52, i32 0
  %temp.vect86 = insertelement <4 x i8> %temp.vect85, i8 %53, i32 1
  %temp.vect87 = insertelement <4 x i8> %temp.vect86, i8 %54, i32 2
  %temp.vect88 = insertelement <4 x i8> %temp.vect87, i8 %55, i32 3
  %conv15.189 = zext <4 x i8> %temp.vect88 to <4 x i32>
  %add20.190 = add <4 x i32> %vector57, %sub.169
  %extract91 = extractelement <4 x i32> %add20.190, i32 0
  %extract92 = extractelement <4 x i32> %add20.190, i32 1
  %extract93 = extractelement <4 x i32> %add20.190, i32 2
  %extract94 = extractelement <4 x i32> %add20.190, i32 3
  %56 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract91
  %57 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract92
  %58 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract93
  %59 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract94
  %60 = load i8 addrspace(1)* %56, align 1
  %61 = load i8 addrspace(1)* %57, align 1
  %62 = load i8 addrspace(1)* %58, align 1
  %63 = load i8 addrspace(1)* %59, align 1
  %temp.vect95 = insertelement <4 x i8> undef, i8 %60, i32 0
  %temp.vect96 = insertelement <4 x i8> %temp.vect95, i8 %61, i32 1
  %temp.vect97 = insertelement <4 x i8> %temp.vect96, i8 %62, i32 2
  %temp.vect98 = insertelement <4 x i8> %temp.vect97, i8 %63, i32 3
  %conv22.199 = zext <4 x i8> %temp.vect98 to <4 x i32>
  %add7.2100 = add <4 x i32> %broadcast2, <i32 3, i32 4, i32 5, i32 6>
  %mul.2101 = mul nsw <4 x i32> %add7.2100, %vector
  %add8.2102 = add nsw <4 x i32> %mul.2101, %vector30
  %sub.2103 = shl <4 x i32> %add8.2102, <i32 2, i32 2, i32 2, i32 2>
  %add10.2104 = add <4 x i32> %vector34, %sub.2103
  %extract105 = extractelement <4 x i32> %add10.2104, i32 0
  %extract106 = extractelement <4 x i32> %add10.2104, i32 1
  %extract107 = extractelement <4 x i32> %add10.2104, i32 2
  %extract108 = extractelement <4 x i32> %add10.2104, i32 3
  %64 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract105
  %65 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract106
  %66 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract107
  %67 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract108
  %68 = load i8 addrspace(1)* %64, align 1
  %69 = load i8 addrspace(1)* %65, align 1
  %70 = load i8 addrspace(1)* %66, align 1
  %71 = load i8 addrspace(1)* %67, align 1
  %temp.vect109 = insertelement <4 x i8> undef, i8 %68, i32 0
  %temp.vect110 = insertelement <4 x i8> %temp.vect109, i8 %69, i32 1
  %temp.vect111 = insertelement <4 x i8> %temp.vect110, i8 %70, i32 2
  %temp.vect112 = insertelement <4 x i8> %temp.vect111, i8 %71, i32 3
  %conv.2113 = zext <4 x i8> %temp.vect112 to <4 x i32>
  %add13.2114 = add nsw <4 x i32> %sub.2103, %vector45
  %extract115 = extractelement <4 x i32> %add13.2114, i32 0
  %extract116 = extractelement <4 x i32> %add13.2114, i32 1
  %extract117 = extractelement <4 x i32> %add13.2114, i32 2
  %extract118 = extractelement <4 x i32> %add13.2114, i32 3
  %72 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract115
  %73 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract116
  %74 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract117
  %75 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract118
  %76 = load i8 addrspace(1)* %72, align 1
  %77 = load i8 addrspace(1)* %73, align 1
  %78 = load i8 addrspace(1)* %74, align 1
  %79 = load i8 addrspace(1)* %75, align 1
  %temp.vect119 = insertelement <4 x i8> undef, i8 %76, i32 0
  %temp.vect120 = insertelement <4 x i8> %temp.vect119, i8 %77, i32 1
  %temp.vect121 = insertelement <4 x i8> %temp.vect120, i8 %78, i32 2
  %temp.vect122 = insertelement <4 x i8> %temp.vect121, i8 %79, i32 3
  %conv15.2123 = zext <4 x i8> %temp.vect122 to <4 x i32>
  %add20.2124 = add <4 x i32> %vector57, %sub.2103
  %extract125 = extractelement <4 x i32> %add20.2124, i32 0
  %extract126 = extractelement <4 x i32> %add20.2124, i32 1
  %extract127 = extractelement <4 x i32> %add20.2124, i32 2
  %extract128 = extractelement <4 x i32> %add20.2124, i32 3
  %80 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract125
  %81 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract126
  %82 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract127
  %83 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract128
  %84 = load i8 addrspace(1)* %80, align 1
  %85 = load i8 addrspace(1)* %81, align 1
  %86 = load i8 addrspace(1)* %82, align 1
  %87 = load i8 addrspace(1)* %83, align 1
  %temp.vect129 = insertelement <4 x i8> undef, i8 %84, i32 0
  %temp.vect130 = insertelement <4 x i8> %temp.vect129, i8 %85, i32 1
  %temp.vect131 = insertelement <4 x i8> %temp.vect130, i8 %86, i32 2
  %temp.vect132 = insertelement <4 x i8> %temp.vect131, i8 %87, i32 3
  %conv22.2133 = zext <4 x i8> %temp.vect132 to <4 x i32>
  br label %for.cond31.preheader

for.cond31.preheader:                             ; preds = %for.cond31.preheader, %for.cond4.preheader
  %iSearch.09 = phi i32 [ 0, %for.cond4.preheader ], [ %inc63, %for.cond31.preheader ]
  %vectorPHI = phi <4 x i32> [ <i32 255, i32 255, i32 255, i32 255>, %for.cond4.preheader ], [ %108, %for.cond31.preheader ]
  %vectorPHI134 = phi <4 x i32> [ zeroinitializer, %for.cond4.preheader ], [ %102, %for.cond31.preheader ]
  %vectorPHI135 = phi <4 x i32> [ <i32 128, i32 128, i32 128, i32 128>, %for.cond4.preheader ], [ %shr156, %for.cond31.preheader ]
  %88 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %conv43, <4 x i32> %vectorPHI135)
  %msb = and <4 x i32> %88, <i32 1, i32 1, i32 1, i32 1>
  %89 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %conv1555, <4 x i32> %vectorPHI135)
  %msb192 = and <4 x i32> %89, <i32 1, i32 1, i32 1, i32 1>
  %90 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %conv2267, <4 x i32> %vectorPHI135)
  %msb194 = and <4 x i32> %90, <i32 1, i32 1, i32 1, i32 1>
  %add43139 = add <4 x i32> %msb, %msb192
  %add48140 = add <4 x i32> %add43139, %msb194
  %91 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %conv.179, <4 x i32> %vectorPHI135)
  %msb196 = and <4 x i32> %91, <i32 1, i32 1, i32 1, i32 1>
  %92 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %conv15.189, <4 x i32> %vectorPHI135)
  %msb198 = and <4 x i32> %92, <i32 1, i32 1, i32 1, i32 1>
  %93 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %conv22.199, <4 x i32> %vectorPHI135)
  %msb200 = and <4 x i32> %93, <i32 1, i32 1, i32 1, i32 1>
  %add38.1144 = add <4 x i32> %msb196, %add48140
  %add43.1145 = add <4 x i32> %add38.1144, %msb198
  %add48.1146 = add <4 x i32> %add43.1145, %msb200
  %94 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %conv.2113, <4 x i32> %vectorPHI135)
  %msb202 = and <4 x i32> %94, <i32 1, i32 1, i32 1, i32 1>
  %95 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %conv15.2123, <4 x i32> %vectorPHI135)
  %msb204 = and <4 x i32> %95, <i32 1, i32 1, i32 1, i32 1>
  %96 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %conv22.2133, <4 x i32> %vectorPHI135)
  %msb206 = and <4 x i32> %96, <i32 1, i32 1, i32 1, i32 1>
  %add38.2150 = add <4 x i32> %msb202, %add48.1146
  %add43.2151 = add <4 x i32> %add38.2150, %msb204
  %add48.2152 = add <4 x i32> %add43.2151, %msb206
  %97 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %add48.2152, <4 x i32> <i32 4, i32 4, i32 4, i32 4>)
  %98 = bitcast <4 x i32> %vectorPHI135 to <4 x float>
  %99 = bitcast <4 x i32> %vectorPHI134 to <4 x float>
  %100 = bitcast <4 x i32> %97 to <4 x float>
  %101 = call <4 x float> @llvm.x86.sse41.blendvps(<4 x float> %99, <4 x float> %98, <4 x float> %100)
  %102 = bitcast <4 x float> %101 to <4 x i32>
  %103 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> <i32 5, i32 5, i32 5, i32 5>, <4 x i32> %add48.2152)
  %104 = bitcast <4 x i32> %vectorPHI135 to <4 x float>
  %105 = bitcast <4 x i32> %vectorPHI to <4 x float>
  %106 = bitcast <4 x i32> %103 to <4 x float>
  %107 = call <4 x float> @llvm.x86.sse41.blendvps(<4 x float> %105, <4 x float> %104, <4 x float> %106)
  %108 = bitcast <4 x float> %107 to <4 x i32>
  %add61155 = add nsw <4 x i32> %108, %102
  %shr156 = ashr <4 x i32> %add61155, <i32 1, i32 1, i32 1, i32 1>
  %inc63 = add nsw i32 %iSearch.09, 1
  %exitcond = icmp eq i32 %inc63, 8
  br i1 %exitcond, label %for.end64, label %for.cond31.preheader

for.end64:                                        ; preds = %for.cond31.preheader
  %extract160 = extractelement <4 x i32> %shr156, i32 3
  %extract159 = extractelement <4 x i32> %shr156, i32 2
  %extract158 = extractelement <4 x i32> %shr156, i32 1
  %extract157 = extractelement <4 x i32> %shr156, i32 0
  %109 = getelementptr inbounds [4 x i32]* %0, i32 0, i32 %ch.010
  %110 = getelementptr inbounds [4 x i32]* %1, i32 0, i32 %ch.010
  %111 = getelementptr inbounds [4 x i32]* %2, i32 0, i32 %ch.010
  %112 = getelementptr inbounds [4 x i32]* %3, i32 0, i32 %ch.010
  store i32 %extract157, i32* %109, align 4
  store i32 %extract158, i32* %110, align 4
  store i32 %extract159, i32* %111, align 4
  store i32 %extract160, i32* %112, align 4
  %inc67 = add nsw i32 %ch.010, 1
  %exitcond13 = icmp eq i32 %inc67, 3
  br i1 %exitcond13, label %for.end68, label %for.cond4.preheader

for.end68:                                        ; preds = %for.end64
  %113 = load i32* %4, align 4
  %114 = load i32* %5, align 4
  %115 = load i32* %6, align 4
  %116 = load i32* %7, align 4
  %temp.vect161 = insertelement <4 x i32> undef, i32 %113, i32 0
  %temp.vect162 = insertelement <4 x i32> %temp.vect161, i32 %114, i32 1
  %temp.vect163 = insertelement <4 x i32> %temp.vect162, i32 %115, i32 2
  %temp.vect164 = insertelement <4 x i32> %temp.vect163, i32 %116, i32 3
  %and165 = and <4 x i32> %temp.vect164, <i32 255, i32 255, i32 255, i32 255>
  %117 = load i32* %8, align 4
  %118 = load i32* %9, align 4
  %119 = load i32* %10, align 4
  %120 = load i32* %11, align 4
  %temp.vect166 = insertelement <4 x i32> undef, i32 %117, i32 0
  %temp.vect167 = insertelement <4 x i32> %temp.vect166, i32 %118, i32 1
  %temp.vect168 = insertelement <4 x i32> %temp.vect167, i32 %119, i32 2
  %temp.vect169 = insertelement <4 x i32> %temp.vect168, i32 %120, i32 3
  %shl170 = shl <4 x i32> %temp.vect169, <i32 8, i32 8, i32 8, i32 8>
  %and71171 = and <4 x i32> %shl170, <i32 65280, i32 65280, i32 65280, i32 65280>
  %121 = load i32* %12, align 4
  %122 = load i32* %13, align 4
  %123 = load i32* %14, align 4
  %124 = load i32* %15, align 4
  %temp.vect172 = insertelement <4 x i32> undef, i32 %121, i32 0
  %temp.vect173 = insertelement <4 x i32> %temp.vect172, i32 %122, i32 1
  %temp.vect174 = insertelement <4 x i32> %temp.vect173, i32 %123, i32 2
  %temp.vect175 = insertelement <4 x i32> %temp.vect174, i32 %124, i32 3
  %shl73176 = shl <4 x i32> %temp.vect175, <i32 16, i32 16, i32 16, i32 16>
  %and74177 = and <4 x i32> %shl73176, <i32 16711680, i32 16711680, i32 16711680, i32 16711680>
  %or178 = or <4 x i32> %and71171, %and165
  %or75179 = or <4 x i32> %or178, %and74177
  %extract185 = extractelement <4 x i32> %or75179, i32 0
  %extract186 = extractelement <4 x i32> %or75179, i32 1
  %extract187 = extractelement <4 x i32> %or75179, i32 2
  %extract188 = extractelement <4 x i32> %or75179, i32 3
  %add78180 = add nsw <4 x i32> %vector30, %mul7726
  %extract181 = extractelement <4 x i32> %add78180, i32 0
  %extract182 = extractelement <4 x i32> %add78180, i32 1
  %extract183 = extractelement <4 x i32> %add78180, i32 2
  %extract184 = extractelement <4 x i32> %add78180, i32 3
  %125 = getelementptr inbounds i32 addrspace(1)* %pDst, i32 %extract181
  %126 = getelementptr inbounds i32 addrspace(1)* %pDst, i32 %extract182
  %127 = getelementptr inbounds i32 addrspace(1)* %pDst, i32 %extract183
  %128 = getelementptr inbounds i32 addrspace(1)* %pDst, i32 %extract184
  store i32 %extract185, i32 addrspace(1)* %125, align 4
  store i32 %extract186, i32 addrspace(1)* %126, align 4
  store i32 %extract187, i32 addrspace(1)* %127, align 4
  store i32 %extract188, i32 addrspace(1)* %128, align 4
  %inc81 = add nsw i32 %x.012, 1
  %exitcond14 = icmp eq i32 %inc81, %iImageWidth
  br i1 %exitcond14, label %for.end82, label %for.cond1.preheader

for.end82:                                        ; preds = %for.end68
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

define <8 x i32> @local.avx256.pcmpeq.d1(<8 x i32>, <8 x i32>) {
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

define <8 x i32> @local.avx256.pcmpgt.d2(<8 x i32>, <8 x i32>) {
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

declare <4 x float> @llvm.x86.sse41.blendvps(<4 x float>, <4 x float>, <4 x float>) nounwind readnone

!opencl.kernels = !{!0, !8}
!opencl.build.options = !{!11}
!cl.noBarrierPath.kernels = !{!12}

!0 = metadata !{void (<4 x i8> addrspace(1)*, i32 addrspace(1)*, i32, i32)* @intel_median, metadata !1, metadata !2}
!1 = metadata !{metadata !"image_access_qualifier", i32 3, i32 3, i32 3, i32 3}
!2 = metadata !{metadata !"cl_kernel_arg_info", metadata !3, metadata !4, metadata !5, metadata !6, metadata !7}
!3 = metadata !{i32 0, i32 0, i32 3, i32 3}
!4 = metadata !{i32 3, i32 3, i32 3, i32 3}
!5 = metadata !{metadata !"uchar4 *", metadata !"unsigned int *", metadata !"int", metadata !"int"}
!6 = metadata !{i32 0, i32 0, i32 0, i32 0}
!7 = metadata !{metadata !"pSrc", metadata !"pDst", metadata !"iImageWidth", metadata !"iImageHeight"}
!8 = metadata !{void (i8 addrspace(1)*, i32 addrspace(1)*, i32, i32)* @intel_median_scalar, metadata !1, metadata !9}
!9 = metadata !{metadata !"cl_kernel_arg_info", metadata !3, metadata !4, metadata !10, metadata !6, metadata !7}
!10 = metadata !{metadata !"uchar *", metadata !"unsigned int *", metadata !"int", metadata !"int"}
!11 = metadata !{metadata !"-cl-kernel-arg-info"}
!12 = metadata !{metadata !"intel_median", metadata !"intel_median_scalar"}
