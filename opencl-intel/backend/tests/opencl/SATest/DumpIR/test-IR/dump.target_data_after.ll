; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @intel_median(<4 x i8> addrspace(1)* %pSrc, i32 addrspace(1)* %pDst, i32 %iImageWidth, i32 %iImageHeight) nounwind {
entry:
  %iPixels = alloca [9 x <4 x i32>], align 16
  %call = call i64 @get_global_id(i32 0) nounwind readnone
  %conv = trunc i64 %call to i32
  br label %for.cond

for.cond:                                         ; preds = %for.end83, %entry
  %x.0 = phi i32 [ 0, %entry ], [ %inc96, %for.end83 ]
  %cmp = icmp slt i32 %x.0, %iImageWidth
  br i1 %cmp, label %for.body, label %for.end97

for.body:                                         ; preds = %for.cond
  br label %for.cond2

for.cond2:                                        ; preds = %for.body5, %for.body
  %iPixelCount.0 = phi i32 [ 0, %for.body ], [ %inc41, %for.body5 ]
  %iRow.0 = phi i32 [ -1, %for.body ], [ %inc42, %for.body5 ]
  %cmp3 = icmp slt i32 %iRow.0, 2
  br i1 %cmp3, label %for.body5, label %for.end

for.body5:                                        ; preds = %for.cond2
  %add = add nsw i32 %conv, %iRow.0
  %add6 = add nsw i32 %add, 2
  %mul = mul nsw i32 %add6, %iImageWidth
  %add7 = add nsw i32 %mul, %x.0
  %sub = add nsw i32 %add7, -1
  %idxprom = sext i32 %sub to i64
  %arrayidx = getelementptr inbounds <4 x i8> addrspace(1)* %pSrc, i64 %idxprom
  %0 = load <4 x i8> addrspace(1)* %arrayidx, align 4
  %1 = extractelement <4 x i8> %0, i32 0
  %conv8 = zext i8 %1 to i32
  %idxprom9 = sext i32 %iPixelCount.0 to i64
  %arrayidx10 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i64 0, i64 %idxprom9
  %2 = load <4 x i32>* %arrayidx10, align 16
  %3 = insertelement <4 x i32> %2, i32 %conv8, i32 0
  store <4 x i32> %3, <4 x i32>* %arrayidx10, align 16
  %4 = extractelement <4 x i8> %0, i32 1
  %conv11 = zext i8 %4 to i32
  %idxprom12 = sext i32 %iPixelCount.0 to i64
  %arrayidx13 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i64 0, i64 %idxprom12
  %5 = load <4 x i32>* %arrayidx13, align 16
  %6 = insertelement <4 x i32> %5, i32 %conv11, i32 1
  store <4 x i32> %6, <4 x i32>* %arrayidx13, align 16
  %7 = extractelement <4 x i8> %0, i32 2
  %conv14 = zext i8 %7 to i32
  %idxprom15 = sext i32 %iPixelCount.0 to i64
  %arrayidx16 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i64 0, i64 %idxprom15
  %8 = load <4 x i32>* %arrayidx16, align 16
  %9 = insertelement <4 x i32> %8, i32 %conv14, i32 2
  store <4 x i32> %9, <4 x i32>* %arrayidx16, align 16
  %inc = add nsw i32 %iPixelCount.0, 1
  %idxprom17 = sext i32 %add7 to i64
  %arrayidx18 = getelementptr inbounds <4 x i8> addrspace(1)* %pSrc, i64 %idxprom17
  %10 = load <4 x i8> addrspace(1)* %arrayidx18, align 4
  %11 = extractelement <4 x i8> %10, i32 0
  %conv19 = zext i8 %11 to i32
  %idxprom20 = sext i32 %inc to i64
  %arrayidx21 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i64 0, i64 %idxprom20
  %12 = load <4 x i32>* %arrayidx21, align 16
  %13 = insertelement <4 x i32> %12, i32 %conv19, i32 0
  store <4 x i32> %13, <4 x i32>* %arrayidx21, align 16
  %14 = extractelement <4 x i8> %10, i32 1
  %conv22 = zext i8 %14 to i32
  %idxprom23 = sext i32 %inc to i64
  %arrayidx24 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i64 0, i64 %idxprom23
  %15 = load <4 x i32>* %arrayidx24, align 16
  %16 = insertelement <4 x i32> %15, i32 %conv22, i32 1
  store <4 x i32> %16, <4 x i32>* %arrayidx24, align 16
  %17 = extractelement <4 x i8> %10, i32 2
  %conv25 = zext i8 %17 to i32
  %idxprom26 = sext i32 %inc to i64
  %arrayidx27 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i64 0, i64 %idxprom26
  %18 = load <4 x i32>* %arrayidx27, align 16
  %19 = insertelement <4 x i32> %18, i32 %conv25, i32 2
  store <4 x i32> %19, <4 x i32>* %arrayidx27, align 16
  %inc28 = add nsw i32 %iPixelCount.0, 2
  %add29 = add nsw i32 %add7, 1
  %idxprom30 = sext i32 %add29 to i64
  %arrayidx31 = getelementptr inbounds <4 x i8> addrspace(1)* %pSrc, i64 %idxprom30
  %20 = load <4 x i8> addrspace(1)* %arrayidx31, align 4
  %21 = extractelement <4 x i8> %20, i32 0
  %conv32 = zext i8 %21 to i32
  %idxprom33 = sext i32 %inc28 to i64
  %arrayidx34 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i64 0, i64 %idxprom33
  %22 = load <4 x i32>* %arrayidx34, align 16
  %23 = insertelement <4 x i32> %22, i32 %conv32, i32 0
  store <4 x i32> %23, <4 x i32>* %arrayidx34, align 16
  %24 = extractelement <4 x i8> %20, i32 1
  %conv35 = zext i8 %24 to i32
  %idxprom36 = sext i32 %inc28 to i64
  %arrayidx37 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i64 0, i64 %idxprom36
  %25 = load <4 x i32>* %arrayidx37, align 16
  %26 = insertelement <4 x i32> %25, i32 %conv35, i32 1
  store <4 x i32> %26, <4 x i32>* %arrayidx37, align 16
  %27 = extractelement <4 x i8> %20, i32 2
  %conv38 = zext i8 %27 to i32
  %idxprom39 = sext i32 %inc28 to i64
  %arrayidx40 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i64 0, i64 %idxprom39
  %28 = load <4 x i32>* %arrayidx40, align 16
  %29 = insertelement <4 x i32> %28, i32 %conv38, i32 2
  store <4 x i32> %29, <4 x i32>* %arrayidx40, align 16
  %inc41 = add nsw i32 %iPixelCount.0, 3
  %inc42 = add nsw i32 %iRow.0, 1
  br label %for.cond2

for.end:                                          ; preds = %for.cond2
  br label %for.cond43

for.cond43:                                       ; preds = %for.end71, %for.end
  %iMax.0 = phi <4 x i32> [ <i32 255, i32 255, i32 255, i32 255>, %for.end ], [ %or79, %for.end71 ]
  %iMin.0 = phi <4 x i32> [ zeroinitializer, %for.end ], [ %or, %for.end71 ]
  %iYes.0 = phi <4 x i32> [ <i32 128, i32 128, i32 128, i32 128>, %for.end ], [ %shr, %for.end71 ]
  %iSearch.0 = phi i32 [ 0, %for.end ], [ %inc82, %for.end71 ]
  %cmp44 = icmp slt i32 %iSearch.0, 8
  br i1 %cmp44, label %for.body46, label %for.end83

for.body46:                                       ; preds = %for.cond43
  br label %for.cond48

for.cond48:                                       ; preds = %for.body51, %for.body46
  %iPixelCount.1 = phi i32 [ 0, %for.body46 ], [ %inc68, %for.body51 ]
  %iHighCount.0 = phi <4 x i32> [ zeroinitializer, %for.body46 ], [ %add67, %for.body51 ]
  %iRow47.0 = phi i32 [ -1, %for.body46 ], [ %inc70, %for.body51 ]
  %cmp49 = icmp slt i32 %iRow47.0, 2
  br i1 %cmp49, label %for.body51, label %for.end71

for.body51:                                       ; preds = %for.cond48
  %idxprom52 = sext i32 %iPixelCount.1 to i64
  %arrayidx53 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i64 0, i64 %idxprom52
  %30 = load <4 x i32>* %arrayidx53, align 16
  %cmp54 = icmp slt <4 x i32> %iYes.0, %30
  %sext = sext <4 x i1> %cmp54 to <4 x i32>
  %add55 = add <4 x i32> %iHighCount.0, %sext
  %inc56 = add nsw i32 %iPixelCount.1, 1
  %idxprom57 = sext i32 %inc56 to i64
  %arrayidx58 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i64 0, i64 %idxprom57
  %31 = load <4 x i32>* %arrayidx58, align 16
  %cmp59 = icmp slt <4 x i32> %iYes.0, %31
  %sext60 = sext <4 x i1> %cmp59 to <4 x i32>
  %add61 = add <4 x i32> %add55, %sext60
  %inc62 = add nsw i32 %iPixelCount.1, 2
  %idxprom63 = sext i32 %inc62 to i64
  %arrayidx64 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i64 0, i64 %idxprom63
  %32 = load <4 x i32>* %arrayidx64, align 16
  %cmp65 = icmp slt <4 x i32> %iYes.0, %32
  %sext66 = sext <4 x i1> %cmp65 to <4 x i32>
  %add67 = add <4 x i32> %add61, %sext66
  %inc68 = add nsw i32 %iPixelCount.1, 3
  %inc70 = add nsw i32 %iRow47.0, 1
  br label %for.cond48

for.end71:                                        ; preds = %for.cond48
  %sub72 = sub <4 x i32> zeroinitializer, %iHighCount.0
  %cmp73 = icmp sgt <4 x i32> %sub72, <i32 4, i32 4, i32 4, i32 4>
  %sext74 = sext <4 x i1> %cmp73 to <4 x i32>
  %and = and <4 x i32> %iYes.0, %sext74
  %neg = xor <4 x i32> %sext74, <i32 -1, i32 -1, i32 -1, i32 -1>
  %and75 = and <4 x i32> %iMin.0, %neg
  %or = or <4 x i32> %and, %and75
  %neg76 = xor <4 x i32> %sext74, <i32 -1, i32 -1, i32 -1, i32 -1>
  %and77 = and <4 x i32> %iYes.0, %neg76
  %and78 = and <4 x i32> %iMax.0, %sext74
  %or79 = or <4 x i32> %and77, %and78
  %add80 = add <4 x i32> %or79, %or
  %shr = ashr <4 x i32> %add80, <i32 1, i32 1, i32 1, i32 1>
  %inc82 = add nsw i32 %iSearch.0, 1
  br label %for.cond43

for.end83:                                        ; preds = %for.cond43
  %33 = extractelement <4 x i32> %iYes.0, i32 0
  %and84 = and i32 %33, 255
  %34 = extractelement <4 x i32> %iYes.0, i32 1
  %shl = shl i32 %34, 8
  %and85 = and i32 %shl, 65280
  %or86 = or i32 %and84, %and85
  %35 = extractelement <4 x i32> %iYes.0, i32 2
  %shl87 = shl i32 %35, 16
  %and88 = and i32 %shl87, 16711680
  %or89 = or i32 %or86, %and88
  %add90 = add nsw i32 %conv, 2
  %mul91 = mul nsw i32 %add90, %iImageWidth
  %add92 = add nsw i32 %mul91, %x.0
  %idxprom93 = sext i32 %add92 to i64
  %arrayidx94 = getelementptr inbounds i32 addrspace(1)* %pDst, i64 %idxprom93
  store i32 %or89, i32 addrspace(1)* %arrayidx94, align 4
  %inc96 = add nsw i32 %x.0, 1
  br label %for.cond

for.end97:                                        ; preds = %for.cond
  ret void
}

