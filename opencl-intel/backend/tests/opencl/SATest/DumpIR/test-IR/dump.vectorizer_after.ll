; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @intel_median(<4 x i8> addrspace(1)* nocapture %pSrc, i32 addrspace(1)* nocapture %pDst, i32 %iImageWidth, i32 %iImageHeight) nounwind {
entry:
  %iPixels = alloca [9 x <4 x i32>], align 16
  %call = call i64 @get_global_id(i32 0) nounwind readnone
  %conv = trunc i64 %call to i32
  %add = add i32 %conv, 2
  %mul91 = mul nsw i32 %add, %iImageWidth
  %0 = sext i32 %mul91 to i64
  br label %for.cond2.preheader

for.cond2.preheader:                              ; preds = %for.end83, %entry
  %1 = phi <4 x i32> [ undef, %entry ], [ %.pre, %for.end83 ]
  %indvars.iv22 = phi i64 [ 0, %entry ], [ %indvars.iv.next23, %for.end83 ]
  br label %for.body5

for.body5:                                        ; preds = %for.body5.for.body5_crit_edge, %for.cond2.preheader
  %2 = phi <4 x i32> [ %1, %for.cond2.preheader ], [ %.pre35, %for.body5.for.body5_crit_edge ]
  %indvars.iv12 = phi i64 [ 0, %for.cond2.preheader ], [ %indvars.iv.next13, %for.body5.for.body5_crit_edge ]
  %indvars.iv = phi i64 [ -1, %for.cond2.preheader ], [ %indvars.iv.next, %for.body5.for.body5_crit_edge ]
  %3 = trunc i64 %indvars.iv to i32
  %add6 = add i32 %add, %3
  %mul = mul nsw i32 %add6, %iImageWidth
  %4 = trunc i64 %indvars.iv22 to i32
  %add7 = add nsw i32 %mul, %4
  %sub = add nsw i32 %add7, -1
  %idxprom = sext i32 %sub to i64
  %arrayidx = getelementptr inbounds <4 x i8> addrspace(1)* %pSrc, i64 %idxprom
  %5 = load <4 x i8> addrspace(1)* %arrayidx, align 4
  %6 = extractelement <4 x i8> %5, i32 0
  %conv8 = zext i8 %6 to i32
  %arrayidx10 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i64 0, i64 %indvars.iv12
  %7 = insertelement <4 x i32> %2, i32 %conv8, i32 0
  %8 = extractelement <4 x i8> %5, i32 1
  %conv11 = zext i8 %8 to i32
  %9 = insertelement <4 x i32> %7, i32 %conv11, i32 1
  %10 = extractelement <4 x i8> %5, i32 2
  %conv14 = zext i8 %10 to i32
  %11 = insertelement <4 x i32> %9, i32 %conv14, i32 2
  store <4 x i32> %11, <4 x i32>* %arrayidx10, align 16
  %12 = add nsw i64 %indvars.iv12, 1
  %idxprom17 = sext i32 %add7 to i64
  %arrayidx18 = getelementptr inbounds <4 x i8> addrspace(1)* %pSrc, i64 %idxprom17
  %13 = load <4 x i8> addrspace(1)* %arrayidx18, align 4
  %14 = extractelement <4 x i8> %13, i32 0
  %conv19 = zext i8 %14 to i32
  %arrayidx21 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i64 0, i64 %12
  %15 = load <4 x i32>* %arrayidx21, align 16
  %16 = insertelement <4 x i32> %15, i32 %conv19, i32 0
  %17 = extractelement <4 x i8> %13, i32 1
  %conv22 = zext i8 %17 to i32
  %18 = insertelement <4 x i32> %16, i32 %conv22, i32 1
  %19 = extractelement <4 x i8> %13, i32 2
  %conv25 = zext i8 %19 to i32
  %20 = insertelement <4 x i32> %18, i32 %conv25, i32 2
  store <4 x i32> %20, <4 x i32>* %arrayidx21, align 16
  %21 = add nsw i64 %indvars.iv12, 2
  %add29 = add nsw i32 %add7, 1
  %idxprom30 = sext i32 %add29 to i64
  %arrayidx31 = getelementptr inbounds <4 x i8> addrspace(1)* %pSrc, i64 %idxprom30
  %22 = load <4 x i8> addrspace(1)* %arrayidx31, align 4
  %23 = extractelement <4 x i8> %22, i32 0
  %conv32 = zext i8 %23 to i32
  %arrayidx34 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i64 0, i64 %21
  %24 = load <4 x i32>* %arrayidx34, align 16
  %25 = insertelement <4 x i32> %24, i32 %conv32, i32 0
  %26 = extractelement <4 x i8> %22, i32 1
  %conv35 = zext i8 %26 to i32
  %27 = insertelement <4 x i32> %25, i32 %conv35, i32 1
  %28 = extractelement <4 x i8> %22, i32 2
  %conv38 = zext i8 %28 to i32
  %29 = insertelement <4 x i32> %27, i32 %conv38, i32 2
  store <4 x i32> %29, <4 x i32>* %arrayidx34, align 16
  %indvars.iv.next13 = add i64 %indvars.iv12, 3
  %indvars.iv.next = add i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, 2
  br i1 %exitcond, label %for.cond43.loopexit, label %for.body5.for.body5_crit_edge

for.body5.for.body5_crit_edge:                    ; preds = %for.body5
  %arrayidx10.phi.trans.insert = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i64 0, i64 %indvars.iv.next13
  %.pre35 = load <4 x i32>* %arrayidx10.phi.trans.insert, align 16
  br label %for.body5

for.cond43.loopexit:                              ; preds = %for.body5
  %arrayidx53 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i64 0, i64 0
  %.pre = load <4 x i32>* %arrayidx53, align 16
  %arrayidx58.phi.trans.insert = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i64 0, i64 1
  %.pre27 = load <4 x i32>* %arrayidx58.phi.trans.insert, align 16
  %arrayidx64.phi.trans.insert = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i64 0, i64 2
  %.pre28 = load <4 x i32>* %arrayidx64.phi.trans.insert, align 16
  %arrayidx53.1.phi.trans.insert = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i64 0, i64 3
  %.pre29 = load <4 x i32>* %arrayidx53.1.phi.trans.insert, align 16
  %arrayidx58.1.phi.trans.insert = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i64 0, i64 4
  %.pre30 = load <4 x i32>* %arrayidx58.1.phi.trans.insert, align 16
  %arrayidx64.1.phi.trans.insert = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i64 0, i64 5
  %.pre31 = load <4 x i32>* %arrayidx64.1.phi.trans.insert, align 16
  %arrayidx53.2.phi.trans.insert = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i64 0, i64 6
  %.pre32 = load <4 x i32>* %arrayidx53.2.phi.trans.insert, align 16
  %arrayidx58.2.phi.trans.insert = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i64 0, i64 7
  %.pre33 = load <4 x i32>* %arrayidx58.2.phi.trans.insert, align 16
  %arrayidx64.2.phi.trans.insert = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i64 0, i64 8
  %.pre34 = load <4 x i32>* %arrayidx64.2.phi.trans.insert, align 16
  br label %for.cond48.preheader

for.cond48.preheader:                             ; preds = %for.cond48.preheader, %for.cond43.loopexit
  %iSearch.09 = phi i32 [ 0, %for.cond43.loopexit ], [ %inc82, %for.cond48.preheader ]
  %iYes.08 = phi <4 x i32> [ <i32 128, i32 128, i32 128, i32 128>, %for.cond43.loopexit ], [ %shr, %for.cond48.preheader ]
  %iMin.07 = phi <4 x i32> [ zeroinitializer, %for.cond43.loopexit ], [ %or, %for.cond48.preheader ]
  %iMax.06 = phi <4 x i32> [ <i32 255, i32 255, i32 255, i32 255>, %for.cond43.loopexit ], [ %or79, %for.cond48.preheader ]
  %cmp54 = icmp slt <4 x i32> %iYes.08, %.pre
  %sext = sext <4 x i1> %cmp54 to <4 x i32>
  %cmp59 = icmp slt <4 x i32> %iYes.08, %.pre27
  %sext60 = sext <4 x i1> %cmp59 to <4 x i32>
  %add61 = add <4 x i32> %sext, %sext60
  %cmp65 = icmp slt <4 x i32> %iYes.08, %.pre28
  %sext66 = sext <4 x i1> %cmp65 to <4 x i32>
  %add67 = add <4 x i32> %add61, %sext66
  %cmp54.1 = icmp slt <4 x i32> %iYes.08, %.pre29
  %sext.1 = sext <4 x i1> %cmp54.1 to <4 x i32>
  %add55.1 = add <4 x i32> %add67, %sext.1
  %cmp59.1 = icmp slt <4 x i32> %iYes.08, %.pre30
  %sext60.1 = sext <4 x i1> %cmp59.1 to <4 x i32>
  %add61.1 = add <4 x i32> %add55.1, %sext60.1
  %cmp65.1 = icmp slt <4 x i32> %iYes.08, %.pre31
  %sext66.1 = sext <4 x i1> %cmp65.1 to <4 x i32>
  %add67.1 = add <4 x i32> %add61.1, %sext66.1
  %cmp54.2 = icmp slt <4 x i32> %iYes.08, %.pre32
  %sext.2 = sext <4 x i1> %cmp54.2 to <4 x i32>
  %add55.2 = add <4 x i32> %add67.1, %sext.2
  %cmp59.2 = icmp slt <4 x i32> %iYes.08, %.pre33
  %sext60.2 = sext <4 x i1> %cmp59.2 to <4 x i32>
  %add61.2 = add <4 x i32> %add55.2, %sext60.2
  %cmp65.2 = icmp slt <4 x i32> %iYes.08, %.pre34
  %sext66.2 = sext <4 x i1> %cmp65.2 to <4 x i32>
  %add67.2 = add <4 x i32> %add61.2, %sext66.2
  %sub72 = sub <4 x i32> zeroinitializer, %add67.2
  %cmp73 = icmp sgt <4 x i32> %sub72, <i32 4, i32 4, i32 4, i32 4>
  %sext74 = sext <4 x i1> %cmp73 to <4 x i32>
  %and = and <4 x i32> %iYes.08, %sext74
  %neg = xor <4 x i32> %sext74, <i32 -1, i32 -1, i32 -1, i32 -1>
  %and75 = and <4 x i32> %iMin.07, %neg
  %or = or <4 x i32> %and, %and75
  %and77 = and <4 x i32> %iYes.08, %neg
  %and78 = and <4 x i32> %iMax.06, %sext74
  %or79 = or <4 x i32> %and77, %and78
  %add80 = add <4 x i32> %or79, %or
  %shr = ashr <4 x i32> %add80, <i32 1, i32 1, i32 1, i32 1>
  %inc82 = add nsw i32 %iSearch.09, 1
  %exitcond21 = icmp eq i32 %inc82, 8
  br i1 %exitcond21, label %for.end83, label %for.cond48.preheader

for.end83:                                        ; preds = %for.cond48.preheader
  %30 = extractelement <4 x i32> %shr, i32 0
  %and84 = and i32 %30, 255
  %31 = extractelement <4 x i32> %shr, i32 1
  %shl = shl i32 %31, 8
  %and85 = and i32 %shl, 65280
  %32 = extractelement <4 x i32> %shr, i32 2
  %shl87 = shl i32 %32, 16
  %and88 = and i32 %shl87, 16711680
  %or86 = or i32 %and88, %and84
  %or89 = or i32 %or86, %and85
  %33 = add nsw i64 %indvars.iv22, %0
  %arrayidx94 = getelementptr inbounds i32 addrspace(1)* %pDst, i64 %33
  store i32 %or89, i32 addrspace(1)* %arrayidx94, align 4
  %indvars.iv.next23 = add i64 %indvars.iv22, 1
  %lftr.wideiv25 = trunc i64 %indvars.iv.next23 to i32
  %exitcond26 = icmp eq i32 %lftr.wideiv25, %iImageWidth
  br i1 %exitcond26, label %for.end97, label %for.cond2.preheader

for.end97:                                        ; preds = %for.end83
  ret void
}

declare i64 @get_global_id(i32) nounwind readnone

