; ModuleID = 'Program'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"
target triple = "i686-pc-win32"

define void @intel_median(<4 x i8> addrspace(1)* %pSrc, i32 addrspace(1)* %pDst, i32 %iImageWidth, i32 %iImageHeight) nounwind {
entry:
  %iPixels = alloca [9 x <4 x i32>], align 16
  %call = call i32 @get_global_id(i32 0) nounwind readnone
  br label %for.cond

for.cond:                                         ; preds = %for.end66, %entry
  %x.0 = phi i32 [ 0, %entry ], [ %inc78, %for.end66 ]
  %cmp = icmp slt i32 %x.0, %iImageWidth
  br i1 %cmp, label %for.body, label %for.end79

for.body:                                         ; preds = %for.cond
  br label %for.cond1

for.cond1:                                        ; preds = %for.body3, %for.body
  %iPixelCount.0 = phi i32 [ 0, %for.body ], [ %inc27, %for.body3 ]
  %iRow.0 = phi i32 [ -1, %for.body ], [ %inc28, %for.body3 ]
  %cmp2 = icmp slt i32 %iRow.0, 2
  br i1 %cmp2, label %for.body3, label %for.end

for.body3:                                        ; preds = %for.cond1
  %add = add nsw i32 %call, %iRow.0
  %add4 = add nsw i32 %add, 2
  %mul = mul nsw i32 %add4, %iImageWidth
  %add5 = add nsw i32 %mul, %x.0
  %sub = add nsw i32 %add5, -1
  %arrayidx = getelementptr inbounds <4 x i8> addrspace(1)* %pSrc, i32 %sub
  %0 = load <4 x i8> addrspace(1)* %arrayidx, align 4
  %1 = extractelement <4 x i8> %0, i32 0
  %conv = zext i8 %1 to i32
  %arrayidx6 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 %iPixelCount.0
  %2 = load <4 x i32>* %arrayidx6, align 16
  %3 = insertelement <4 x i32> %2, i32 %conv, i32 0
  %4 = extractelement <4 x i8> %0, i32 1
  %conv7 = zext i8 %4 to i32
  %5 = insertelement <4 x i32> %3, i32 %conv7, i32 1
  %6 = extractelement <4 x i8> %0, i32 2
  %conv9 = zext i8 %6 to i32
  %arrayidx10 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 %iPixelCount.0
  %7 = insertelement <4 x i32> %5, i32 %conv9, i32 2
  store <4 x i32> %7, <4 x i32>* %arrayidx10, align 16
  %inc = add nsw i32 %iPixelCount.0, 1
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
  %arrayidx17 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 %inc
  %15 = insertelement <4 x i32> %13, i32 %conv16, i32 2
  store <4 x i32> %15, <4 x i32>* %arrayidx17, align 16
  %inc18 = add nsw i32 %iPixelCount.0, 2
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
  %arrayidx26 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 %inc18
  %23 = insertelement <4 x i32> %21, i32 %conv25, i32 2
  store <4 x i32> %23, <4 x i32>* %arrayidx26, align 16
  %inc27 = add nsw i32 %iPixelCount.0, 3
  %inc28 = add nsw i32 %iRow.0, 1
  br label %for.cond1

for.end:                                          ; preds = %for.cond1
  br label %for.cond29

for.cond29:                                       ; preds = %for.end54, %for.end
  %iMax.0 = phi <4 x i32> [ <i32 255, i32 255, i32 255, i32 255>, %for.end ], [ %or62, %for.end54 ]
  %iMin.0 = phi <4 x i32> [ zeroinitializer, %for.end ], [ %or, %for.end54 ]
  %iYes.0 = phi <4 x i32> [ <i32 128, i32 128, i32 128, i32 128>, %for.end ], [ %shr, %for.end54 ]
  %iSearch.0 = phi i32 [ 0, %for.end ], [ %inc65, %for.end54 ]
  %cmp30 = icmp slt i32 %iSearch.0, 8
  br i1 %cmp30, label %for.body32, label %for.end66

for.body32:                                       ; preds = %for.cond29
  br label %for.cond34

for.cond34:                                       ; preds = %for.body37, %for.body32
  %iPixelCount.1 = phi i32 [ 0, %for.body32 ], [ %inc51, %for.body37 ]
  %iHighCount.0 = phi <4 x i32> [ zeroinitializer, %for.body32 ], [ %add50, %for.body37 ]
  %iRow33.0 = phi i32 [ -1, %for.body32 ], [ %inc53, %for.body37 ]
  %cmp35 = icmp slt i32 %iRow33.0, 2
  br i1 %cmp35, label %for.body37, label %for.end54

for.body37:                                       ; preds = %for.cond34
  %arrayidx38 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 %iPixelCount.1
  %24 = load <4 x i32>* %arrayidx38, align 16
  %cmp39 = icmp slt <4 x i32> %iYes.0, %24
  %sext = sext <4 x i1> %cmp39 to <4 x i32>
  %add40 = add <4 x i32> %iHighCount.0, %sext
  %inc41 = add nsw i32 %iPixelCount.1, 1
  %arrayidx42 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 %inc41
  %25 = load <4 x i32>* %arrayidx42, align 16
  %cmp43 = icmp slt <4 x i32> %iYes.0, %25
  %sext44 = sext <4 x i1> %cmp43 to <4 x i32>
  %add45 = add <4 x i32> %add40, %sext44
  %inc46 = add nsw i32 %iPixelCount.1, 2
  %arrayidx47 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 %inc46
  %26 = load <4 x i32>* %arrayidx47, align 16
  %cmp48 = icmp slt <4 x i32> %iYes.0, %26
  %sext49 = sext <4 x i1> %cmp48 to <4 x i32>
  %add50 = add <4 x i32> %add45, %sext49
  %inc51 = add nsw i32 %iPixelCount.1, 3
  %inc53 = add nsw i32 %iRow33.0, 1
  br label %for.cond34

for.end54:                                        ; preds = %for.cond34
  %sub55 = sub <4 x i32> zeroinitializer, %iHighCount.0
  %cmp56 = icmp sgt <4 x i32> %sub55, <i32 4, i32 4, i32 4, i32 4>
  %sext57 = sext <4 x i1> %cmp56 to <4 x i32>
  %and = and <4 x i32> %iYes.0, %sext57
  %neg = xor <4 x i32> %sext57, <i32 -1, i32 -1, i32 -1, i32 -1>
  %and58 = and <4 x i32> %iMin.0, %neg
  %or = or <4 x i32> %and, %and58
  %neg59 = xor <4 x i32> %sext57, <i32 -1, i32 -1, i32 -1, i32 -1>
  %and60 = and <4 x i32> %iYes.0, %neg59
  %and61 = and <4 x i32> %iMax.0, %sext57
  %or62 = or <4 x i32> %and60, %and61
  %add63 = add <4 x i32> %or62, %or
  %shr = ashr <4 x i32> %add63, <i32 1, i32 1, i32 1, i32 1>
  %inc65 = add nsw i32 %iSearch.0, 1
  br label %for.cond29

for.end66:                                        ; preds = %for.cond29
  %27 = extractelement <4 x i32> %iYes.0, i32 0
  %and67 = and i32 %27, 255
  %28 = extractelement <4 x i32> %iYes.0, i32 1
  %shl = shl i32 %28, 8
  %and68 = and i32 %shl, 65280
  %or69 = or i32 %and67, %and68
  %29 = extractelement <4 x i32> %iYes.0, i32 2
  %shl70 = shl i32 %29, 16
  %and71 = and i32 %shl70, 16711680
  %or72 = or i32 %or69, %and71
  %add73 = add nsw i32 %call, 2
  %mul74 = mul nsw i32 %add73, %iImageWidth
  %add75 = add nsw i32 %mul74, %x.0
  %arrayidx76 = getelementptr inbounds i32 addrspace(1)* %pDst, i32 %add75
  store i32 %or72, i32 addrspace(1)* %arrayidx76, align 4
  %inc78 = add nsw i32 %x.0, 1
  br label %for.cond

for.end79:                                        ; preds = %for.cond
  ret void
}