declare i64 @get_global_id(i32) nounwind readnone

define void @intel_median_scalar(i8 addrspace(1)* %pSrc, i32 addrspace(1)* %pDst, i32 %iImageWidth, i32 %iImageHeight) nounwind {
entry:
  %iResult = alloca [4 x i32], align 16
  %iPixels = alloca [9 x i32], align 16
  %call = call i64 @get_global_id(i32 0) nounwind readnone
  %conv = trunc i64 %call to i32
  br label %for.cond

for.cond:                                         ; preds = %for.end81, %entry
  %x.0 = phi i32 [ 0, %entry ], [ %inc95, %for.end81 ]
  %cmp = icmp slt i32 %x.0, %iImageWidth
  br i1 %cmp, label %for.body, label %for.end96

for.body:                                         ; preds = %for.cond
  br label %for.cond2

for.cond2:                                        ; preds = %for.end76, %for.body
  %ch.0 = phi i32 [ 0, %for.body ], [ %inc80, %for.end76 ]
  %cmp3 = icmp slt i32 %ch.0, 3
  br i1 %cmp3, label %for.body5, label %for.end81

for.body5:                                        ; preds = %for.cond2
  br label %for.cond6

for.cond6:                                        ; preds = %for.body9, %for.body5
  %iPixelCount.0 = phi i32 [ 0, %for.body5 ], [ %inc33, %for.body9 ]
  %iRow.0 = phi i32 [ -1, %for.body5 ], [ %inc34, %for.body9 ]
  %cmp7 = icmp slt i32 %iRow.0, 2
  br i1 %cmp7, label %for.body9, label %for.end

for.body9:                                        ; preds = %for.cond6
  %add = add nsw i32 %conv, %iRow.0
  %add10 = add nsw i32 %add, 2
  %mul = mul nsw i32 %add10, %iImageWidth
  %add11 = add nsw i32 %mul, %x.0
  %sub = shl i32 %add11, 2
  %mul12 = add i32 %sub, -4
  %add13 = add nsw i32 %mul12, %ch.0
  %idxprom = sext i32 %add13 to i64
  %arrayidx = getelementptr inbounds i8 addrspace(1)* %pSrc, i64 %idxprom
  %0 = load i8 addrspace(1)* %arrayidx, align 1
  %conv14 = zext i8 %0 to i32
  %idxprom15 = sext i32 %iPixelCount.0 to i64
  %arrayidx16 = getelementptr inbounds [9 x i32]* %iPixels, i64 0, i64 %idxprom15
  store i32 %conv14, i32* %arrayidx16, align 4
  %inc = add nsw i32 %iPixelCount.0, 1
  %mul17 = shl nsw i32 %add11, 2
  %add18 = add nsw i32 %mul17, %ch.0
  %idxprom19 = sext i32 %add18 to i64
  %arrayidx20 = getelementptr inbounds i8 addrspace(1)* %pSrc, i64 %idxprom19
  %1 = load i8 addrspace(1)* %arrayidx20, align 1
  %conv21 = zext i8 %1 to i32
  %idxprom22 = sext i32 %inc to i64
  %arrayidx23 = getelementptr inbounds [9 x i32]* %iPixels, i64 0, i64 %idxprom22
  store i32 %conv21, i32* %arrayidx23, align 4
  %inc24 = add nsw i32 %iPixelCount.0, 2
  %add25 = shl i32 %add11, 2
  %mul26 = add i32 %add25, 4
  %add27 = add nsw i32 %mul26, %ch.0
  %idxprom28 = sext i32 %add27 to i64
  %arrayidx29 = getelementptr inbounds i8 addrspace(1)* %pSrc, i64 %idxprom28
  %2 = load i8 addrspace(1)* %arrayidx29, align 1
  %conv30 = zext i8 %2 to i32
  %idxprom31 = sext i32 %inc24 to i64
  %arrayidx32 = getelementptr inbounds [9 x i32]* %iPixels, i64 0, i64 %idxprom31
  store i32 %conv30, i32* %arrayidx32, align 4
  %inc33 = add nsw i32 %iPixelCount.0, 3
  %inc34 = add nsw i32 %iRow.0, 1
  br label %for.cond6

for.end:                                          ; preds = %for.cond6
  br label %for.cond35

for.cond35:                                       ; preds = %for.end64, %for.end
  %iYes.0 = phi i32 [ 128, %for.end ], [ %shr, %for.end64 ]
  %iMin.0 = phi i32 [ 0, %for.end ], [ %cond, %for.end64 ]
  %iMax.0 = phi i32 [ 255, %for.end ], [ %cond72, %for.end64 ]
  %iSearch.0 = phi i32 [ 0, %for.end ], [ %inc75, %for.end64 ]
  %cmp36 = icmp slt i32 %iSearch.0, 8
  br i1 %cmp36, label %for.body38, label %for.end76

for.body38:                                       ; preds = %for.cond35
  br label %for.cond40

for.cond40:                                       ; preds = %for.body43, %for.body38
  %iPixelCount.1 = phi i32 [ 0, %for.body38 ], [ %inc61, %for.body43 ]
  %iHighCount.0 = phi i32 [ 0, %for.body38 ], [ %add60, %for.body43 ]
  %iRow39.0 = phi i32 [ -1, %for.body38 ], [ %inc63, %for.body43 ]
  %cmp41 = icmp slt i32 %iRow39.0, 2
  br i1 %cmp41, label %for.body43, label %for.end64

for.body43:                                       ; preds = %for.cond40
  %idxprom44 = sext i32 %iPixelCount.1 to i64
  %arrayidx45 = getelementptr inbounds [9 x i32]* %iPixels, i64 0, i64 %idxprom44
  %3 = load i32* %arrayidx45, align 4
  %cmp46 = icmp slt i32 %iYes.0, %3
  %conv47 = zext i1 %cmp46 to i32
  %add48 = add nsw i32 %iHighCount.0, %conv47
  %inc49 = add nsw i32 %iPixelCount.1, 1
  %idxprom50 = sext i32 %inc49 to i64
  %arrayidx51 = getelementptr inbounds [9 x i32]* %iPixels, i64 0, i64 %idxprom50
  %4 = load i32* %arrayidx51, align 4
  %cmp52 = icmp slt i32 %iYes.0, %4
  %conv53 = zext i1 %cmp52 to i32
  %add54 = add nsw i32 %add48, %conv53
  %inc55 = add nsw i32 %iPixelCount.1, 2
  %idxprom56 = sext i32 %inc55 to i64
  %arrayidx57 = getelementptr inbounds [9 x i32]* %iPixels, i64 0, i64 %idxprom56
  %5 = load i32* %arrayidx57, align 4
  %cmp58 = icmp slt i32 %iYes.0, %5
  %conv59 = zext i1 %cmp58 to i32
  %add60 = add nsw i32 %add54, %conv59
  %inc61 = add nsw i32 %iPixelCount.1, 3
  %inc63 = add nsw i32 %iRow39.0, 1
  br label %for.cond40

for.end64:                                        ; preds = %for.cond40
  %cmp65 = icmp sgt i32 %iHighCount.0, 4
  %cond = select i1 %cmp65, i32 %iYes.0, i32 %iMin.0
  %cmp67 = icmp slt i32 %iHighCount.0, 5
  %cond72 = select i1 %cmp67, i32 %iYes.0, i32 %iMax.0
  %add73 = add nsw i32 %cond72, %cond
  %shr = ashr i32 %add73, 1
  %inc75 = add nsw i32 %iSearch.0, 1
  br label %for.cond35

for.end76:                                        ; preds = %for.cond35
  %idxprom77 = sext i32 %ch.0 to i64
  %arrayidx78 = getelementptr inbounds [4 x i32]* %iResult, i64 0, i64 %idxprom77
  store i32 %iYes.0, i32* %arrayidx78, align 4
  %inc80 = add nsw i32 %ch.0, 1
  br label %for.cond2

for.end81:                                        ; preds = %for.cond2
  %arrayidx82 = getelementptr inbounds [4 x i32]* %iResult, i64 0, i64 0
  %6 = load i32* %arrayidx82, align 16
  %and = and i32 %6, 255
  %arrayidx83 = getelementptr inbounds [4 x i32]* %iResult, i64 0, i64 1
  %7 = load i32* %arrayidx83, align 4
  %shl = shl i32 %7, 8
  %and84 = and i32 %shl, 65280
  %or = or i32 %and, %and84
  %arrayidx85 = getelementptr inbounds [4 x i32]* %iResult, i64 0, i64 2
  %8 = load i32* %arrayidx85, align 8
  %shl86 = shl i32 %8, 16
  %and87 = and i32 %shl86, 16711680
  %or88 = or i32 %or, %and87
  %add89 = add nsw i32 %conv, 2
  %mul90 = mul nsw i32 %add89, %iImageWidth
  %add91 = add nsw i32 %mul90, %x.0
  %idxprom92 = sext i32 %add91 to i64
  %arrayidx93 = getelementptr inbounds i32 addrspace(1)* %pDst, i64 %idxprom92
  store i32 %or88, i32 addrspace(1)* %arrayidx93, align 4
  %inc95 = add nsw i32 %x.0, 1
  br label %for.cond

for.end96:                                        ; preds = %for.cond
  ret void
}

!opencl.kernels = !{!0, !8}
!opencl.build.options = !{!11}

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