define void @intel_median_scalar(i8 addrspace(1)* nocapture %pSrc, i32 addrspace(1)* nocapture %pDst, i32 %iImageWidth, i32 %iImageHeight) nounwind {
entry:
  %iResult33 = alloca [2 x i64], align 16
  %iResult33.sub = getelementptr inbounds [2 x i64]* %iResult33, i64 0, i64 0
  %tmpcast = bitcast [2 x i64]* %iResult33 to [4 x i32]*
  %iPixels = alloca [9 x i32], align 16
  %call = call i64 @get_global_id(i32 0) nounwind readnone
  %conv = trunc i64 %call to i32
  %add = add i32 %conv, 2
  %arrayidx85 = getelementptr inbounds [2 x i64]* %iResult33, i64 0, i64 1
  %0 = bitcast i64* %arrayidx85 to i32*
  %mul90 = mul nsw i32 %add, %iImageWidth
  %1 = sext i32 %mul90 to i64
  br label %for.cond2.preheader

for.cond2.preheader:                              ; preds = %for.end81, %entry
  %indvars.iv25 = phi i64 [ 0, %entry ], [ %indvars.iv.next26, %for.end81 ]
  br label %for.cond6.preheader

for.cond6.preheader:                              ; preds = %for.end76, %for.cond2.preheader
  %indvars.iv = phi i64 [ 0, %for.cond2.preheader ], [ %indvars.iv.next, %for.end76 ]
  %2 = add i64 %indvars.iv, 4294967292
  %3 = add i64 %indvars.iv, 4
  %add10 = add i32 %conv, 1
  %mul = mul nsw i32 %add10, %iImageWidth
  %4 = zext i32 %mul to i64
  %5 = add nsw i64 %4, %indvars.iv25
  %6 = trunc i64 %5 to i32
  %sub = shl i32 %6, 2
  %7 = trunc i64 %2 to i32
  %add13 = add i32 %7, %sub
  %idxprom = sext i32 %add13 to i64
  %arrayidx = getelementptr inbounds i8 addrspace(1)* %pSrc, i64 %idxprom
  %8 = load i8 addrspace(1)* %arrayidx, align 1
  %conv14 = zext i8 %8 to i32
  %arrayidx16 = getelementptr inbounds [9 x i32]* %iPixels, i64 0, i64 0
  store i32 %conv14, i32* %arrayidx16, align 16
  %9 = sext i32 %sub to i64
  %10 = add nsw i64 %9, %indvars.iv
  %arrayidx20 = getelementptr inbounds i8 addrspace(1)* %pSrc, i64 %10
  %11 = load i8 addrspace(1)* %arrayidx20, align 1
  %conv21 = zext i8 %11 to i32
  %arrayidx23 = getelementptr inbounds [9 x i32]* %iPixels, i64 0, i64 1
  store i32 %conv21, i32* %arrayidx23, align 4
  %12 = trunc i64 %3 to i32
  %add27 = add i32 %12, %sub
  %idxprom28 = sext i32 %add27 to i64
  %arrayidx29 = getelementptr inbounds i8 addrspace(1)* %pSrc, i64 %idxprom28
  %13 = load i8 addrspace(1)* %arrayidx29, align 1
  %conv30 = zext i8 %13 to i32
  %arrayidx32 = getelementptr inbounds [9 x i32]* %iPixels, i64 0, i64 2
  store i32 %conv30, i32* %arrayidx32, align 8
  %14 = add nsw i64 %1, %indvars.iv25
  %15 = trunc i64 %14 to i32
  %sub.1 = shl i32 %15, 2
  %add13.1 = add i32 %7, %sub.1
  %idxprom.1 = sext i32 %add13.1 to i64
  %arrayidx.1 = getelementptr inbounds i8 addrspace(1)* %pSrc, i64 %idxprom.1
  %16 = load i8 addrspace(1)* %arrayidx.1, align 1
  %conv14.1 = zext i8 %16 to i32
  %arrayidx16.1 = getelementptr inbounds [9 x i32]* %iPixels, i64 0, i64 3
  store i32 %conv14.1, i32* %arrayidx16.1, align 4
  %17 = sext i32 %sub.1 to i64
  %18 = add nsw i64 %17, %indvars.iv
  %arrayidx20.1 = getelementptr inbounds i8 addrspace(1)* %pSrc, i64 %18
  %19 = load i8 addrspace(1)* %arrayidx20.1, align 1
  %conv21.1 = zext i8 %19 to i32
  %arrayidx23.1 = getelementptr inbounds [9 x i32]* %iPixels, i64 0, i64 4
  store i32 %conv21.1, i32* %arrayidx23.1, align 16
  %add27.1 = add i32 %12, %sub.1
  %idxprom28.1 = sext i32 %add27.1 to i64
  %arrayidx29.1 = getelementptr inbounds i8 addrspace(1)* %pSrc, i64 %idxprom28.1
  %20 = load i8 addrspace(1)* %arrayidx29.1, align 1
  %conv30.1 = zext i8 %20 to i32
  %arrayidx32.1 = getelementptr inbounds [9 x i32]* %iPixels, i64 0, i64 5
  store i32 %conv30.1, i32* %arrayidx32.1, align 4
  %add10.2 = add i32 %conv, 3
  %mul.2 = mul nsw i32 %add10.2, %iImageWidth
  %21 = zext i32 %mul.2 to i64
  %22 = add nsw i64 %21, %indvars.iv25
  %23 = trunc i64 %22 to i32
  %sub.2 = shl i32 %23, 2
  %add13.2 = add i32 %7, %sub.2
  %idxprom.2 = sext i32 %add13.2 to i64
  %arrayidx.2 = getelementptr inbounds i8 addrspace(1)* %pSrc, i64 %idxprom.2
  %24 = load i8 addrspace(1)* %arrayidx.2, align 1
  %conv14.2 = zext i8 %24 to i32
  %arrayidx16.2 = getelementptr inbounds [9 x i32]* %iPixels, i64 0, i64 6
  store i32 %conv14.2, i32* %arrayidx16.2, align 8
  %25 = sext i32 %sub.2 to i64
  %26 = add nsw i64 %25, %indvars.iv
  %arrayidx20.2 = getelementptr inbounds i8 addrspace(1)* %pSrc, i64 %26
  %27 = load i8 addrspace(1)* %arrayidx20.2, align 1
  %conv21.2 = zext i8 %27 to i32
  %arrayidx23.2 = getelementptr inbounds [9 x i32]* %iPixels, i64 0, i64 7
  store i32 %conv21.2, i32* %arrayidx23.2, align 4
  %add27.2 = add i32 %12, %sub.2
  %idxprom28.2 = sext i32 %add27.2 to i64
  %arrayidx29.2 = getelementptr inbounds i8 addrspace(1)* %pSrc, i64 %idxprom28.2
  %28 = load i8 addrspace(1)* %arrayidx29.2, align 1
  %conv30.2 = zext i8 %28 to i32
  %arrayidx32.2 = getelementptr inbounds [9 x i32]* %iPixels, i64 0, i64 8
  store i32 %conv30.2, i32* %arrayidx32.2, align 16
  br label %for.cond40.preheader

for.cond40.preheader:                             ; preds = %for.cond40.preheader, %for.cond6.preheader
  %iSearch.09 = phi i32 [ 0, %for.cond6.preheader ], [ %inc75, %for.cond40.preheader ]
  %iMax.08 = phi i32 [ 255, %for.cond6.preheader ], [ %cond72, %for.cond40.preheader ]
  %iMin.07 = phi i32 [ 0, %for.cond6.preheader ], [ %cond, %for.cond40.preheader ]
  %iYes.06 = phi i32 [ 128, %for.cond6.preheader ], [ %shr, %for.cond40.preheader ]
  %cmp46 = icmp slt i32 %iYes.06, %conv14
  %conv47 = zext i1 %cmp46 to i32
  %cmp52 = icmp slt i32 %iYes.06, %conv21
  %conv53 = zext i1 %cmp52 to i32
  %cmp58 = icmp slt i32 %iYes.06, %conv30
  %conv59 = zext i1 %cmp58 to i32
  %add54 = add i32 %conv47, %conv53
  %add60 = add i32 %add54, %conv59
  %cmp46.1 = icmp slt i32 %iYes.06, %conv14.1
  %conv47.1 = zext i1 %cmp46.1 to i32
  %cmp52.1 = icmp slt i32 %iYes.06, %conv21.1
  %conv53.1 = zext i1 %cmp52.1 to i32
  %cmp58.1 = icmp slt i32 %iYes.06, %conv30.1
  %conv59.1 = zext i1 %cmp58.1 to i32
  %add48.1 = add i32 %conv47.1, %add60
  %add54.1 = add i32 %add48.1, %conv53.1
  %add60.1 = add i32 %add54.1, %conv59.1
  %cmp46.2 = icmp slt i32 %iYes.06, %conv14.2
  %conv47.2 = zext i1 %cmp46.2 to i32
  %cmp52.2 = icmp slt i32 %iYes.06, %conv21.2
  %conv53.2 = zext i1 %cmp52.2 to i32
  %cmp58.2 = icmp slt i32 %iYes.06, %conv30.2
  %conv59.2 = zext i1 %cmp58.2 to i32
  %add48.2 = add i32 %conv47.2, %add60.1
  %add54.2 = add i32 %add48.2, %conv53.2
  %add60.2 = add i32 %add54.2, %conv59.2
  %cmp65 = icmp sgt i32 %add60.2, 4
  %cond = select i1 %cmp65, i32 %iYes.06, i32 %iMin.07
  %cmp67 = icmp slt i32 %add60.2, 5
  %cond72 = select i1 %cmp67, i32 %iYes.06, i32 %iMax.08
  %add73 = add nsw i32 %cond72, %cond
  %shr = ashr i32 %add73, 1
  %inc75 = add nsw i32 %iSearch.09, 1
  %exitcond = icmp eq i32 %inc75, 8
  br i1 %exitcond, label %for.end76, label %for.cond40.preheader

for.end76:                                        ; preds = %for.cond40.preheader
  %arrayidx78 = getelementptr inbounds [4 x i32]* %tmpcast, i64 0, i64 %indvars.iv
  store i32 %shr, i32* %arrayidx78, align 4
  %indvars.iv.next = add i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond24 = icmp eq i32 %lftr.wideiv, 3
  br i1 %exitcond24, label %for.end81, label %for.cond6.preheader

for.end81:                                        ; preds = %for.end76
  %29 = load i64* %iResult33.sub, align 16
  %30 = trunc i64 %29 to i32
  %and = and i32 %30, 255
  %31 = lshr i64 %29, 24
  %.tr = trunc i64 %31 to i32
  %and84 = and i32 %.tr, 65280
  %32 = load i32* %0, align 8
  %shl86 = shl i32 %32, 16
  %and87 = and i32 %shl86, 16711680
  %or = or i32 %and84, %and
  %or88 = or i32 %or, %and87
  %33 = add nsw i64 %indvars.iv25, %1
  %arrayidx93 = getelementptr inbounds i32 addrspace(1)* %pDst, i64 %33
  store i32 %or88, i32 addrspace(1)* %arrayidx93, align 4
  %indvars.iv.next26 = add i64 %indvars.iv25, 1
  %lftr.wideiv31 = trunc i64 %indvars.iv.next26 to i32
  %exitcond32 = icmp eq i32 %lftr.wideiv31, %iImageWidth
  br i1 %exitcond32, label %for.end96, label %for.cond2.preheader

for.end96:                                        ; preds = %for.end81
  ret void
}

define [7 x i64] @WG.boundaries.intel_median(<4 x i8> addrspace(1)*, i32 addrspace(1)*, i32, i32) {
entry:
  %4 = call i64 @get_local_size(i32 0)
  %5 = call i64 @get_base_global_id.(i32 0)
  %6 = call i64 @get_local_size(i32 1)
  %7 = call i64 @get_base_global_id.(i32 1)
  %8 = call i64 @get_local_size(i32 2)
  %9 = call i64 @get_base_global_id.(i32 2)
  %10 = icmp sgt i32 %2, 0
  %11 = and i1 true, %10
  %zext_cast = zext i1 %11 to i64
  %12 = insertvalue [7 x i64] undef, i64 %4, 2
  %13 = insertvalue [7 x i64] %12, i64 %5, 1
  %14 = insertvalue [7 x i64] %13, i64 %6, 4
  %15 = insertvalue [7 x i64] %14, i64 %7, 3
  %16 = insertvalue [7 x i64] %15, i64 %8, 6
  %17 = insertvalue [7 x i64] %16, i64 %9, 5
  %18 = insertvalue [7 x i64] %17, i64 %zext_cast, 0
  ret [7 x i64] %18
}

declare i64 @get_local_size(i32)

declare i64 @get_base_global_id.(i32)