declare i32 @get_global_id(i32) nounwind readnone

define void @intel_median_scalar(i8 addrspace(1)* %pSrc, i32 addrspace(1)* %pDst, i32 %iImageWidth, i32 %iImageHeight) nounwind {
entry:
  %iResult = alloca [4 x i32], align 4
  %iPixels = alloca [9 x i32], align 4
  %call = call i32 @get_global_id(i32 0) nounwind readnone
  br label %for.cond

for.cond:                                         ; preds = %for.end68, %entry
  %x.0 = phi i32 [ 0, %entry ], [ %inc81, %for.end68 ]
  %cmp = icmp slt i32 %x.0, %iImageWidth
  br i1 %cmp, label %for.body, label %for.end82

for.body:                                         ; preds = %for.cond
  br label %for.cond1

for.cond1:                                        ; preds = %for.end64, %for.body
  %ch.0 = phi i32 [ 0, %for.body ], [ %inc67, %for.end64 ]
  %cmp2 = icmp slt i32 %ch.0, 3
  br i1 %cmp2, label %for.body3, label %for.end68

for.body3:                                        ; preds = %for.cond1
  br label %for.cond4

for.cond4:                                        ; preds = %for.body6, %for.body3
  %iPixelCount.0 = phi i32 [ 0, %for.body3 ], [ %inc24, %for.body6 ]
  %iRow.0 = phi i32 [ -1, %for.body3 ], [ %inc25, %for.body6 ]
  %cmp5 = icmp slt i32 %iRow.0, 2
  br i1 %cmp5, label %for.body6, label %for.end

for.body6:                                        ; preds = %for.cond4
  %add = add nsw i32 %call, %iRow.0
  %add7 = add nsw i32 %add, 2
  %mul = mul nsw i32 %add7, %iImageWidth
  %add8 = add nsw i32 %mul, %x.0
  %sub = shl i32 %add8, 2
  %mul9 = add i32 %sub, -4
  %add10 = add nsw i32 %mul9, %ch.0
  %arrayidx = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %add10
  %0 = load i8 addrspace(1)* %arrayidx, align 1
  %conv = zext i8 %0 to i32
  %arrayidx11 = getelementptr inbounds [9 x i32]* %iPixels, i32 0, i32 %iPixelCount.0
  store i32 %conv, i32* %arrayidx11, align 4
  %inc = add nsw i32 %iPixelCount.0, 1
  %mul12 = shl nsw i32 %add8, 2
  %add13 = add nsw i32 %mul12, %ch.0
  %arrayidx14 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %add13
  %1 = load i8 addrspace(1)* %arrayidx14, align 1
  %conv15 = zext i8 %1 to i32
  %arrayidx16 = getelementptr inbounds [9 x i32]* %iPixels, i32 0, i32 %inc
  store i32 %conv15, i32* %arrayidx16, align 4
  %inc17 = add nsw i32 %iPixelCount.0, 2
  %add18 = shl i32 %add8, 2
  %mul19 = add i32 %add18, 4
  %add20 = add nsw i32 %mul19, %ch.0
  %arrayidx21 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %add20
  %2 = load i8 addrspace(1)* %arrayidx21, align 1
  %conv22 = zext i8 %2 to i32
  %arrayidx23 = getelementptr inbounds [9 x i32]* %iPixels, i32 0, i32 %inc17
  store i32 %conv22, i32* %arrayidx23, align 4
  %inc24 = add nsw i32 %iPixelCount.0, 3
  %inc25 = add nsw i32 %iRow.0, 1
  br label %for.cond4

for.end:                                          ; preds = %for.cond4
  br label %for.cond26

for.cond26:                                       ; preds = %for.end52, %for.end
  %iYes.0 = phi i32 [ 128, %for.end ], [ %shr, %for.end52 ]
  %iMin.0 = phi i32 [ 0, %for.end ], [ %cond, %for.end52 ]
  %iMax.0 = phi i32 [ 255, %for.end ], [ %cond60, %for.end52 ]
  %iSearch.0 = phi i32 [ 0, %for.end ], [ %inc63, %for.end52 ]
  %cmp27 = icmp slt i32 %iSearch.0, 8
  br i1 %cmp27, label %for.body29, label %for.end64

for.body29:                                       ; preds = %for.cond26
  br label %for.cond31

for.cond31:                                       ; preds = %for.body34, %for.body29
  %iPixelCount.1 = phi i32 [ 0, %for.body29 ], [ %inc49, %for.body34 ]
  %iHighCount.0 = phi i32 [ 0, %for.body29 ], [ %add48, %for.body34 ]
  %iRow30.0 = phi i32 [ -1, %for.body29 ], [ %inc51, %for.body34 ]
  %cmp32 = icmp slt i32 %iRow30.0, 2
  br i1 %cmp32, label %for.body34, label %for.end52

for.body34:                                       ; preds = %for.cond31
  %arrayidx35 = getelementptr inbounds [9 x i32]* %iPixels, i32 0, i32 %iPixelCount.1
  %3 = load i32* %arrayidx35, align 4
  %cmp36 = icmp slt i32 %iYes.0, %3
  %conv37 = zext i1 %cmp36 to i32
  %add38 = add nsw i32 %iHighCount.0, %conv37
  %inc39 = add nsw i32 %iPixelCount.1, 1
  %arrayidx40 = getelementptr inbounds [9 x i32]* %iPixels, i32 0, i32 %inc39
  %4 = load i32* %arrayidx40, align 4
  %cmp41 = icmp slt i32 %iYes.0, %4
  %conv42 = zext i1 %cmp41 to i32
  %add43 = add nsw i32 %add38, %conv42
  %inc44 = add nsw i32 %iPixelCount.1, 2
  %arrayidx45 = getelementptr inbounds [9 x i32]* %iPixels, i32 0, i32 %inc44
  %5 = load i32* %arrayidx45, align 4
  %cmp46 = icmp slt i32 %iYes.0, %5
  %conv47 = zext i1 %cmp46 to i32
  %add48 = add nsw i32 %add43, %conv47
  %inc49 = add nsw i32 %iPixelCount.1, 3
  %inc51 = add nsw i32 %iRow30.0, 1
  br label %for.cond31

for.end52:                                        ; preds = %for.cond31
  %cmp53 = icmp sgt i32 %iHighCount.0, 4
  %cond = select i1 %cmp53, i32 %iYes.0, i32 %iMin.0
  %cmp55 = icmp slt i32 %iHighCount.0, 5
  %cond60 = select i1 %cmp55, i32 %iYes.0, i32 %iMax.0
  %add61 = add nsw i32 %cond60, %cond
  %shr = ashr i32 %add61, 1
  %inc63 = add nsw i32 %iSearch.0, 1
  br label %for.cond26

for.end64:                                        ; preds = %for.cond26
  %arrayidx65 = getelementptr inbounds [4 x i32]* %iResult, i32 0, i32 %ch.0
  store i32 %iYes.0, i32* %arrayidx65, align 4
  %inc67 = add nsw i32 %ch.0, 1
  br label %for.cond1

for.end68:                                        ; preds = %for.cond1
  %arrayidx69 = getelementptr inbounds [4 x i32]* %iResult, i32 0, i32 0
  %6 = load i32* %arrayidx69, align 4
  %and = and i32 %6, 255
  %arrayidx70 = getelementptr inbounds [4 x i32]* %iResult, i32 0, i32 1
  %7 = load i32* %arrayidx70, align 4
  %shl = shl i32 %7, 8
  %and71 = and i32 %shl, 65280
  %or = or i32 %and, %and71
  %arrayidx72 = getelementptr inbounds [4 x i32]* %iResult, i32 0, i32 2
  %8 = load i32* %arrayidx72, align 4
  %shl73 = shl i32 %8, 16
  %and74 = and i32 %shl73, 16711680
  %or75 = or i32 %or, %and74
  %add76 = add nsw i32 %call, 2
  %mul77 = mul nsw i32 %add76, %iImageWidth
  %add78 = add nsw i32 %mul77, %x.0
  %arrayidx79 = getelementptr inbounds i32 addrspace(1)* %pDst, i32 %add78
  store i32 %or75, i32 addrspace(1)* %arrayidx79, align 4
  %inc81 = add nsw i32 %x.0, 1
  br label %for.cond

for.end82:                                        ; preds = %for.cond
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