define [7 x i64] @WG.boundaries.intel_median_scalar(i8 addrspace(1)*, i32 addrspace(1)*, i32, i32) {
entry:
  %4 = call i64 @get_local_size(i32 0)
  %5 = call i64 @get_base_global_id.(i32 0)
  %6 = call i64 @get_local_size(i32 1)
  %7 = call i64 @get_base_global_id.(i32 1)
  %8 = call i64 @get_local_size(i32 2)
  %9 = call i64 @get_base_global_id.(i32 2)
  %10 = icmp sgt i32 %2, 0
  %11 = and i1 true, %10
  %zext_cast = zext i1 %11 to i64
  %12 = insertvalue [7 x i64] undef, i64 %4, 2
  %13 = insertvalue [7 x i64] %12, i64 %5, 1
  %14 = insertvalue [7 x i64] %13, i64 %6, 4
  %15 = insertvalue [7 x i64] %14, i64 %7, 3
  %16 = insertvalue [7 x i64] %15, i64 %8, 6
  %17 = insertvalue [7 x i64] %16, i64 %9, 5
  %18 = insertvalue [7 x i64] %17, i64 %zext_cast, 0
  ret [7 x i64] %18
}

define void @__Vectorized_.intel_median(<4 x i8> addrspace(1)* nocapture %pSrc, i32 addrspace(1)* nocapture %pDst, i32 %iImageWidth, i32 %iImageHeight) nounwind {
entry:
  %temp = insertelement <4 x i32> undef, i32 %iImageWidth, i32 0
  %vector = shufflevector <4 x i32> %temp, <4 x i32> undef, <4 x i32> zeroinitializer
  %0 = alloca [9 x <4 x i32>], align 16
  %1 = alloca [9 x <4 x i32>], align 16
  %2 = alloca [9 x <4 x i32>], align 16
  %3 = alloca [9 x <4 x i32>], align 16
  %call = call i64 @get_global_id(i32 0) nounwind readnone
  %broadcast1 = insertelement <4 x i64> undef, i64 %call, i32 0
  %broadcast2 = shufflevector <4 x i64> %broadcast1, <4 x i64> undef, <4 x i32> zeroinitializer
  %4 = add <4 x i64> %broadcast2, <i64 0, i64 1, i64 2, i64 3>
  %conv235 = trunc <4 x i64> %4 to <4 x i32>
  %add236 = add <4 x i32> %conv235, <i32 2, i32 2, i32 2, i32 2>
  %mul91237 = mul nsw <4 x i32> %add236, %vector
  %5 = sext <4 x i32> %mul91237 to <4 x i64>
  br label %for.cond2.preheader

for.cond2.preheader:                              ; preds = %for.end83, %entry
  %vectorPHI = phi <4 x i32> [ undef, %entry ], [ %shuffleMerge337, %for.end83 ]
  %indvars.iv22 = phi i64 [ 0, %entry ], [ %indvars.iv.next23, %for.end83 ]
  %temp565 = insertelement <4 x i64> undef, i64 %indvars.iv22, i32 0
  %vector566 = shufflevector <4 x i64> %temp565, <4 x i64> undef, <4 x i32> zeroinitializer
  br label %for.body5

for.body5:                                        ; preds = %for.body5.for.body5_crit_edge, %for.cond2.preheader
  %vectorPHI238 = phi <4 x i32> [ %vectorPHI, %for.cond2.preheader ], [ %shuffleMerge325, %for.body5.for.body5_crit_edge ]
  %indvars.iv12 = phi i64 [ 0, %for.cond2.preheader ], [ %indvars.iv.next13, %for.body5.for.body5_crit_edge ]
  %indvars.iv = phi i64 [ -1, %for.cond2.preheader ], [ %indvars.iv.next, %for.body5.for.body5_crit_edge ]
  %6 = trunc i64 %indvars.iv to i32
  %temp239 = insertelement <4 x i32> undef, i32 %6, i32 0
  %vector240 = shufflevector <4 x i32> %temp239, <4 x i32> undef, <4 x i32> zeroinitializer
  %add6241 = add <4 x i32> %add236, %vector240
  %mul242 = mul nsw <4 x i32> %add6241, %vector
  %7 = trunc i64 %indvars.iv22 to i32
  %temp243 = insertelement <4 x i32> undef, i32 %7, i32 0
  %vector244 = shufflevector <4 x i32> %temp243, <4 x i32> undef, <4 x i32> zeroinitializer
  %add7245 = add nsw <4 x i32> %mul242, %vector244
  %sub246 = add nsw <4 x i32> %add7245, <i32 -1, i32 -1, i32 -1, i32 -1>
  %idxprom247 = sext <4 x i32> %sub246 to <4 x i64>
  %extract = extractelement <4 x i64> %idxprom247, i32 0
  %extract248 = extractelement <4 x i64> %idxprom247, i32 1
  %extract249 = extractelement <4 x i64> %idxprom247, i32 2
  %extract250 = extractelement <4 x i64> %idxprom247, i32 3
  %8 = getelementptr inbounds <4 x i8> addrspace(1)* %pSrc, i64 %extract
  %9 = getelementptr inbounds <4 x i8> addrspace(1)* %pSrc, i64 %extract248
  %10 = getelementptr inbounds <4 x i8> addrspace(1)* %pSrc, i64 %extract249
  %11 = getelementptr inbounds <4 x i8> addrspace(1)* %pSrc, i64 %extract250
  %12 = load <4 x i8> addrspace(1)* %8, align 4
  %13 = load <4 x i8> addrspace(1)* %9, align 4
  %14 = load <4 x i8> addrspace(1)* %10, align 4
  %15 = load <4 x i8> addrspace(1)* %11, align 4
  %shuffle0 = shufflevector <4 x i8> %12, <4 x i8> %13, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffle1 = shufflevector <4 x i8> %14, <4 x i8> %15, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffleMerge = shufflevector <4 x i8> %shuffle0, <4 x i8> %shuffle1, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffle0251 = shufflevector <4 x i8> %12, <4 x i8> %13, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffle1252 = shufflevector <4 x i8> %14, <4 x i8> %15, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffleMerge253 = shufflevector <4 x i8> %shuffle0251, <4 x i8> %shuffle1252, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle0254 = shufflevector <4 x i8> %12, <4 x i8> %13, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffle1255 = shufflevector <4 x i8> %14, <4 x i8> %15, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffleMerge256 = shufflevector <4 x i8> %shuffle0254, <4 x i8> %shuffle1255, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %conv8257 = zext <4 x i8> %shuffleMerge to <4 x i32>
  %16 = getelementptr inbounds [9 x <4 x i32>]* %0, i64 0, i64 %indvars.iv12
  %17 = getelementptr inbounds [9 x <4 x i32>]* %1, i64 0, i64 %indvars.iv12
  %18 = getelementptr inbounds [9 x <4 x i32>]* %2, i64 0, i64 %indvars.iv12
  %19 = getelementptr inbounds [9 x <4 x i32>]* %3, i64 0, i64 %indvars.iv12
  %conv11258 = zext <4 x i8> %shuffleMerge253 to <4 x i32>
  %conv14259 = zext <4 x i8> %shuffleMerge256 to <4 x i32>
  %shuf_transpL = shufflevector <4 x i32> %conv8257, <4 x i32> %conv14259, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuf_transpL260 = shufflevector <4 x i32> %conv11258, <4 x i32> %vectorPHI238, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuf_transpH = shufflevector <4 x i32> %conv8257, <4 x i32> %conv14259, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuf_transpH261 = shufflevector <4 x i32> %conv11258, <4 x i32> %vectorPHI238, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuf_transpL262 = shufflevector <4 x i32> %shuf_transpL, <4 x i32> %shuf_transpL260, <4 x i32> <i32 0, i32 4, i32 2, i32 6>
  %shuf_transpH263 = shufflevector <4 x i32> %shuf_transpL, <4 x i32> %shuf_transpL260, <4 x i32> <i32 1, i32 5, i32 3, i32 7>
  %shuf_transpL264 = shufflevector <4 x i32> %shuf_transpH, <4 x i32> %shuf_transpH261, <4 x i32> <i32 0, i32 4, i32 2, i32 6>
  %shuf_transpH265 = shufflevector <4 x i32> %shuf_transpH, <4 x i32> %shuf_transpH261, <4 x i32> <i32 1, i32 5, i32 3, i32 7>
  store <4 x i32> %shuf_transpL262, <4 x i32>* %16, align 16
  store <4 x i32> %shuf_transpH263, <4 x i32>* %17, align 16
  store <4 x i32> %shuf_transpL264, <4 x i32>* %18, align 16
  store <4 x i32> %shuf_transpH265, <4 x i32>* %19, align 16
  %20 = add nsw i64 %indvars.iv12, 1
  %idxprom17266 = sext <4 x i32> %add7245 to <4 x i64>
  %extract267 = extractelement <4 x i64> %idxprom17266, i32 0
  %extract268 = extractelement <4 x i64> %idxprom17266, i32 1
  %extract269 = extractelement <4 x i64> %idxprom17266, i32 2
  %extract270 = extractelement <4 x i64> %idxprom17266, i32 3
  %21 = getelementptr inbounds <4 x i8> addrspace(1)* %pSrc, i64 %extract267
  %22 = getelementptr inbounds <4 x i8> addrspace(1)* %pSrc, i64 %extract268
  %23 = getelementptr inbounds <4 x i8> addrspace(1)* %pSrc, i64 %extract269
  %24 = getelementptr inbounds <4 x i8> addrspace(1)* %pSrc, i64 %extract270
  %25 = load <4 x i8> addrspace(1)* %21, align 4
  %26 = load <4 x i8> addrspace(1)* %22, align 4
  %27 = load <4 x i8> addrspace(1)* %23, align 4
  %28 = load <4 x i8> addrspace(1)* %24, align 4
  %shuffle0271 = shufflevector <4 x i8> %25, <4 x i8> %26, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffle1272 = shufflevector <4 x i8> %27, <4 x i8> %28, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffleMerge273 = shufflevector <4 x i8> %shuffle0271, <4 x i8> %shuffle1272, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffle0274 = shufflevector <4 x i8> %25, <4 x i8> %26, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffle1275 = shufflevector <4 x i8> %27, <4 x i8> %28, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffleMerge276 = shufflevector <4 x i8> %shuffle0274, <4 x i8> %shuffle1275, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle0277 = shufflevector <4 x i8> %25, <4 x i8> %26, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffle1278 = shufflevector <4 x i8> %27, <4 x i8> %28, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffleMerge279 = shufflevector <4 x i8> %shuffle0277, <4 x i8> %shuffle1278, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %conv19280 = zext <4 x i8> %shuffleMerge273 to <4 x i32>
  %29 = getelementptr inbounds [9 x <4 x i32>]* %0, i64 0, i64 %20
  %30 = getelementptr inbounds [9 x <4 x i32>]* %1, i64 0, i64 %20
  %31 = getelementptr inbounds [9 x <4 x i32>]* %2, i64 0, i64 %20
  %32 = getelementptr inbounds [9 x <4 x i32>]* %3, i64 0, i64 %20
  %33 = load <4 x i32>* %29, align 16
  %34 = load <4 x i32>* %30, align 16
  %35 = load <4 x i32>* %31, align 16
  %36 = load <4 x i32>* %32, align 16
  %shuffle0281 = shufflevector <4 x i32> %33, <4 x i32> %34, <4 x i32> <i32 undef, i32 3, i32 undef, i32 7>
  %shuffle1282 = shufflevector <4 x i32> %35, <4 x i32> %36, <4 x i32> <i32 undef, i32 3, i32 undef, i32 7>
  %shuffleMerge283 = shufflevector <4 x i32> %shuffle0281, <4 x i32> %shuffle1282, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %conv22284 = zext <4 x i8> %shuffleMerge276 to <4 x i32>
  %conv25285 = zext <4 x i8> %shuffleMerge279 to <4 x i32>
  %shuf_transpL286 = shufflevector <4 x i32> %conv19280, <4 x i32> %conv25285, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuf_transpL287 = shufflevector <4 x i32> %conv22284, <4 x i32> %shuffleMerge283, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuf_transpH288 = shufflevector <4 x i32> %conv19280, <4 x i32> %conv25285, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuf_transpH289 = shufflevector <4 x i32> %conv22284, <4 x i32> %shuffleMerge283, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuf_transpL290 = shufflevector <4 x i32> %shuf_transpL286, <4 x i32> %shuf_transpL287, <4 x i32> <i32 0, i32 4, i32 2, i32 6>
  %shuf_transpH291 = shufflevector <4 x i32> %shuf_transpL286, <4 x i32> %shuf_transpL287, <4 x i32> <i32 1, i32 5, i32 3, i32 7>
  %shuf_transpL292 = shufflevector <4 x i32> %shuf_transpH288, <4 x i32> %shuf_transpH289, <4 x i32> <i32 0, i32 4, i32 2, i32 6>
  %shuf_transpH293 = shufflevector <4 x i32> %shuf_transpH288, <4 x i32> %shuf_transpH289, <4 x i32> <i32 1, i32 5, i32 3, i32 7>
  store <4 x i32> %shuf_transpL290, <4 x i32>* %29, align 16
  store <4 x i32> %shuf_transpH291, <4 x i32>* %30, align 16
  store <4 x i32> %shuf_transpL292, <4 x i32>* %31, align 16
  store <4 x i32> %shuf_transpH293, <4 x i32>* %32, align 16
  %37 = add nsw i64 %indvars.iv12, 2
  %add29294 = add nsw <4 x i32> %add7245, <i32 1, i32 1, i32 1, i32 1>
  %idxprom30295 = sext <4 x i32> %add29294 to <4 x i64>
  %extract296 = extractelement <4 x i64> %idxprom30295, i32 0
  %extract297 = extractelement <4 x i64> %idxprom30295, i32 1
  %extract298 = extractelement <4 x i64> %idxprom30295, i32 2
  %extract299 = extractelement <4 x i64> %idxprom30295, i32 3
  %38 = getelementptr inbounds <4 x i8> addrspace(1)* %pSrc, i64 %extract296
  %39 = getelementptr inbounds <4 x i8> addrspace(1)* %pSrc, i64 %extract297
  %40 = getelementptr inbounds <4 x i8> addrspace(1)* %pSrc, i64 %extract298
  %41 = getelementptr inbounds <4 x i8> addrspace(1)* %pSrc, i64 %extract299
  %42 = load <4 x i8> addrspace(1)* %38, align 4
  %43 = load <4 x i8> addrspace(1)* %39, align 4
  %44 = load <4 x i8> addrspace(1)* %40, align 4
  %45 = load <4 x i8> addrspace(1)* %41, align 4
  %shuffle0300 = shufflevector <4 x i8> %42, <4 x i8> %43, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffle1301 = shufflevector <4 x i8> %44, <4 x i8> %45, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffleMerge302 = shufflevector <4 x i8> %shuffle0300, <4 x i8> %shuffle1301, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffle0303 = shufflevector <4 x i8> %42, <4 x i8> %43, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffle1304 = shufflevector <4 x i8> %44, <4 x i8> %45, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffleMerge305 = shufflevector <4 x i8> %shuffle0303, <4 x i8> %shuffle1304, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle0306 = shufflevector <4 x i8> %42, <4 x i8> %43, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffle1307 = shufflevector <4 x i8> %44, <4 x i8> %45, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffleMerge308 = shufflevector <4 x i8> %shuffle0306, <4 x i8> %shuffle1307, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %conv32309 = zext <4 x i8> %shuffleMerge302 to <4 x i32>
  %46 = getelementptr inbounds [9 x <4 x i32>]* %0, i64 0, i64 %37
  %47 = getelementptr inbounds [9 x <4 x i32>]* %1, i64 0, i64 %37
  %48 = getelementptr inbounds [9 x <4 x i32>]* %2, i64 0, i64 %37
  %49 = getelementptr inbounds [9 x <4 x i32>]* %3, i64 0, i64 %37
  %50 = load <4 x i32>* %46, align 16
  %51 = load <4 x i32>* %47, align 16
  %52 = load <4 x i32>* %48, align 16
  %53 = load <4 x i32>* %49, align 16
  %shuffle0310 = shufflevector <4 x i32> %50, <4 x i32> %51, <4 x i32> <i32 undef, i32 3, i32 undef, i32 7>
  %shuffle1311 = shufflevector <4 x i32> %52, <4 x i32> %53, <4 x i32> <i32 undef, i32 3, i32 undef, i32 7>
  %shuffleMerge312 = shufflevector <4 x i32> %shuffle0310, <4 x i32> %shuffle1311, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %conv35313 = zext <4 x i8> %shuffleMerge305 to <4 x i32>
  %conv38314 = zext <4 x i8> %shuffleMerge308 to <4 x i32>
  %shuf_transpL315 = shufflevector <4 x i32> %conv32309, <4 x i32> %conv38314, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuf_transpL316 = shufflevector <4 x i32> %conv35313, <4 x i32> %shuffleMerge312, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuf_transpH317 = shufflevector <4 x i32> %conv32309, <4 x i32> %conv38314, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuf_transpH318 = shufflevector <4 x i32> %conv35313, <4 x i32> %shuffleMerge312, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuf_transpL319 = shufflevector <4 x i32> %shuf_transpL315, <4 x i32> %shuf_transpL316, <4 x i32> <i32 0, i32 4, i32 2, i32 6>
  %shuf_transpH320 = shufflevector <4 x i32> %shuf_transpL315, <4 x i32> %shuf_transpL316, <4 x i32> <i32 1, i32 5, i32 3, i32 7>
  %shuf_transpL321 = shufflevector <4 x i32> %shuf_transpH317, <4 x i32> %shuf_transpH318, <4 x i32> <i32 0, i32 4, i32 2, i32 6>
  %shuf_transpH322 = shufflevector <4 x i32> %shuf_transpH317, <4 x i32> %shuf_transpH318, <4 x i32> <i32 1, i32 5, i32 3, i32 7>
  store <4 x i32> %shuf_transpL319, <4 x i32>* %46, align 16
  store <4 x i32> %shuf_transpH320, <4 x i32>* %47, align 16
  store <4 x i32> %shuf_transpL321, <4 x i32>* %48, align 16
  store <4 x i32> %shuf_transpH322, <4 x i32>* %49, align 16
  %indvars.iv.next13 = add i64 %indvars.iv12, 3
  %indvars.iv.next = add i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, 2
  br i1 %exitcond, label %for.cond43.loopexit, label %for.body5.for.body5_crit_edge

for.body5.for.body5_crit_edge:                    ; preds = %for.body5
  %54 = getelementptr inbounds [9 x <4 x i32>]* %0, i64 0, i64 %indvars.iv.next13
  %55 = getelementptr inbounds [9 x <4 x i32>]* %1, i64 0, i64 %indvars.iv.next13
  %56 = getelementptr inbounds [9 x <4 x i32>]* %2, i64 0, i64 %indvars.iv.next13
  %57 = getelementptr inbounds [9 x <4 x i32>]* %3, i64 0, i64 %indvars.iv.next13
  %58 = load <4 x i32>* %54, align 16
  %59 = load <4 x i32>* %55, align 16
  %60 = load <4 x i32>* %56, align 16
  %61 = load <4 x i32>* %57, align 16
  %shuffle0323 = shufflevector <4 x i32> %58, <4 x i32> %59, <4 x i32> <i32 undef, i32 3, i32 undef, i32 7>
  %shuffle1324 = shufflevector <4 x i32> %60, <4 x i32> %61, <4 x i32> <i32 undef, i32 3, i32 undef, i32 7>
  %shuffleMerge325 = shufflevector <4 x i32> %shuffle0323, <4 x i32> %shuffle1324, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  br label %for.body5

for.cond43.loopexit:                              ; preds = %for.body5
  %62 = getelementptr inbounds [9 x <4 x i32>]* %0, i64 0, i64 0
  %63 = getelementptr inbounds [9 x <4 x i32>]* %1, i64 0, i64 0
  %64 = getelementptr inbounds [9 x <4 x i32>]* %2, i64 0, i64 0
  %65 = getelementptr inbounds [9 x <4 x i32>]* %3, i64 0, i64 0
  %66 = load <4 x i32>* %62, align 16
  %67 = load <4 x i32>* %63, align 16
  %68 = load <4 x i32>* %64, align 16
  %69 = load <4 x i32>* %65, align 16
  %shuffle0326 = shufflevector <4 x i32> %66, <4 x i32> %67, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffle1327 = shufflevector <4 x i32> %68, <4 x i32> %69, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffleMerge328 = shufflevector <4 x i32> %shuffle0326, <4 x i32> %shuffle1327, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffle0329 = shufflevector <4 x i32> %66, <4 x i32> %67, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffle1330 = shufflevector <4 x i32> %68, <4 x i32> %69, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffleMerge331 = shufflevector <4 x i32> %shuffle0329, <4 x i32> %shuffle1330, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle0332 = shufflevector <4 x i32> %66, <4 x i32> %67, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffle1333 = shufflevector <4 x i32> %68, <4 x i32> %69, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffleMerge334 = shufflevector <4 x i32> %shuffle0332, <4 x i32> %shuffle1333, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffle0335 = shufflevector <4 x i32> %66, <4 x i32> %67, <4 x i32> <i32 undef, i32 3, i32 undef, i32 7>
  %shuffle1336 = shufflevector <4 x i32> %68, <4 x i32> %69, <4 x i32> <i32 undef, i32 3, i32 undef, i32 7>
  %shuffleMerge337 = shufflevector <4 x i32> %shuffle0335, <4 x i32> %shuffle1336, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %70 = getelementptr inbounds [9 x <4 x i32>]* %0, i64 0, i64 1
  %71 = getelementptr inbounds [9 x <4 x i32>]* %1, i64 0, i64 1
  %72 = getelementptr inbounds [9 x <4 x i32>]* %2, i64 0, i64 1
  %73 = getelementptr inbounds [9 x <4 x i32>]* %3, i64 0, i64 1
  %74 = load <4 x i32>* %70, align 16
  %75 = load <4 x i32>* %71, align 16
  %76 = load <4 x i32>* %72, align 16
  %77 = load <4 x i32>* %73, align 16
  %shuffle0338 = shufflevector <4 x i32> %74, <4 x i32> %75, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffle1339 = shufflevector <4 x i32> %76, <4 x i32> %77, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffleMerge340 = shufflevector <4 x i32> %shuffle0338, <4 x i32> %shuffle1339, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffle0341 = shufflevector <4 x i32> %74, <4 x i32> %75, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffle1342 = shufflevector <4 x i32> %76, <4 x i32> %77, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffleMerge343 = shufflevector <4 x i32> %shuffle0341, <4 x i32> %shuffle1342, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle0344 = shufflevector <4 x i32> %74, <4 x i32> %75, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffle1345 = shufflevector <4 x i32> %76, <4 x i32> %77, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffleMerge346 = shufflevector <4 x i32> %shuffle0344, <4 x i32> %shuffle1345, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %78 = getelementptr inbounds [9 x <4 x i32>]* %0, i64 0, i64 2
  %79 = getelementptr inbounds [9 x <4 x i32>]* %1, i64 0, i64 2
  %80 = getelementptr inbounds [9 x <4 x i32>]* %2, i64 0, i64 2
  %81 = getelementptr inbounds [9 x <4 x i32>]* %3, i64 0, i64 2
  %82 = load <4 x i32>* %78, align 16
  %83 = load <4 x i32>* %79, align 16
  %84 = load <4 x i32>* %80, align 16
  %85 = load <4 x i32>* %81, align 16
  %shuffle0350 = shufflevector <4 x i32> %82, <4 x i32> %83, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffle1351 = shufflevector <4 x i32> %84, <4 x i32> %85, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffleMerge352 = shufflevector <4 x i32> %shuffle0350, <4 x i32> %shuffle1351, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffle0353 = shufflevector <4 x i32> %82, <4 x i32> %83, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffle1354 = shufflevector <4 x i32> %84, <4 x i32> %85, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffleMerge355 = shufflevector <4 x i32> %shuffle0353, <4 x i32> %shuffle1354, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle0356 = shufflevector <4 x i32> %82, <4 x i32> %83, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffle1357 = shufflevector <4 x i32> %84, <4 x i32> %85, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffleMerge358 = shufflevector <4 x i32> %shuffle0356, <4 x i32> %shuffle1357, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %86 = getelementptr inbounds [9 x <4 x i32>]* %0, i64 0, i64 3
  %87 = getelementptr inbounds [9 x <4 x i32>]* %1, i64 0, i64 3
  %88 = getelementptr inbounds [9 x <4 x i32>]* %2, i64 0, i64 3
  %89 = getelementptr inbounds [9 x <4 x i32>]* %3, i64 0, i64 3
  %90 = load <4 x i32>* %86, align 16
  %91 = load <4 x i32>* %87, align 16
  %92 = load <4 x i32>* %88, align 16
  %93 = load <4 x i32>* %89, align 16
  %shuffle0362 = shufflevector <4 x i32> %90, <4 x i32> %91, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffle1363 = shufflevector <4 x i32> %92, <4 x i32> %93, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffleMerge364 = shufflevector <4 x i32> %shuffle0362, <4 x i32> %shuffle1363, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffle0365 = shufflevector <4 x i32> %90, <4 x i32> %91, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffle1366 = shufflevector <4 x i32> %92, <4 x i32> %93, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffleMerge367 = shufflevector <4 x i32> %shuffle0365, <4 x i32> %shuffle1366, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle0368 = shufflevector <4 x i32> %90, <4 x i32> %91, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffle1369 = shufflevector <4 x i32> %92, <4 x i32> %93, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffleMerge370 = shufflevector <4 x i32> %shuffle0368, <4 x i32> %shuffle1369, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %94 = getelementptr inbounds [9 x <4 x i32>]* %0, i64 0, i64 4
  %95 = getelementptr inbounds [9 x <4 x i32>]* %1, i64 0, i64 4
  %96 = getelementptr inbounds [9 x <4 x i32>]* %2, i64 0, i64 4
  %97 = getelementptr inbounds [9 x <4 x i32>]* %3, i64 0, i64 4
  %98 = load <4 x i32>* %94, align 16
  %99 = load <4 x i32>* %95, align 16
  %100 = load <4 x i32>* %96, align 16
  %101 = load <4 x i32>* %97, align 16
  %shuffle0374 = shufflevector <4 x i32> %98, <4 x i32> %99, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffle1375 = shufflevector <4 x i32> %100, <4 x i32> %101, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffleMerge376 = shufflevector <4 x i32> %shuffle0374, <4 x i32> %shuffle1375, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffle0377 = shufflevector <4 x i32> %98, <4 x i32> %99, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffle1378 = shufflevector <4 x i32> %100, <4 x i32> %101, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffleMerge379 = shufflevector <4 x i32> %shuffle0377, <4 x i32> %shuffle1378, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle0380 = shufflevector <4 x i32> %98, <4 x i32> %99, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffle1381 = shufflevector <4 x i32> %100, <4 x i32> %101, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffleMerge382 = shufflevector <4 x i32> %shuffle0380, <4 x i32> %shuffle1381, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %102 = getelementptr inbounds [9 x <4 x i32>]* %0, i64 0, i64 5
  %103 = getelementptr inbounds [9 x <4 x i32>]* %1, i64 0, i64 5
  %104 = getelementptr inbounds [9 x <4 x i32>]* %2, i64 0, i64 5
  %105 = getelementptr inbounds [9 x <4 x i32>]* %3, i64 0, i64 5
  %106 = load <4 x i32>* %102, align 16
  %107 = load <4 x i32>* %103, align 16
  %108 = load <4 x i32>* %104, align 16
  %109 = load <4 x i32>* %105, align 16
  %shuffle0386 = shufflevector <4 x i32> %106, <4 x i32> %107, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffle1387 = shufflevector <4 x i32> %108, <4 x i32> %109, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffleMerge388 = shufflevector <4 x i32> %shuffle0386, <4 x i32> %shuffle1387, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffle0389 = shufflevector <4 x i32> %106, <4 x i32> %107, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffle1390 = shufflevector <4 x i32> %108, <4 x i32> %109, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffleMerge391 = shufflevector <4 x i32> %shuffle0389, <4 x i32> %shuffle1390, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle0392 = shufflevector <4 x i32> %106, <4 x i32> %107, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffle1393 = shufflevector <4 x i32> %108, <4 x i32> %109, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffleMerge394 = shufflevector <4 x i32> %shuffle0392, <4 x i32> %shuffle1393, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %110 = getelementptr inbounds [9 x <4 x i32>]* %0, i64 0, i64 6
  %111 = getelementptr inbounds [9 x <4 x i32>]* %1, i64 0, i64 6
  %112 = getelementptr inbounds [9 x <4 x i32>]* %2, i64 0, i64 6
  %113 = getelementptr inbounds [9 x <4 x i32>]* %3, i64 0, i64 6
  %114 = load <4 x i32>* %110, align 16
  %115 = load <4 x i32>* %111, align 16
  %116 = load <4 x i32>* %112, align 16
  %117 = load <4 x i32>* %113, align 16
  %shuffle0398 = shufflevector <4 x i32> %114, <4 x i32> %115, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffle1399 = shufflevector <4 x i32> %116, <4 x i32> %117, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffleMerge400 = shufflevector <4 x i32> %shuffle0398, <4 x i32> %shuffle1399, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffle0401 = shufflevector <4 x i32> %114, <4 x i32> %115, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffle1402 = shufflevector <4 x i32> %116, <4 x i32> %117, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffleMerge403 = shufflevector <4 x i32> %shuffle0401, <4 x i32> %shuffle1402, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle0404 = shufflevector <4 x i32> %114, <4 x i32> %115, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffle1405 = shufflevector <4 x i32> %116, <4 x i32> %117, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffleMerge406 = shufflevector <4 x i32> %shuffle0404, <4 x i32> %shuffle1405, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %118 = getelementptr inbounds [9 x <4 x i32>]* %0, i64 0, i64 7
  %119 = getelementptr inbounds [9 x <4 x i32>]* %1, i64 0, i64 7
  %120 = getelementptr inbounds [9 x <4 x i32>]* %2, i64 0, i64 7
  %121 = getelementptr inbounds [9 x <4 x i32>]* %3, i64 0, i64 7
  %122 = load <4 x i32>* %118, align 16
  %123 = load <4 x i32>* %119, align 16
  %124 = load <4 x i32>* %120, align 16
  %125 = load <4 x i32>* %121, align 16
  %shuffle0410 = shufflevector <4 x i32> %122, <4 x i32> %123, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffle1411 = shufflevector <4 x i32> %124, <4 x i32> %125, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffleMerge412 = shufflevector <4 x i32> %shuffle0410, <4 x i32> %shuffle1411, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffle0413 = shufflevector <4 x i32> %122, <4 x i32> %123, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffle1414 = shufflevector <4 x i32> %124, <4 x i32> %125, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffleMerge415 = shufflevector <4 x i32> %shuffle0413, <4 x i32> %shuffle1414, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle0416 = shufflevector <4 x i32> %122, <4 x i32> %123, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffle1417 = shufflevector <4 x i32> %124, <4 x i32> %125, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffleMerge418 = shufflevector <4 x i32> %shuffle0416, <4 x i32> %shuffle1417, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %126 = getelementptr inbounds [9 x <4 x i32>]* %0, i64 0, i64 8
  %127 = getelementptr inbounds [9 x <4 x i32>]* %1, i64 0, i64 8
  %128 = getelementptr inbounds [9 x <4 x i32>]* %2, i64 0, i64 8
  %129 = getelementptr inbounds [9 x <4 x i32>]* %3, i64 0, i64 8
  %130 = load <4 x i32>* %126, align 16
  %131 = load <4 x i32>* %127, align 16
  %132 = load <4 x i32>* %128, align 16
  %133 = load <4 x i32>* %129, align 16
  %shuffle0422 = shufflevector <4 x i32> %130, <4 x i32> %131, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffle1423 = shufflevector <4 x i32> %132, <4 x i32> %133, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffleMerge424 = shufflevector <4 x i32> %shuffle0422, <4 x i32> %shuffle1423, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffle0425 = shufflevector <4 x i32> %130, <4 x i32> %131, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffle1426 = shufflevector <4 x i32> %132, <4 x i32> %133, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffleMerge427 = shufflevector <4 x i32> %shuffle0425, <4 x i32> %shuffle1426, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle0428 = shufflevector <4 x i32> %130, <4 x i32> %131, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffle1429 = shufflevector <4 x i32> %132, <4 x i32> %133, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffleMerge430 = shufflevector <4 x i32> %shuffle0428, <4 x i32> %shuffle1429, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  br label %for.cond48.preheader

for.cond48.preheader:                             ; preds = %for.cond48.preheader, %for.cond43.loopexit
  %iSearch.09 = phi i32 [ 0, %for.cond43.loopexit ], [ %inc82, %for.cond48.preheader ]
  %vectorPHI434 = phi <4 x i32> [ <i32 128, i32 128, i32 128, i32 128>, %for.cond43.loopexit ], [ %shr216554, %for.cond48.preheader ]
  %vectorPHI435 = phi <4 x i32> [ <i32 128, i32 128, i32 128, i32 128>, %for.cond43.loopexit ], [ %shr217555, %for.cond48.preheader ]
  %vectorPHI436 = phi <4 x i32> [ <i32 128, i32 128, i32 128, i32 128>, %for.cond43.loopexit ], [ %shr218556, %for.cond48.preheader ]
  %vectorPHI438 = phi <4 x i32> [ zeroinitializer, %for.cond43.loopexit ], [ %or196534, %for.cond48.preheader ]
  %vectorPHI439 = phi <4 x i32> [ zeroinitializer, %for.cond43.loopexit ], [ %or197535, %for.cond48.preheader ]
  %vectorPHI440 = phi <4 x i32> [ zeroinitializer, %for.cond43.loopexit ], [ %or198536, %for.cond48.preheader ]
  %vectorPHI442 = phi <4 x i32> [ <i32 255, i32 255, i32 255, i32 255>, %for.cond43.loopexit ], [ %or79208546, %for.cond48.preheader ]
  %vectorPHI443 = phi <4 x i32> [ <i32 255, i32 255, i32 255, i32 255>, %for.cond43.loopexit ], [ %or79209547, %for.cond48.preheader ]
  %vectorPHI444 = phi <4 x i32> [ <i32 255, i32 255, i32 255, i32 255>, %for.cond43.loopexit ], [ %or79210548, %for.cond48.preheader ]
  %cmp5436 = icmp sgt <4 x i32> %shuffleMerge328, %vectorPHI434
  %cmp5437 = icmp sgt <4 x i32> %shuffleMerge331, %vectorPHI435
  %cmp5438 = icmp sgt <4 x i32> %shuffleMerge334, %vectorPHI436
  %sext40446 = sext <4 x i1> %cmp5436 to <4 x i32>
  %sext41447 = sext <4 x i1> %cmp5437 to <4 x i32>
  %sext42448 = sext <4 x i1> %cmp5438 to <4 x i32>
  %cmp5948 = icmp sgt <4 x i32> %shuffleMerge340, %vectorPHI434
  %cmp5949 = icmp sgt <4 x i32> %shuffleMerge343, %vectorPHI435
  %cmp5950 = icmp sgt <4 x i32> %shuffleMerge346, %vectorPHI436
  %sext6052450 = sext <4 x i1> %cmp5948 to <4 x i32>
  %sext6053451 = sext <4 x i1> %cmp5949 to <4 x i32>
  %sext6054452 = sext <4 x i1> %cmp5950 to <4 x i32>
  %add6156454 = add <4 x i32> %sext40446, %sext6052450
  %add6157455 = add <4 x i32> %sext41447, %sext6053451
  %add6158456 = add <4 x i32> %sext42448, %sext6054452
  %cmp6564 = icmp sgt <4 x i32> %shuffleMerge352, %vectorPHI434
  %cmp6565 = icmp sgt <4 x i32> %shuffleMerge355, %vectorPHI435
  %cmp6566 = icmp sgt <4 x i32> %shuffleMerge358, %vectorPHI436
  %sext6668458 = sext <4 x i1> %cmp6564 to <4 x i32>
  %sext6669459 = sext <4 x i1> %cmp6565 to <4 x i32>
  %sext6670460 = sext <4 x i1> %cmp6566 to <4 x i32>
  %add6772462 = add <4 x i32> %add6156454, %sext6668458
  %add6773463 = add <4 x i32> %add6157455, %sext6669459
  %add6774464 = add <4 x i32> %add6158456, %sext6670460
  %cmp54.180 = icmp sgt <4 x i32> %shuffleMerge364, %vectorPHI434
  %cmp54.181 = icmp sgt <4 x i32> %shuffleMerge367, %vectorPHI435
  %cmp54.182 = icmp sgt <4 x i32> %shuffleMerge370, %vectorPHI436
  %sext.184466 = sext <4 x i1> %cmp54.180 to <4 x i32>
  %sext.185467 = sext <4 x i1> %cmp54.181 to <4 x i32>
  %sext.186468 = sext <4 x i1> %cmp54.182 to <4 x i32>
  %add55.188470 = add <4 x i32> %add6772462, %sext.184466
  %add55.189471 = add <4 x i32> %add6773463, %sext.185467
  %add55.190472 = add <4 x i32> %add6774464, %sext.186468
  %cmp59.196 = icmp sgt <4 x i32> %shuffleMerge376, %vectorPHI434
  %cmp59.197 = icmp sgt <4 x i32> %shuffleMerge379, %vectorPHI435
  %cmp59.198 = icmp sgt <4 x i32> %shuffleMerge382, %vectorPHI436
  %sext60.1100474 = sext <4 x i1> %cmp59.196 to <4 x i32>
  %sext60.1101475 = sext <4 x i1> %cmp59.197 to <4 x i32>
  %sext60.1102476 = sext <4 x i1> %cmp59.198 to <4 x i32>
  %add61.1104478 = add <4 x i32> %add55.188470, %sext60.1100474
  %add61.1105479 = add <4 x i32> %add55.189471, %sext60.1101475
  %add61.1106480 = add <4 x i32> %add55.190472, %sext60.1102476
  %cmp65.1112 = icmp sgt <4 x i32> %shuffleMerge388, %vectorPHI434
  %cmp65.1113 = icmp sgt <4 x i32> %shuffleMerge391, %vectorPHI435
  %cmp65.1114 = icmp sgt <4 x i32> %shuffleMerge394, %vectorPHI436
  %sext66.1116482 = sext <4 x i1> %cmp65.1112 to <4 x i32>
  %sext66.1117483 = sext <4 x i1> %cmp65.1113 to <4 x i32>
  %sext66.1118484 = sext <4 x i1> %cmp65.1114 to <4 x i32>
  %add67.1120486 = add <4 x i32> %add61.1104478, %sext66.1116482
  %add67.1121487 = add <4 x i32> %add61.1105479, %sext66.1117483
  %add67.1122488 = add <4 x i32> %add61.1106480, %sext66.1118484
  %cmp54.2128 = icmp sgt <4 x i32> %shuffleMerge400, %vectorPHI434
  %cmp54.2129 = icmp sgt <4 x i32> %shuffleMerge403, %vectorPHI435
  %cmp54.2130 = icmp sgt <4 x i32> %shuffleMerge406, %vectorPHI436
  %sext.2132490 = sext <4 x i1> %cmp54.2128 to <4 x i32>
  %sext.2133491 = sext <4 x i1> %cmp54.2129 to <4 x i32>
  %sext.2134492 = sext <4 x i1> %cmp54.2130 to <4 x i32>
  %add55.2136494 = add <4 x i32> %add67.1120486, %sext.2132490
  %add55.2137495 = add <4 x i32> %add67.1121487, %sext.2133491
  %add55.2138496 = add <4 x i32> %add67.1122488, %sext.2134492
  %cmp59.2144 = icmp sgt <4 x i32> %shuffleMerge412, %vectorPHI434
  %cmp59.2145 = icmp sgt <4 x i32> %shuffleMerge415, %vectorPHI435
  %cmp59.2146 = icmp sgt <4 x i32> %shuffleMerge418, %vectorPHI436
  %sext60.2148498 = sext <4 x i1> %cmp59.2144 to <4 x i32>
  %sext60.2149499 = sext <4 x i1> %cmp59.2145 to <4 x i32>
  %sext60.2150500 = sext <4 x i1> %cmp59.2146 to <4 x i32>
  %add61.2152502 = add <4 x i32> %add55.2136494, %sext60.2148498
  %add61.2153503 = add <4 x i32> %add55.2137495, %sext60.2149499
  %add61.2154504 = add <4 x i32> %add55.2138496, %sext60.2150500
  %cmp65.2160 = icmp sgt <4 x i32> %shuffleMerge424, %vectorPHI434
  %cmp65.2161 = icmp sgt <4 x i32> %shuffleMerge427, %vectorPHI435
  %cmp65.2162 = icmp sgt <4 x i32> %shuffleMerge430, %vectorPHI436
  %sext66.2164506 = sext <4 x i1> %cmp65.2160 to <4 x i32>
  %sext66.2165507 = sext <4 x i1> %cmp65.2161 to <4 x i32>
  %sext66.2166508 = sext <4 x i1> %cmp65.2162 to <4 x i32>
  %add67.2168510 = add <4 x i32> %add61.2152502, %sext66.2164506
  %add67.2169511 = add <4 x i32> %add61.2153503, %sext66.2165507
  %add67.2170512 = add <4 x i32> %add61.2154504, %sext66.2166508
  %sub72172514 = sub <4 x i32> zeroinitializer, %add67.2168510
  %sub72173515 = sub <4 x i32> zeroinitializer, %add67.2169511
  %sub72174516 = sub <4 x i32> zeroinitializer, %add67.2170512
  %cmp73176 = icmp sgt <4 x i32> %sub72172514, <i32 4, i32 4, i32 4, i32 4>
  %cmp73177 = icmp sgt <4 x i32> %sub72173515, <i32 4, i32 4, i32 4, i32 4>
  %cmp73178 = icmp sgt <4 x i32> %sub72174516, <i32 4, i32 4, i32 4, i32 4>
  %sext74180518 = sext <4 x i1> %cmp73176 to <4 x i32>
  %sext74181519 = sext <4 x i1> %cmp73177 to <4 x i32>
  %sext74182520 = sext <4 x i1> %cmp73178 to <4 x i32>
  %and184522 = and <4 x i32> %vectorPHI434, %sext74180518
  %and185523 = and <4 x i32> %vectorPHI435, %sext74181519
  %and186524 = and <4 x i32> %vectorPHI436, %sext74182520
  %neg188526 = xor <4 x i32> %sext74180518, <i32 -1, i32 -1, i32 -1, i32 -1>
  %neg189527 = xor <4 x i32> %sext74181519, <i32 -1, i32 -1, i32 -1, i32 -1>
  %neg190528 = xor <4 x i32> %sext74182520, <i32 -1, i32 -1, i32 -1, i32 -1>
  %and75192530 = and <4 x i32> %vectorPHI438, %neg188526
  %and75193531 = and <4 x i32> %vectorPHI439, %neg189527
  %and75194532 = and <4 x i32> %vectorPHI440, %neg190528
  %or196534 = or <4 x i32> %and184522, %and75192530
  %or197535 = or <4 x i32> %and185523, %and75193531
  %or198536 = or <4 x i32> %and186524, %and75194532
  %and77200538 = and <4 x i32> %vectorPHI434, %neg188526
  %and77201539 = and <4 x i32> %vectorPHI435, %neg189527
  %and77202540 = and <4 x i32> %vectorPHI436, %neg190528
  %and78204542 = and <4 x i32> %vectorPHI442, %sext74180518
  %and78205543 = and <4 x i32> %vectorPHI443, %sext74181519
  %and78206544 = and <4 x i32> %vectorPHI444, %sext74182520
  %or79208546 = or <4 x i32> %and77200538, %and78204542
  %or79209547 = or <4 x i32> %and77201539, %and78205543
  %or79210548 = or <4 x i32> %and77202540, %and78206544
  %add80212550 = add <4 x i32> %or79208546, %or196534
  %add80213551 = add <4 x i32> %or79209547, %or197535
  %add80214552 = add <4 x i32> %or79210548, %or198536
  %shr216554 = ashr <4 x i32> %add80212550, <i32 1, i32 1, i32 1, i32 1>
  %shr217555 = ashr <4 x i32> %add80213551, <i32 1, i32 1, i32 1, i32 1>
  %shr218556 = ashr <4 x i32> %add80214552, <i32 1, i32 1, i32 1, i32 1>
  %inc82 = add nsw i32 %iSearch.09, 1
  %exitcond21 = icmp eq i32 %inc82, 8
  br i1 %exitcond21, label %for.end83, label %for.cond48.preheader

for.end83:                                        ; preds = %for.cond48.preheader
  %and84558 = and <4 x i32> %shr216554, <i32 255, i32 255, i32 255, i32 255>
  %shl559 = shl <4 x i32> %shr217555, <i32 8, i32 8, i32 8, i32 8>
  %and85560 = and <4 x i32> %shl559, <i32 65280, i32 65280, i32 65280, i32 65280>
  %shl87561 = shl <4 x i32> %shr218556, <i32 16, i32 16, i32 16, i32 16>
  %and88562 = and <4 x i32> %shl87561, <i32 16711680, i32 16711680, i32 16711680, i32 16711680>
  %or86563 = or <4 x i32> %and88562, %and84558
  %or89564 = or <4 x i32> %or86563, %and85560
  %extract571 = extractelement <4 x i32> %or89564, i32 0
  %extract572 = extractelement <4 x i32> %or89564, i32 1
  %extract573 = extractelement <4 x i32> %or89564, i32 2
  %extract574 = extractelement <4 x i32> %or89564, i32 3
  %134 = add nsw <4 x i64> %vector566, %5
  %extract567 = extractelement <4 x i64> %134, i32 0
  %extract568 = extractelement <4 x i64> %134, i32 1
  %extract569 = extractelement <4 x i64> %134, i32 2
  %extract570 = extractelement <4 x i64> %134, i32 3
  %135 = getelementptr inbounds i32 addrspace(1)* %pDst, i64 %extract567
  %136 = getelementptr inbounds i32 addrspace(1)* %pDst, i64 %extract568
  %137 = getelementptr inbounds i32 addrspace(1)* %pDst, i64 %extract569
  %138 = getelementptr inbounds i32 addrspace(1)* %pDst, i64 %extract570
  store i32 %extract571, i32 addrspace(1)* %135, align 4
  store i32 %extract572, i32 addrspace(1)* %136, align 4
  store i32 %extract573, i32 addrspace(1)* %137, align 4
  store i32 %extract574, i32 addrspace(1)* %138, align 4
  %indvars.iv.next23 = add i64 %indvars.iv22, 1
  %lftr.wideiv25 = trunc i64 %indvars.iv.next23 to i32
  %exitcond26 = icmp eq i32 %lftr.wideiv25, %iImageWidth
  br i1 %exitcond26, label %for.end97, label %for.cond2.preheader

for.end97:                                        ; preds = %for.end83
  ret void
}

define void @__Vectorized_.intel_median_scalar(i8 addrspace(1)* nocapture %pSrc, i32 addrspace(1)* nocapture %pDst, i32 %iImageWidth, i32 %iImageHeight) nounwind {
entry:
  %temp = insertelement <4 x i32> undef, i32 %iImageWidth, i32 0
  %vector = shufflevector <4 x i32> %temp, <4 x i32> undef, <4 x i32> zeroinitializer
  %0 = alloca [2 x i64], align 16
  %1 = alloca [2 x i64], align 16
  %2 = alloca [2 x i64], align 16
  %3 = alloca [2 x i64], align 16
  %4 = getelementptr inbounds [2 x i64]* %0, i64 0, i64 0
  %5 = getelementptr inbounds [2 x i64]* %1, i64 0, i64 0
  %6 = getelementptr inbounds [2 x i64]* %2, i64 0, i64 0
  %7 = getelementptr inbounds [2 x i64]* %3, i64 0, i64 0
  %8 = bitcast [2 x i64]* %0 to [4 x i32]*
  %9 = bitcast [2 x i64]* %1 to [4 x i32]*
  %10 = bitcast [2 x i64]* %2 to [4 x i32]*
  %11 = bitcast [2 x i64]* %3 to [4 x i32]*
  %call = call i64 @get_global_id(i32 0) nounwind readnone
  %broadcast1 = insertelement <4 x i64> undef, i64 %call, i32 0
  %broadcast2 = shufflevector <4 x i64> %broadcast1, <4 x i64> undef, <4 x i32> zeroinitializer
  %12 = add <4 x i64> %broadcast2, <i64 0, i64 1, i64 2, i64 3>
  %conv25 = trunc <4 x i64> %12 to <4 x i32>
  %add26 = add <4 x i32> %conv25, <i32 2, i32 2, i32 2, i32 2>
  %13 = getelementptr inbounds [2 x i64]* %0, i64 0, i64 1
  %14 = getelementptr inbounds [2 x i64]* %1, i64 0, i64 1
  %15 = getelementptr inbounds [2 x i64]* %2, i64 0, i64 1
  %16 = getelementptr inbounds [2 x i64]* %3, i64 0, i64 1
  %17 = bitcast i64* %13 to i32*
  %18 = bitcast i64* %14 to i32*
  %19 = bitcast i64* %15 to i32*
  %20 = bitcast i64* %16 to i32*
  %mul9027 = mul nsw <4 x i32> %add26, %vector
  %21 = sext <4 x i32> %mul9027 to <4 x i64>
  br label %for.cond2.preheader

for.cond2.preheader:                              ; preds = %for.end81, %entry
  %indvars.iv25 = phi i64 [ 0, %entry ], [ %indvars.iv.next26, %for.end81 ]
  %temp30 = insertelement <4 x i64> undef, i64 %indvars.iv25, i32 0
  %vector31 = shufflevector <4 x i64> %temp30, <4 x i64> undef, <4 x i32> zeroinitializer
  br label %for.cond6.preheader

for.cond6.preheader:                              ; preds = %for.end76, %for.cond2.preheader
  %indvars.iv = phi i64 [ 0, %for.cond2.preheader ], [ %indvars.iv.next, %for.end76 ]
  %temp44 = insertelement <4 x i64> undef, i64 %indvars.iv, i32 0
  %vector45 = shufflevector <4 x i64> %temp44, <4 x i64> undef, <4 x i32> zeroinitializer
  %22 = add i64 %indvars.iv, 4294967292
  %23 = add i64 %indvars.iv, 4
  %add1028 = add <4 x i32> %conv25, <i32 1, i32 1, i32 1, i32 1>
  %mul29 = mul nsw <4 x i32> %add1028, %vector
  %24 = zext <4 x i32> %mul29 to <4 x i64>
  %25 = add nsw <4 x i64> %24, %vector31
  %26 = trunc <4 x i64> %25 to <4 x i32>
  %sub32 = shl <4 x i32> %26, <i32 2, i32 2, i32 2, i32 2>
  %27 = trunc i64 %22 to i32
  %temp33 = insertelement <4 x i32> undef, i32 %27, i32 0
  %vector34 = shufflevector <4 x i32> %temp33, <4 x i32> undef, <4 x i32> zeroinitializer
  %add1335 = add <4 x i32> %vector34, %sub32
  %idxprom36 = sext <4 x i32> %add1335 to <4 x i64>
  %extract = extractelement <4 x i64> %idxprom36, i32 0
  %extract37 = extractelement <4 x i64> %idxprom36, i32 1
  %extract38 = extractelement <4 x i64> %idxprom36, i32 2
  %extract39 = extractelement <4 x i64> %idxprom36, i32 3
  %28 = getelementptr inbounds i8 addrspace(1)* %pSrc, i64 %extract
  %29 = getelementptr inbounds i8 addrspace(1)* %pSrc, i64 %extract37
  %30 = getelementptr inbounds i8 addrspace(1)* %pSrc, i64 %extract38
  %31 = getelementptr inbounds i8 addrspace(1)* %pSrc, i64 %extract39
  %32 = load i8 addrspace(1)* %28, align 1
  %33 = load i8 addrspace(1)* %29, align 1
  %34 = load i8 addrspace(1)* %30, align 1
  %35 = load i8 addrspace(1)* %31, align 1
  %temp.vect = insertelement <4 x i8> undef, i8 %32, i32 0
  %temp.vect40 = insertelement <4 x i8> %temp.vect, i8 %33, i32 1
  %temp.vect41 = insertelement <4 x i8> %temp.vect40, i8 %34, i32 2
  %temp.vect42 = insertelement <4 x i8> %temp.vect41, i8 %35, i32 3
  %conv1443 = zext <4 x i8> %temp.vect42 to <4 x i32>
  %36 = sext <4 x i32> %sub32 to <4 x i64>
  %37 = add nsw <4 x i64> %36, %vector45
  %extract46 = extractelement <4 x i64> %37, i32 0
  %extract47 = extractelement <4 x i64> %37, i32 1
  %extract48 = extractelement <4 x i64> %37, i32 2
  %extract49 = extractelement <4 x i64> %37, i32 3
  %38 = getelementptr inbounds i8 addrspace(1)* %pSrc, i64 %extract46
  %39 = getelementptr inbounds i8 addrspace(1)* %pSrc, i64 %extract47
  %40 = getelementptr inbounds i8 addrspace(1)* %pSrc, i64 %extract48
  %41 = getelementptr inbounds i8 addrspace(1)* %pSrc, i64 %extract49
  %42 = load i8 addrspace(1)* %38, align 1
  %43 = load i8 addrspace(1)* %39, align 1
  %44 = load i8 addrspace(1)* %40, align 1
  %45 = load i8 addrspace(1)* %41, align 1
  %temp.vect50 = insertelement <4 x i8> undef, i8 %42, i32 0
  %temp.vect51 = insertelement <4 x i8> %temp.vect50, i8 %43, i32 1
  %temp.vect52 = insertelement <4 x i8> %temp.vect51, i8 %44, i32 2
  %temp.vect53 = insertelement <4 x i8> %temp.vect52, i8 %45, i32 3
  %conv2154 = zext <4 x i8> %temp.vect53 to <4 x i32>
  %46 = trunc i64 %23 to i32
  %temp55 = insertelement <4 x i32> undef, i32 %46, i32 0
  %vector56 = shufflevector <4 x i32> %temp55, <4 x i32> undef, <4 x i32> zeroinitializer
  %add2757 = add <4 x i32> %vector56, %sub32
  %idxprom2858 = sext <4 x i32> %add2757 to <4 x i64>
  %extract59 = extractelement <4 x i64> %idxprom2858, i32 0
  %extract60 = extractelement <4 x i64> %idxprom2858, i32 1
  %extract61 = extractelement <4 x i64> %idxprom2858, i32 2
  %extract62 = extractelement <4 x i64> %idxprom2858, i32 3
  %47 = getelementptr inbounds i8 addrspace(1)* %pSrc, i64 %extract59
  %48 = getelementptr inbounds i8 addrspace(1)* %pSrc, i64 %extract60
  %49 = getelementptr inbounds i8 addrspace(1)* %pSrc, i64 %extract61
  %50 = getelementptr inbounds i8 addrspace(1)* %pSrc, i64 %extract62
  %51 = load i8 addrspace(1)* %47, align 1
  %52 = load i8 addrspace(1)* %48, align 1
  %53 = load i8 addrspace(1)* %49, align 1
  %54 = load i8 addrspace(1)* %50, align 1
  %temp.vect63 = insertelement <4 x i8> undef, i8 %51, i32 0
  %temp.vect64 = insertelement <4 x i8> %temp.vect63, i8 %52, i32 1
  %temp.vect65 = insertelement <4 x i8> %temp.vect64, i8 %53, i32 2
  %temp.vect66 = insertelement <4 x i8> %temp.vect65, i8 %54, i32 3
  %conv3067 = zext <4 x i8> %temp.vect66 to <4 x i32>
  %55 = add nsw <4 x i64> %21, %vector31
  %56 = trunc <4 x i64> %55 to <4 x i32>
  %sub.168 = shl <4 x i32> %56, <i32 2, i32 2, i32 2, i32 2>
  %add13.169 = add <4 x i32> %vector34, %sub.168
  %idxprom.170 = sext <4 x i32> %add13.169 to <4 x i64>
  %extract71 = extractelement <4 x i64> %idxprom.170, i32 0
  %extract72 = extractelement <4 x i64> %idxprom.170, i32 1
  %extract73 = extractelement <4 x i64> %idxprom.170, i32 2
  %extract74 = extractelement <4 x i64> %idxprom.170, i32 3
  %57 = getelementptr inbounds i8 addrspace(1)* %pSrc, i64 %extract71
  %58 = getelementptr inbounds i8 addrspace(1)* %pSrc, i64 %extract72
  %59 = getelementptr inbounds i8 addrspace(1)* %pSrc, i64 %extract73
  %60 = getelementptr inbounds i8 addrspace(1)* %pSrc, i64 %extract74
  %61 = load i8 addrspace(1)* %57, align 1
  %62 = load i8 addrspace(1)* %58, align 1
  %63 = load i8 addrspace(1)* %59, align 1
  %64 = load i8 addrspace(1)* %60, align 1
  %temp.vect75 = insertelement <4 x i8> undef, i8 %61, i32 0
  %temp.vect76 = insertelement <4 x i8> %temp.vect75, i8 %62, i32 1
  %temp.vect77 = insertelement <4 x i8> %temp.vect76, i8 %63, i32 2
  %temp.vect78 = insertelement <4 x i8> %temp.vect77, i8 %64, i32 3
  %conv14.179 = zext <4 x i8> %temp.vect78 to <4 x i32>
  %65 = sext <4 x i32> %sub.168 to <4 x i64>
  %66 = add nsw <4 x i64> %65, %vector45
  %extract80 = extractelement <4 x i64> %66, i32 0
  %extract81 = extractelement <4 x i64> %66, i32 1
  %extract82 = extractelement <4 x i64> %66, i32 2
  %extract83 = extractelement <4 x i64> %66, i32 3
  %67 = getelementptr inbounds i8 addrspace(1)* %pSrc, i64 %extract80
  %68 = getelementptr inbounds i8 addrspace(1)* %pSrc, i64 %extract81
  %69 = getelementptr inbounds i8 addrspace(1)* %pSrc, i64 %extract82
  %70 = getelementptr inbounds i8 addrspace(1)* %pSrc, i64 %extract83
  %71 = load i8 addrspace(1)* %67, align 1
  %72 = load i8 addrspace(1)* %68, align 1
  %73 = load i8 addrspace(1)* %69, align 1
  %74 = load i8 addrspace(1)* %70, align 1
  %temp.vect84 = insertelement <4 x i8> undef, i8 %71, i32 0
  %temp.vect85 = insertelement <4 x i8> %temp.vect84, i8 %72, i32 1
  %temp.vect86 = insertelement <4 x i8> %temp.vect85, i8 %73, i32 2
  %temp.vect87 = insertelement <4 x i8> %temp.vect86, i8 %74, i32 3
  %conv21.188 = zext <4 x i8> %temp.vect87 to <4 x i32>
  %add27.189 = add <4 x i32> %vector56, %sub.168
  %idxprom28.190 = sext <4 x i32> %add27.189 to <4 x i64>
  %extract91 = extractelement <4 x i64> %idxprom28.190, i32 0
  %extract92 = extractelement <4 x i64> %idxprom28.190, i32 1
  %extract93 = extractelement <4 x i64> %idxprom28.190, i32 2
  %extract94 = extractelement <4 x i64> %idxprom28.190, i32 3
  %75 = getelementptr inbounds i8 addrspace(1)* %pSrc, i64 %extract91
  %76 = getelementptr inbounds i8 addrspace(1)* %pSrc, i64 %extract92
  %77 = getelementptr inbounds i8 addrspace(1)* %pSrc, i64 %extract93
  %78 = getelementptr inbounds i8 addrspace(1)* %pSrc, i64 %extract94
  %79 = load i8 addrspace(1)* %75, align 1
  %80 = load i8 addrspace(1)* %76, align 1
  %81 = load i8 addrspace(1)* %77, align 1
  %82 = load i8 addrspace(1)* %78, align 1
  %temp.vect95 = insertelement <4 x i8> undef, i8 %79, i32 0
  %temp.vect96 = insertelement <4 x i8> %temp.vect95, i8 %80, i32 1
  %temp.vect97 = insertelement <4 x i8> %temp.vect96, i8 %81, i32 2
  %temp.vect98 = insertelement <4 x i8> %temp.vect97, i8 %82, i32 3
  %conv30.199 = zext <4 x i8> %temp.vect98 to <4 x i32>
  %add10.2100 = add <4 x i32> %conv25, <i32 3, i32 3, i32 3, i32 3>
  %mul.2101 = mul nsw <4 x i32> %add10.2100, %vector
  %83 = zext <4 x i32> %mul.2101 to <4 x i64>
  %84 = add nsw <4 x i64> %83, %vector31
  %85 = trunc <4 x i64> %84 to <4 x i32>
  %sub.2102 = shl <4 x i32> %85, <i32 2, i32 2, i32 2, i32 2>
  %add13.2103 = add <4 x i32> %vector34, %sub.2102
  %idxprom.2104 = sext <4 x i32> %add13.2103 to <4 x i64>
  %extract105 = extractelement <4 x i64> %idxprom.2104, i32 0
  %extract106 = extractelement <4 x i64> %idxprom.2104, i32 1
  %extract107 = extractelement <4 x i64> %idxprom.2104, i32 2
  %extract108 = extractelement <4 x i64> %idxprom.2104, i32 3
  %86 = getelementptr inbounds i8 addrspace(1)* %pSrc, i64 %extract105
  %87 = getelementptr inbounds i8 addrspace(1)* %pSrc, i64 %extract106
  %88 = getelementptr inbounds i8 addrspace(1)* %pSrc, i64 %extract107
  %89 = getelementptr inbounds i8 addrspace(1)* %pSrc, i64 %extract108
  %90 = load i8 addrspace(1)* %86, align 1
  %91 = load i8 addrspace(1)* %87, align 1
  %92 = load i8 addrspace(1)* %88, align 1
  %93 = load i8 addrspace(1)* %89, align 1
  %temp.vect109 = insertelement <4 x i8> undef, i8 %90, i32 0
  %temp.vect110 = insertelement <4 x i8> %temp.vect109, i8 %91, i32 1
  %temp.vect111 = insertelement <4 x i8> %temp.vect110, i8 %92, i32 2
  %temp.vect112 = insertelement <4 x i8> %temp.vect111, i8 %93, i32 3
  %conv14.2113 = zext <4 x i8> %temp.vect112 to <4 x i32>
  %94 = sext <4 x i32> %sub.2102 to <4 x i64>
  %95 = add nsw <4 x i64> %94, %vector45
  %extract114 = extractelement <4 x i64> %95, i32 0
  %extract115 = extractelement <4 x i64> %95, i32 1
  %extract116 = extractelement <4 x i64> %95, i32 2
  %extract117 = extractelement <4 x i64> %95, i32 3
  %96 = getelementptr inbounds i8 addrspace(1)* %pSrc, i64 %extract114
  %97 = getelementptr inbounds i8 addrspace(1)* %pSrc, i64 %extract115
  %98 = getelementptr inbounds i8 addrspace(1)* %pSrc, i64 %extract116
  %99 = getelementptr inbounds i8 addrspace(1)* %pSrc, i64 %extract117
  %100 = load i8 addrspace(1)* %96, align 1
  %101 = load i8 addrspace(1)* %97, align 1
  %102 = load i8 addrspace(1)* %98, align 1
  %103 = load i8 addrspace(1)* %99, align 1
  %temp.vect118 = insertelement <4 x i8> undef, i8 %100, i32 0
  %temp.vect119 = insertelement <4 x i8> %temp.vect118, i8 %101, i32 1
  %temp.vect120 = insertelement <4 x i8> %temp.vect119, i8 %102, i32 2
  %temp.vect121 = insertelement <4 x i8> %temp.vect120, i8 %103, i32 3
  %conv21.2122 = zext <4 x i8> %temp.vect121 to <4 x i32>
  %add27.2123 = add <4 x i32> %vector56, %sub.2102
  %idxprom28.2124 = sext <4 x i32> %add27.2123 to <4 x i64>
  %extract125 = extractelement <4 x i64> %idxprom28.2124, i32 0
  %extract126 = extractelement <4 x i64> %idxprom28.2124, i32 1
  %extract127 = extractelement <4 x i64> %idxprom28.2124, i32 2
  %extract128 = extractelement <4 x i64> %idxprom28.2124, i32 3
  %104 = getelementptr inbounds i8 addrspace(1)* %pSrc, i64 %extract125
  %105 = getelementptr inbounds i8 addrspace(1)* %pSrc, i64 %extract126
  %106 = getelementptr inbounds i8 addrspace(1)* %pSrc, i64 %extract127
  %107 = getelementptr inbounds i8 addrspace(1)* %pSrc, i64 %extract128
  %108 = load i8 addrspace(1)* %104, align 1
  %109 = load i8 addrspace(1)* %105, align 1
  %110 = load i8 addrspace(1)* %106, align 1
  %111 = load i8 addrspace(1)* %107, align 1
  %temp.vect129 = insertelement <4 x i8> undef, i8 %108, i32 0
  %temp.vect130 = insertelement <4 x i8> %temp.vect129, i8 %109, i32 1
  %temp.vect131 = insertelement <4 x i8> %temp.vect130, i8 %110, i32 2
  %temp.vect132 = insertelement <4 x i8> %temp.vect131, i8 %111, i32 3
  %conv30.2133 = zext <4 x i8> %temp.vect132 to <4 x i32>
  br label %for.cond40.preheader

for.cond40.preheader:                             ; preds = %for.cond40.preheader, %for.cond6.preheader
  %iSearch.09 = phi i32 [ 0, %for.cond6.preheader ], [ %inc75, %for.cond40.preheader ]
  %vectorPHI = phi <4 x i32> [ <i32 255, i32 255, i32 255, i32 255>, %for.cond6.preheader ], [ %132, %for.cond40.preheader ]
  %vectorPHI134 = phi <4 x i32> [ zeroinitializer, %for.cond6.preheader ], [ %126, %for.cond40.preheader ]
  %vectorPHI135 = phi <4 x i32> [ <i32 128, i32 128, i32 128, i32 128>, %for.cond6.preheader ], [ %shr156, %for.cond40.preheader ]
  %112 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %conv1443, <4 x i32> %vectorPHI135)
  %msb = and <4 x i32> %112, <i32 1, i32 1, i32 1, i32 1>
  %113 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %conv2154, <4 x i32> %vectorPHI135)
  %msb187 = and <4 x i32> %113, <i32 1, i32 1, i32 1, i32 1>
  %114 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %conv3067, <4 x i32> %vectorPHI135)
  %msb189 = and <4 x i32> %114, <i32 1, i32 1, i32 1, i32 1>
  %add54139 = add <4 x i32> %msb, %msb187
  %add60140 = add <4 x i32> %add54139, %msb189
  %115 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %conv14.179, <4 x i32> %vectorPHI135)
  %msb191 = and <4 x i32> %115, <i32 1, i32 1, i32 1, i32 1>
  %116 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %conv21.188, <4 x i32> %vectorPHI135)
  %msb193 = and <4 x i32> %116, <i32 1, i32 1, i32 1, i32 1>
  %117 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %conv30.199, <4 x i32> %vectorPHI135)
  %msb195 = and <4 x i32> %117, <i32 1, i32 1, i32 1, i32 1>
  %add48.1144 = add <4 x i32> %msb191, %add60140
  %add54.1145 = add <4 x i32> %add48.1144, %msb193
  %add60.1146 = add <4 x i32> %add54.1145, %msb195
  %118 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %conv14.2113, <4 x i32> %vectorPHI135)
  %msb197 = and <4 x i32> %118, <i32 1, i32 1, i32 1, i32 1>
  %119 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %conv21.2122, <4 x i32> %vectorPHI135)
  %msb199 = and <4 x i32> %119, <i32 1, i32 1, i32 1, i32 1>
  %120 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %conv30.2133, <4 x i32> %vectorPHI135)
  %msb201 = and <4 x i32> %120, <i32 1, i32 1, i32 1, i32 1>
  %add48.2150 = add <4 x i32> %msb197, %add60.1146
  %add54.2151 = add <4 x i32> %add48.2150, %msb199
  %add60.2152 = add <4 x i32> %add54.2151, %msb201
  %121 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %add60.2152, <4 x i32> <i32 4, i32 4, i32 4, i32 4>)
  %122 = bitcast <4 x i32> %vectorPHI135 to <4 x float>
  %123 = bitcast <4 x i32> %vectorPHI134 to <4 x float>
  %124 = bitcast <4 x i32> %121 to <4 x float>
  %125 = call <4 x float> @llvm.x86.sse41.blendvps(<4 x float> %123, <4 x float> %122, <4 x float> %124)
  %126 = bitcast <4 x float> %125 to <4 x i32>
  %127 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> <i32 5, i32 5, i32 5, i32 5>, <4 x i32> %add60.2152)
  %128 = bitcast <4 x i32> %vectorPHI135 to <4 x float>
  %129 = bitcast <4 x i32> %vectorPHI to <4 x float>
  %130 = bitcast <4 x i32> %127 to <4 x float>
  %131 = call <4 x float> @llvm.x86.sse41.blendvps(<4 x float> %129, <4 x float> %128, <4 x float> %130)
  %132 = bitcast <4 x float> %131 to <4 x i32>
  %add73155 = add nsw <4 x i32> %132, %126
  %shr156 = ashr <4 x i32> %add73155, <i32 1, i32 1, i32 1, i32 1>
  %inc75 = add nsw i32 %iSearch.09, 1
  %exitcond = icmp eq i32 %inc75, 8
  br i1 %exitcond, label %for.end76, label %for.cond40.preheader

for.end76:                                        ; preds = %for.cond40.preheader
  %extract160 = extractelement <4 x i32> %shr156, i32 3
  %extract159 = extractelement <4 x i32> %shr156, i32 2
  %extract158 = extractelement <4 x i32> %shr156, i32 1
  %extract157 = extractelement <4 x i32> %shr156, i32 0
  %133 = getelementptr inbounds [4 x i32]* %8, i64 0, i64 %indvars.iv
  %134 = getelementptr inbounds [4 x i32]* %9, i64 0, i64 %indvars.iv
  %135 = getelementptr inbounds [4 x i32]* %10, i64 0, i64 %indvars.iv
  %136 = getelementptr inbounds [4 x i32]* %11, i64 0, i64 %indvars.iv
  store i32 %extract157, i32* %133, align 4
  store i32 %extract158, i32* %134, align 4
  store i32 %extract159, i32* %135, align 4
  store i32 %extract160, i32* %136, align 4
  %indvars.iv.next = add i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond24 = icmp eq i32 %lftr.wideiv, 3
  br i1 %exitcond24, label %for.end81, label %for.cond6.preheader

for.end81:                                        ; preds = %for.end76
  %137 = load i64* %4, align 16
  %138 = load i64* %5, align 16
  %139 = load i64* %6, align 16
  %140 = load i64* %7, align 16
  %temp.vect161 = insertelement <4 x i64> undef, i64 %137, i32 0
  %temp.vect162 = insertelement <4 x i64> %temp.vect161, i64 %138, i32 1
  %temp.vect163 = insertelement <4 x i64> %temp.vect162, i64 %139, i32 2
  %temp.vect164 = insertelement <4 x i64> %temp.vect163, i64 %140, i32 3
  %141 = trunc <4 x i64> %temp.vect164 to <4 x i32>
  %and165 = and <4 x i32> %141, <i32 255, i32 255, i32 255, i32 255>
  %142 = lshr <4 x i64> %temp.vect164, <i64 24, i64 24, i64 24, i64 24>
  %.tr166 = trunc <4 x i64> %142 to <4 x i32>
  %and84167 = and <4 x i32> %.tr166, <i32 65280, i32 65280, i32 65280, i32 65280>
  %143 = load i32* %17, align 8
  %144 = load i32* %18, align 8
  %145 = load i32* %19, align 8
  %146 = load i32* %20, align 8
  %temp.vect168 = insertelement <4 x i32> undef, i32 %143, i32 0
  %temp.vect169 = insertelement <4 x i32> %temp.vect168, i32 %144, i32 1
  %temp.vect170 = insertelement <4 x i32> %temp.vect169, i32 %145, i32 2
  %temp.vect171 = insertelement <4 x i32> %temp.vect170, i32 %146, i32 3
  %shl86172 = shl <4 x i32> %temp.vect171, <i32 16, i32 16, i32 16, i32 16>
  %and87173 = and <4 x i32> %shl86172, <i32 16711680, i32 16711680, i32 16711680, i32 16711680>
  %or174 = or <4 x i32> %and84167, %and165
  %or88175 = or <4 x i32> %or174, %and87173
  %extract180 = extractelement <4 x i32> %or88175, i32 0
  %extract181 = extractelement <4 x i32> %or88175, i32 1
  %extract182 = extractelement <4 x i32> %or88175, i32 2
  %extract183 = extractelement <4 x i32> %or88175, i32 3
  %147 = add nsw <4 x i64> %vector31, %21
  %extract176 = extractelement <4 x i64> %147, i32 0
  %extract177 = extractelement <4 x i64> %147, i32 1
  %extract178 = extractelement <4 x i64> %147, i32 2
  %extract179 = extractelement <4 x i64> %147, i32 3
  %148 = getelementptr inbounds i32 addrspace(1)* %pDst, i64 %extract176
  %149 = getelementptr inbounds i32 addrspace(1)* %pDst, i64 %extract177
  %150 = getelementptr inbounds i32 addrspace(1)* %pDst, i64 %extract178
  %151 = getelementptr inbounds i32 addrspace(1)* %pDst, i64 %extract179
  store i32 %extract180, i32 addrspace(1)* %148, align 4
  store i32 %extract181, i32 addrspace(1)* %149, align 4
  store i32 %extract182, i32 addrspace(1)* %150, align 4
  store i32 %extract183, i32 addrspace(1)* %151, align 4
  %indvars.iv.next26 = add i64 %indvars.iv25, 1
  %lftr.wideiv31 = trunc i64 %indvars.iv.next26 to i32
  %exitcond32 = icmp eq i32 %lftr.wideiv31, %iImageWidth
  br i1 %exitcond32, label %for.end96, label %for.cond2.preheader

for.end96:                                        ; preds = %for.end81
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
