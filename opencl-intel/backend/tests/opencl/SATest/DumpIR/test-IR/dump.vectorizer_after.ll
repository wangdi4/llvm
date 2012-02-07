; ModuleID = 'Program'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"
target triple = "i686-pc-win32"

define void @intel_median(<4 x i8> addrspace(1)* nocapture %pSrc, i32 addrspace(1)* nocapture %pDst, i32 %iImageWidth, i32 %iImageHeight) nounwind {
  %iPixels = alloca [9 x <4 x i32>], align 16
  %1 = icmp sgt i32 %iImageWidth, 0
  br i1 %1, label %.preheader.lr.ph, label %._crit_edge

.preheader.lr.ph:                                 ; preds = %0
  %2 = call i32 @get_global_id(i32 0) nounwind readnone
  %3 = add i32 %2, 2
  %4 = mul nsw i32 %3, %iImageWidth
  br label %.preheader

.preheader:                                       ; preds = %111, %.preheader.lr.ph
  %x.011 = phi i32 [ 0, %.preheader.lr.ph ], [ %124, %111 ]
  br label %5

; <label>:5                                       ; preds = %5, %.preheader
  %iRow.02 = phi i32 [ -1, %.preheader ], [ %53, %5 ]
  %iPixelCount.01 = phi i32 [ 0, %.preheader ], [ %52, %5 ]
  %6 = add i32 %3, %iRow.02
  %7 = mul nsw i32 %6, %iImageWidth
  %8 = add nsw i32 %7, %x.011
  %9 = add nsw i32 %8, -1
  %10 = getelementptr inbounds <4 x i8> addrspace(1)* %pSrc, i32 %9
  %11 = load <4 x i8> addrspace(1)* %10, align 4
  %12 = extractelement <4 x i8> %11, i32 0
  %13 = zext i8 %12 to i32
  %14 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 %iPixelCount.01
  %15 = load <4 x i32>* %14, align 16
  %16 = insertelement <4 x i32> %15, i32 %13, i32 0
  %17 = extractelement <4 x i8> %11, i32 1
  %18 = zext i8 %17 to i32
  %19 = insertelement <4 x i32> %16, i32 %18, i32 1
  %20 = extractelement <4 x i8> %11, i32 2
  %21 = zext i8 %20 to i32
  %22 = insertelement <4 x i32> %19, i32 %21, i32 2
  store <4 x i32> %22, <4 x i32>* %14, align 16
  %23 = add nsw i32 %iPixelCount.01, 1
  %24 = getelementptr inbounds <4 x i8> addrspace(1)* %pSrc, i32 %8
  %25 = load <4 x i8> addrspace(1)* %24, align 4
  %26 = extractelement <4 x i8> %25, i32 0
  %27 = zext i8 %26 to i32
  %28 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 %23
  %29 = load <4 x i32>* %28, align 16
  %30 = insertelement <4 x i32> %29, i32 %27, i32 0
  %31 = extractelement <4 x i8> %25, i32 1
  %32 = zext i8 %31 to i32
  %33 = insertelement <4 x i32> %30, i32 %32, i32 1
  %34 = extractelement <4 x i8> %25, i32 2
  %35 = zext i8 %34 to i32
  %36 = insertelement <4 x i32> %33, i32 %35, i32 2
  store <4 x i32> %36, <4 x i32>* %28, align 16
  %37 = add nsw i32 %iPixelCount.01, 2
  %38 = add nsw i32 %8, 1
  %39 = getelementptr inbounds <4 x i8> addrspace(1)* %pSrc, i32 %38
  %40 = load <4 x i8> addrspace(1)* %39, align 4
  %41 = extractelement <4 x i8> %40, i32 0
  %42 = zext i8 %41 to i32
  %43 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 %37
  %44 = load <4 x i32>* %43, align 16
  %45 = insertelement <4 x i32> %44, i32 %42, i32 0
  %46 = extractelement <4 x i8> %40, i32 1
  %47 = zext i8 %46 to i32
  %48 = insertelement <4 x i32> %45, i32 %47, i32 1
  %49 = extractelement <4 x i8> %40, i32 2
  %50 = zext i8 %49 to i32
  %51 = insertelement <4 x i32> %48, i32 %50, i32 2
  store <4 x i32> %51, <4 x i32>* %43, align 16
  %52 = add nsw i32 %iPixelCount.01, 3
  %53 = add nsw i32 %iRow.02, 1
  %exitcond = icmp eq i32 %53, 2
  br i1 %exitcond, label %.loopexit, label %5

.loopexit:                                        ; preds = %5
  %54 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 0
  br label %.preheader3

.preheader3:                                      ; preds = %.preheader3, %.loopexit
  %iSearch.010 = phi i32 [ 0, %.loopexit ], [ %110, %.preheader3 ]
  %iYes.09 = phi <4 x i32> [ <i32 128, i32 128, i32 128, i32 128>, %.loopexit ], [ %109, %.preheader3 ]
  %iMin.08 = phi <4 x i32> [ zeroinitializer, %.loopexit ], [ %104, %.preheader3 ]
  %iMax.07 = phi <4 x i32> [ <i32 255, i32 255, i32 255, i32 255>, %.loopexit ], [ %107, %.preheader3 ]
  %55 = load <4 x i32>* %54, align 16
  %56 = icmp slt <4 x i32> %iYes.09, %55
  %57 = sext <4 x i1> %56 to <4 x i32>
  %58 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 1
  %59 = load <4 x i32>* %58, align 16
  %60 = icmp slt <4 x i32> %iYes.09, %59
  %61 = sext <4 x i1> %60 to <4 x i32>
  %62 = add <4 x i32> %57, %61
  %63 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 2
  %64 = load <4 x i32>* %63, align 16
  %65 = icmp slt <4 x i32> %iYes.09, %64
  %66 = sext <4 x i1> %65 to <4 x i32>
  %67 = add <4 x i32> %62, %66
  %68 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 3
  %69 = load <4 x i32>* %68, align 16
  %70 = icmp slt <4 x i32> %iYes.09, %69
  %71 = sext <4 x i1> %70 to <4 x i32>
  %72 = add <4 x i32> %67, %71
  %73 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 4
  %74 = load <4 x i32>* %73, align 16
  %75 = icmp slt <4 x i32> %iYes.09, %74
  %76 = sext <4 x i1> %75 to <4 x i32>
  %77 = add <4 x i32> %72, %76
  %78 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 5
  %79 = load <4 x i32>* %78, align 16
  %80 = icmp slt <4 x i32> %iYes.09, %79
  %81 = sext <4 x i1> %80 to <4 x i32>
  %82 = add <4 x i32> %77, %81
  %83 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 6
  %84 = load <4 x i32>* %83, align 16
  %85 = icmp slt <4 x i32> %iYes.09, %84
  %86 = sext <4 x i1> %85 to <4 x i32>
  %87 = add <4 x i32> %82, %86
  %88 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 7
  %89 = load <4 x i32>* %88, align 16
  %90 = icmp slt <4 x i32> %iYes.09, %89
  %91 = sext <4 x i1> %90 to <4 x i32>
  %92 = add <4 x i32> %87, %91
  %93 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 8
  %94 = load <4 x i32>* %93, align 16
  %95 = icmp slt <4 x i32> %iYes.09, %94
  %96 = sext <4 x i1> %95 to <4 x i32>
  %97 = add <4 x i32> %92, %96
  %98 = sub <4 x i32> zeroinitializer, %97
  %99 = icmp sgt <4 x i32> %98, <i32 4, i32 4, i32 4, i32 4>
  %100 = sext <4 x i1> %99 to <4 x i32>
  %101 = and <4 x i32> %iYes.09, %100
  %102 = xor <4 x i32> %100, <i32 -1, i32 -1, i32 -1, i32 -1>
  %103 = and <4 x i32> %iMin.08, %102
  %104 = or <4 x i32> %101, %103
  %105 = and <4 x i32> %iYes.09, %102
  %106 = and <4 x i32> %iMax.07, %100
  %107 = or <4 x i32> %105, %106
  %108 = add <4 x i32> %107, %104
  %109 = ashr <4 x i32> %108, <i32 1, i32 1, i32 1, i32 1>
  %110 = add nsw i32 %iSearch.010, 1
  %exitcond13 = icmp eq i32 %110, 8
  br i1 %exitcond13, label %111, label %.preheader3

; <label>:111                                     ; preds = %.preheader3
  %112 = extractelement <4 x i32> %109, i32 0
  %113 = and i32 %112, 255
  %114 = extractelement <4 x i32> %109, i32 1
  %115 = shl i32 %114, 8
  %116 = and i32 %115, 65280
  %117 = extractelement <4 x i32> %109, i32 2
  %118 = shl i32 %117, 16
  %119 = and i32 %118, 16711680
  %120 = or i32 %119, %113
  %121 = or i32 %120, %116
  %122 = add nsw i32 %x.011, %4
  %123 = getelementptr inbounds i32 addrspace(1)* %pDst, i32 %122
  store i32 %121, i32 addrspace(1)* %123, align 4
  %124 = add nsw i32 %x.011, 1
  %exitcond14 = icmp eq i32 %124, %iImageWidth
  br i1 %exitcond14, label %._crit_edge.loopexit, label %.preheader

._crit_edge.loopexit:                             ; preds = %111
  br label %._crit_edge

._crit_edge:                                      ; preds = %._crit_edge.loopexit, %0
  ret void
}

declare i32 @get_global_id(i32) nounwind readnone

define void @intel_median_scalar(i8 addrspace(1)* nocapture %pSrc, i32 addrspace(1)* nocapture %pDst, i32 %iImageWidth, i32 %iImageHeight) nounwind {
  %iResult = alloca [4 x i32], align 4
  %iPixels = alloca [9 x i32], align 4
  %1 = call i32 @get_global_id(i32 0) nounwind readnone
  %2 = icmp sgt i32 %iImageWidth, 0
  br i1 %2, label %.preheader11.lr.ph, label %._crit_edge

.preheader11.lr.ph:                               ; preds = %0
  %3 = add i32 %1, 2
  %4 = getelementptr inbounds [4 x i32]* %iResult, i32 0, i32 0
  %5 = getelementptr inbounds [4 x i32]* %iResult, i32 0, i32 1
  %6 = getelementptr inbounds [4 x i32]* %iResult, i32 0, i32 2
  %7 = mul nsw i32 %3, %iImageWidth
  br label %.preheader11

.preheader11:                                     ; preds = %109, %.preheader11.lr.ph
  %x.013 = phi i32 [ 0, %.preheader11.lr.ph ], [ %122, %109 ]
  br label %.preheader

.preheader:                                       ; preds = %106, %.preheader11
  %ch.012 = phi i32 [ 0, %.preheader11 ], [ %108, %106 ]
  %8 = add i32 %ch.012, -4
  %9 = add i32 %ch.012, 4
  %10 = add i32 %1, 1
  %11 = mul nsw i32 %10, %iImageWidth
  %12 = add nsw i32 %11, %x.013
  %13 = shl i32 %12, 2
  %14 = add i32 %8, %13
  %15 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %14
  %16 = load i8 addrspace(1)* %15, align 1
  %17 = zext i8 %16 to i32
  %18 = getelementptr inbounds [9 x i32]* %iPixels, i32 0, i32 0
  store i32 %17, i32* %18, align 4
  %19 = add nsw i32 %13, %ch.012
  %20 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %19
  %21 = load i8 addrspace(1)* %20, align 1
  %22 = zext i8 %21 to i32
  %23 = getelementptr inbounds [9 x i32]* %iPixels, i32 0, i32 1
  store i32 %22, i32* %23, align 4
  %24 = add i32 %9, %13
  %25 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %24
  %26 = load i8 addrspace(1)* %25, align 1
  %27 = zext i8 %26 to i32
  %28 = getelementptr inbounds [9 x i32]* %iPixels, i32 0, i32 2
  store i32 %27, i32* %28, align 4
  %29 = add nsw i32 %7, %x.013
  %30 = shl i32 %29, 2
  %31 = add i32 %8, %30
  %32 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %31
  %33 = load i8 addrspace(1)* %32, align 1
  %34 = zext i8 %33 to i32
  %35 = getelementptr inbounds [9 x i32]* %iPixels, i32 0, i32 3
  store i32 %34, i32* %35, align 4
  %36 = add nsw i32 %30, %ch.012
  %37 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %36
  %38 = load i8 addrspace(1)* %37, align 1
  %39 = zext i8 %38 to i32
  %40 = getelementptr inbounds [9 x i32]* %iPixels, i32 0, i32 4
  store i32 %39, i32* %40, align 4
  %41 = add i32 %9, %30
  %42 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %41
  %43 = load i8 addrspace(1)* %42, align 1
  %44 = zext i8 %43 to i32
  %45 = getelementptr inbounds [9 x i32]* %iPixels, i32 0, i32 5
  store i32 %44, i32* %45, align 4
  %46 = add i32 %1, 3
  %47 = mul nsw i32 %46, %iImageWidth
  %48 = add nsw i32 %47, %x.013
  %49 = shl i32 %48, 2
  %50 = add i32 %8, %49
  %51 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %50
  %52 = load i8 addrspace(1)* %51, align 1
  %53 = zext i8 %52 to i32
  %54 = getelementptr inbounds [9 x i32]* %iPixels, i32 0, i32 6
  store i32 %53, i32* %54, align 4
  %55 = add nsw i32 %49, %ch.012
  %56 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %55
  %57 = load i8 addrspace(1)* %56, align 1
  %58 = zext i8 %57 to i32
  %59 = getelementptr inbounds [9 x i32]* %iPixels, i32 0, i32 7
  store i32 %58, i32* %59, align 4
  %60 = add i32 %9, %49
  %61 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %60
  %62 = load i8 addrspace(1)* %61, align 1
  %63 = zext i8 %62 to i32
  %64 = getelementptr inbounds [9 x i32]* %iPixels, i32 0, i32 8
  store i32 %63, i32* %64, align 4
  br label %.preheader3

.preheader3:                                      ; preds = %.preheader3, %.preheader
  %iSearch.010 = phi i32 [ 0, %.preheader ], [ %105, %.preheader3 ]
  %iMax.09 = phi i32 [ 255, %.preheader ], [ %102, %.preheader3 ]
  %iMin.08 = phi i32 [ 0, %.preheader ], [ %100, %.preheader3 ]
  %iYes.07 = phi i32 [ 128, %.preheader ], [ %104, %.preheader3 ]
  %65 = load i32* %18, align 4
  %66 = icmp slt i32 %iYes.07, %65
  %67 = zext i1 %66 to i32
  %68 = load i32* %23, align 4
  %69 = icmp slt i32 %iYes.07, %68
  %70 = zext i1 %69 to i32
  %71 = load i32* %28, align 4
  %72 = icmp slt i32 %iYes.07, %71
  %73 = zext i1 %72 to i32
  %74 = add i32 %67, %70
  %75 = add i32 %74, %73
  %76 = load i32* %35, align 4
  %77 = icmp slt i32 %iYes.07, %76
  %78 = zext i1 %77 to i32
  %79 = load i32* %40, align 4
  %80 = icmp slt i32 %iYes.07, %79
  %81 = zext i1 %80 to i32
  %82 = load i32* %45, align 4
  %83 = icmp slt i32 %iYes.07, %82
  %84 = zext i1 %83 to i32
  %85 = add i32 %78, %75
  %86 = add i32 %85, %81
  %87 = add i32 %86, %84
  %88 = load i32* %54, align 4
  %89 = icmp slt i32 %iYes.07, %88
  %90 = zext i1 %89 to i32
  %91 = load i32* %59, align 4
  %92 = icmp slt i32 %iYes.07, %91
  %93 = zext i1 %92 to i32
  %94 = icmp slt i32 %iYes.07, %63
  %95 = zext i1 %94 to i32
  %96 = add i32 %90, %87
  %97 = add i32 %96, %93
  %98 = add i32 %97, %95
  %99 = icmp sgt i32 %98, 4
  %100 = select i1 %99, i32 %iYes.07, i32 %iMin.08
  %101 = icmp slt i32 %98, 5
  %102 = select i1 %101, i32 %iYes.07, i32 %iMax.09
  %103 = add nsw i32 %102, %100
  %104 = ashr i32 %103, 1
  %105 = add nsw i32 %iSearch.010, 1
  %exitcond = icmp eq i32 %105, 8
  br i1 %exitcond, label %106, label %.preheader3

; <label>:106                                     ; preds = %.preheader3
  %107 = getelementptr inbounds [4 x i32]* %iResult, i32 0, i32 %ch.012
  store i32 %104, i32* %107, align 4
  %108 = add nsw i32 %ch.012, 1
  %exitcond14 = icmp eq i32 %108, 3
  br i1 %exitcond14, label %109, label %.preheader

; <label>:109                                     ; preds = %106
  %110 = load i32* %4, align 4
  %111 = and i32 %110, 255
  %112 = load i32* %5, align 4
  %113 = shl i32 %112, 8
  %114 = and i32 %113, 65280
  %115 = load i32* %6, align 4
  %116 = shl i32 %115, 16
  %117 = and i32 %116, 16711680
  %118 = or i32 %114, %111
  %119 = or i32 %118, %117
  %120 = add nsw i32 %x.013, %7
  %121 = getelementptr inbounds i32 addrspace(1)* %pDst, i32 %120
  store i32 %119, i32 addrspace(1)* %121, align 4
  %122 = add nsw i32 %x.013, 1
  %exitcond15 = icmp eq i32 %122, %iImageWidth
  br i1 %exitcond15, label %._crit_edge.loopexit, label %.preheader11

._crit_edge.loopexit:                             ; preds = %109
  br label %._crit_edge

._crit_edge:                                      ; preds = %._crit_edge.loopexit, %0
  ret void
}

define void @__Vectorized_.intel_median_scalar(i8 addrspace(1)* nocapture %pSrc, i32 addrspace(1)* nocapture %pDst, i32 %iImageWidth, i32 %iImageHeight) nounwind {
  %temp = insertelement <4 x i32> undef, i32 %iImageWidth, i32 0
  %vector = shufflevector <4 x i32> %temp, <4 x i32> undef, <4 x i32> zeroinitializer
  %1 = alloca [4 x i32], align 4
  %2 = alloca [4 x i32], align 4
  %3 = alloca [4 x i32], align 4
  %4 = alloca [4 x i32], align 4
  %5 = call i32 @get_global_id(i32 0) nounwind readnone
  %broadcast1 = insertelement <4 x i32> undef, i32 %5, i32 0
  %broadcast2 = shufflevector <4 x i32> %broadcast1, <4 x i32> undef, <4 x i32> zeroinitializer
  %6 = icmp sgt i32 %iImageWidth, 0
  br i1 %6, label %.preheader11.lr.ph, label %._crit_edge

.preheader11.lr.ph:                               ; preds = %0
  %7 = add <4 x i32> %broadcast2, <i32 2, i32 3, i32 4, i32 5>
  %8 = getelementptr inbounds [4 x i32]* %1, i32 0, i32 0
  %9 = getelementptr inbounds [4 x i32]* %2, i32 0, i32 0
  %10 = getelementptr inbounds [4 x i32]* %3, i32 0, i32 0
  %11 = getelementptr inbounds [4 x i32]* %4, i32 0, i32 0
  %12 = getelementptr inbounds [4 x i32]* %1, i32 0, i32 1
  %13 = getelementptr inbounds [4 x i32]* %2, i32 0, i32 1
  %14 = getelementptr inbounds [4 x i32]* %3, i32 0, i32 1
  %15 = getelementptr inbounds [4 x i32]* %4, i32 0, i32 1
  %16 = getelementptr inbounds [4 x i32]* %1, i32 0, i32 2
  %17 = getelementptr inbounds [4 x i32]* %2, i32 0, i32 2
  %18 = getelementptr inbounds [4 x i32]* %3, i32 0, i32 2
  %19 = getelementptr inbounds [4 x i32]* %4, i32 0, i32 2
  %20 = mul nsw <4 x i32> %7, %vector
  br label %.preheader11

.preheader11:                                     ; preds = %161, %.preheader11.lr.ph
  %x.013 = phi i32 [ 0, %.preheader11.lr.ph ], [ %186, %161 ]
  %temp25 = insertelement <4 x i32> undef, i32 %x.013, i32 0
  %vector26 = shufflevector <4 x i32> %temp25, <4 x i32> undef, <4 x i32> zeroinitializer
  br label %.preheader

.preheader:                                       ; preds = %155, %.preheader11
  %ch.012 = phi i32 [ 0, %.preheader11 ], [ %160, %155 ]
  %temp32 = insertelement <4 x i32> undef, i32 %ch.012, i32 0
  %vector33 = shufflevector <4 x i32> %temp32, <4 x i32> undef, <4 x i32> zeroinitializer
  %21 = add i32 %ch.012, -4
  %temp27 = insertelement <4 x i32> undef, i32 %21, i32 0
  %vector28 = shufflevector <4 x i32> %temp27, <4 x i32> undef, <4 x i32> zeroinitializer
  %22 = add i32 %ch.012, 4
  %temp38 = insertelement <4 x i32> undef, i32 %22, i32 0
  %vector39 = shufflevector <4 x i32> %temp38, <4 x i32> undef, <4 x i32> zeroinitializer
  %23 = add <4 x i32> %broadcast2, <i32 1, i32 2, i32 3, i32 4>
  %24 = mul nsw <4 x i32> %23, %vector
  %25 = add nsw <4 x i32> %24, %vector26
  %26 = shl <4 x i32> %25, <i32 2, i32 2, i32 2, i32 2>
  %27 = add <4 x i32> %vector28, %26
  %extract = extractelement <4 x i32> %27, i32 0
  %extract29 = extractelement <4 x i32> %27, i32 1
  %extract30 = extractelement <4 x i32> %27, i32 2
  %extract31 = extractelement <4 x i32> %27, i32 3
  %28 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract
  %29 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract29
  %30 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract30
  %31 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract31
  %32 = load i8 addrspace(1)* %28, align 1
  %33 = load i8 addrspace(1)* %29, align 1
  %34 = load i8 addrspace(1)* %30, align 1
  %35 = load i8 addrspace(1)* %31, align 1
  %temp.vect73 = insertelement <4 x i8> undef, i8 %32, i32 0
  %temp.vect74 = insertelement <4 x i8> %temp.vect73, i8 %33, i32 1
  %temp.vect75 = insertelement <4 x i8> %temp.vect74, i8 %34, i32 2
  %temp.vect76 = insertelement <4 x i8> %temp.vect75, i8 %35, i32 3
  %36 = add nsw <4 x i32> %26, %vector33
  %extract34 = extractelement <4 x i32> %36, i32 0
  %extract35 = extractelement <4 x i32> %36, i32 1
  %extract36 = extractelement <4 x i32> %36, i32 2
  %extract37 = extractelement <4 x i32> %36, i32 3
  %37 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract34
  %38 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract35
  %39 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract36
  %40 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract37
  %41 = load i8 addrspace(1)* %37, align 1
  %42 = load i8 addrspace(1)* %38, align 1
  %43 = load i8 addrspace(1)* %39, align 1
  %44 = load i8 addrspace(1)* %40, align 1
  %temp.vect77 = insertelement <4 x i8> undef, i8 %41, i32 0
  %temp.vect78 = insertelement <4 x i8> %temp.vect77, i8 %42, i32 1
  %temp.vect79 = insertelement <4 x i8> %temp.vect78, i8 %43, i32 2
  %temp.vect80 = insertelement <4 x i8> %temp.vect79, i8 %44, i32 3
  %45 = add <4 x i32> %vector39, %26
  %extract40 = extractelement <4 x i32> %45, i32 0
  %extract41 = extractelement <4 x i32> %45, i32 1
  %extract42 = extractelement <4 x i32> %45, i32 2
  %extract43 = extractelement <4 x i32> %45, i32 3
  %46 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract40
  %47 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract41
  %48 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract42
  %49 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract43
  %50 = load i8 addrspace(1)* %46, align 1
  %51 = load i8 addrspace(1)* %47, align 1
  %52 = load i8 addrspace(1)* %48, align 1
  %53 = load i8 addrspace(1)* %49, align 1
  %temp.vect81 = insertelement <4 x i8> undef, i8 %50, i32 0
  %temp.vect82 = insertelement <4 x i8> %temp.vect81, i8 %51, i32 1
  %temp.vect83 = insertelement <4 x i8> %temp.vect82, i8 %52, i32 2
  %temp.vect84 = insertelement <4 x i8> %temp.vect83, i8 %53, i32 3
  %54 = add nsw <4 x i32> %20, %vector26
  %55 = shl <4 x i32> %54, <i32 2, i32 2, i32 2, i32 2>
  %56 = add <4 x i32> %vector28, %55
  %extract44 = extractelement <4 x i32> %56, i32 0
  %extract45 = extractelement <4 x i32> %56, i32 1
  %extract46 = extractelement <4 x i32> %56, i32 2
  %extract47 = extractelement <4 x i32> %56, i32 3
  %57 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract44
  %58 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract45
  %59 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract46
  %60 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract47
  %61 = load i8 addrspace(1)* %57, align 1
  %62 = load i8 addrspace(1)* %58, align 1
  %63 = load i8 addrspace(1)* %59, align 1
  %64 = load i8 addrspace(1)* %60, align 1
  %temp.vect85 = insertelement <4 x i8> undef, i8 %61, i32 0
  %temp.vect86 = insertelement <4 x i8> %temp.vect85, i8 %62, i32 1
  %temp.vect87 = insertelement <4 x i8> %temp.vect86, i8 %63, i32 2
  %temp.vect88 = insertelement <4 x i8> %temp.vect87, i8 %64, i32 3
  %65 = add nsw <4 x i32> %55, %vector33
  %extract48 = extractelement <4 x i32> %65, i32 0
  %extract49 = extractelement <4 x i32> %65, i32 1
  %extract50 = extractelement <4 x i32> %65, i32 2
  %extract51 = extractelement <4 x i32> %65, i32 3
  %66 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract48
  %67 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract49
  %68 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract50
  %69 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract51
  %70 = load i8 addrspace(1)* %66, align 1
  %71 = load i8 addrspace(1)* %67, align 1
  %72 = load i8 addrspace(1)* %68, align 1
  %73 = load i8 addrspace(1)* %69, align 1
  %temp.vect89 = insertelement <4 x i8> undef, i8 %70, i32 0
  %temp.vect90 = insertelement <4 x i8> %temp.vect89, i8 %71, i32 1
  %temp.vect91 = insertelement <4 x i8> %temp.vect90, i8 %72, i32 2
  %temp.vect92 = insertelement <4 x i8> %temp.vect91, i8 %73, i32 3
  %74 = add <4 x i32> %vector39, %55
  %extract52 = extractelement <4 x i32> %74, i32 0
  %extract53 = extractelement <4 x i32> %74, i32 1
  %extract54 = extractelement <4 x i32> %74, i32 2
  %extract55 = extractelement <4 x i32> %74, i32 3
  %75 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract52
  %76 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract53
  %77 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract54
  %78 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract55
  %79 = load i8 addrspace(1)* %75, align 1
  %80 = load i8 addrspace(1)* %76, align 1
  %81 = load i8 addrspace(1)* %77, align 1
  %82 = load i8 addrspace(1)* %78, align 1
  %temp.vect93 = insertelement <4 x i8> undef, i8 %79, i32 0
  %temp.vect94 = insertelement <4 x i8> %temp.vect93, i8 %80, i32 1
  %temp.vect95 = insertelement <4 x i8> %temp.vect94, i8 %81, i32 2
  %temp.vect96 = insertelement <4 x i8> %temp.vect95, i8 %82, i32 3
  %83 = add <4 x i32> %broadcast2, <i32 3, i32 4, i32 5, i32 6>
  %84 = mul nsw <4 x i32> %83, %vector
  %85 = add nsw <4 x i32> %84, %vector26
  %86 = shl <4 x i32> %85, <i32 2, i32 2, i32 2, i32 2>
  %87 = add <4 x i32> %vector28, %86
  %extract56 = extractelement <4 x i32> %87, i32 0
  %extract57 = extractelement <4 x i32> %87, i32 1
  %extract58 = extractelement <4 x i32> %87, i32 2
  %extract59 = extractelement <4 x i32> %87, i32 3
  %88 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract56
  %89 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract57
  %90 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract58
  %91 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract59
  %92 = load i8 addrspace(1)* %88, align 1
  %93 = load i8 addrspace(1)* %89, align 1
  %94 = load i8 addrspace(1)* %90, align 1
  %95 = load i8 addrspace(1)* %91, align 1
  %temp.vect97 = insertelement <4 x i8> undef, i8 %92, i32 0
  %temp.vect98 = insertelement <4 x i8> %temp.vect97, i8 %93, i32 1
  %temp.vect99 = insertelement <4 x i8> %temp.vect98, i8 %94, i32 2
  %temp.vect100 = insertelement <4 x i8> %temp.vect99, i8 %95, i32 3
  %96 = add nsw <4 x i32> %86, %vector33
  %extract60 = extractelement <4 x i32> %96, i32 0
  %extract61 = extractelement <4 x i32> %96, i32 1
  %extract62 = extractelement <4 x i32> %96, i32 2
  %extract63 = extractelement <4 x i32> %96, i32 3
  %97 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract60
  %98 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract61
  %99 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract62
  %100 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract63
  %101 = load i8 addrspace(1)* %97, align 1
  %102 = load i8 addrspace(1)* %98, align 1
  %103 = load i8 addrspace(1)* %99, align 1
  %104 = load i8 addrspace(1)* %100, align 1
  %temp.vect101 = insertelement <4 x i8> undef, i8 %101, i32 0
  %temp.vect102 = insertelement <4 x i8> %temp.vect101, i8 %102, i32 1
  %temp.vect103 = insertelement <4 x i8> %temp.vect102, i8 %103, i32 2
  %temp.vect104 = insertelement <4 x i8> %temp.vect103, i8 %104, i32 3
  %105 = add <4 x i32> %vector39, %86
  %extract64 = extractelement <4 x i32> %105, i32 0
  %extract65 = extractelement <4 x i32> %105, i32 1
  %extract66 = extractelement <4 x i32> %105, i32 2
  %extract67 = extractelement <4 x i32> %105, i32 3
  %106 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract64
  %107 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract65
  %108 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract66
  %109 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract67
  %110 = load i8 addrspace(1)* %106, align 1
  %111 = load i8 addrspace(1)* %107, align 1
  %112 = load i8 addrspace(1)* %108, align 1
  %113 = load i8 addrspace(1)* %109, align 1
  %temp.vect = insertelement <4 x i8> undef, i8 %110, i32 0
  %temp.vect68 = insertelement <4 x i8> %temp.vect, i8 %111, i32 1
  %temp.vect69 = insertelement <4 x i8> %temp.vect68, i8 %112, i32 2
  %temp.vect70 = insertelement <4 x i8> %temp.vect69, i8 %113, i32 3
  %114 = zext <4 x i8> %temp.vect70 to <4 x i32>
  br label %.preheader3

.preheader3:                                      ; preds = %.preheader3, %.preheader
  %iSearch.010 = phi i32 [ 0, %.preheader ], [ %154, %.preheader3 ]
  %vectorPHI = phi <4 x i32> [ <i32 255, i32 255, i32 255, i32 255>, %.preheader ], [ %151, %.preheader3 ]
  %vectorPHI71 = phi <4 x i32> [ zeroinitializer, %.preheader ], [ %145, %.preheader3 ]
  %vectorPHI72 = phi <4 x i32> [ <i32 128, i32 128, i32 128, i32 128>, %.preheader ], [ %153, %.preheader3 ]
  %115 = zext <4 x i8> %temp.vect76 to <4 x i32>
  %116 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %115, <4 x i32> %vectorPHI72)
  %msb = and <4 x i32> %116, <i32 1, i32 1, i32 1, i32 1>
  %117 = zext <4 x i8> %temp.vect80 to <4 x i32>
  %118 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %117, <4 x i32> %vectorPHI72)
  %msb144 = and <4 x i32> %118, <i32 1, i32 1, i32 1, i32 1>
  %119 = zext <4 x i8> %temp.vect84 to <4 x i32>
  %120 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %119, <4 x i32> %vectorPHI72)
  %msb146 = and <4 x i32> %120, <i32 1, i32 1, i32 1, i32 1>
  %121 = add <4 x i32> %msb, %msb144
  %122 = add <4 x i32> %121, %msb146
  %123 = zext <4 x i8> %temp.vect88 to <4 x i32>
  %124 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %123, <4 x i32> %vectorPHI72)
  %msb148 = and <4 x i32> %124, <i32 1, i32 1, i32 1, i32 1>
  %125 = zext <4 x i8> %temp.vect92 to <4 x i32>
  %126 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %125, <4 x i32> %vectorPHI72)
  %msb150 = and <4 x i32> %126, <i32 1, i32 1, i32 1, i32 1>
  %127 = zext <4 x i8> %temp.vect96 to <4 x i32>
  %128 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %127, <4 x i32> %vectorPHI72)
  %msb152 = and <4 x i32> %128, <i32 1, i32 1, i32 1, i32 1>
  %129 = add <4 x i32> %msb148, %122
  %130 = add <4 x i32> %129, %msb150
  %131 = add <4 x i32> %130, %msb152
  %132 = zext <4 x i8> %temp.vect100 to <4 x i32>
  %133 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %132, <4 x i32> %vectorPHI72)
  %msb154 = and <4 x i32> %133, <i32 1, i32 1, i32 1, i32 1>
  %134 = zext <4 x i8> %temp.vect104 to <4 x i32>
  %135 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %134, <4 x i32> %vectorPHI72)
  %msb156 = and <4 x i32> %135, <i32 1, i32 1, i32 1, i32 1>
  %136 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %114, <4 x i32> %vectorPHI72)
  %msb158 = and <4 x i32> %136, <i32 1, i32 1, i32 1, i32 1>
  %137 = add <4 x i32> %msb154, %131
  %138 = add <4 x i32> %137, %msb156
  %139 = add <4 x i32> %138, %msb158
  %140 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %139, <4 x i32> <i32 4, i32 4, i32 4, i32 4>)
  %141 = bitcast <4 x i32> %vectorPHI72 to <4 x float>
  %142 = bitcast <4 x i32> %vectorPHI71 to <4 x float>
  %143 = bitcast <4 x i32> %140 to <4 x float>
  %144 = call <4 x float> @llvm.x86.sse41.blendvps(<4 x float> %142, <4 x float> %141, <4 x float> %143)
  %145 = bitcast <4 x float> %144 to <4 x i32>
  %146 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> <i32 5, i32 5, i32 5, i32 5>, <4 x i32> %139)
  %147 = bitcast <4 x i32> %vectorPHI72 to <4 x float>
  %148 = bitcast <4 x i32> %vectorPHI to <4 x float>
  %149 = bitcast <4 x i32> %146 to <4 x float>
  %150 = call <4 x float> @llvm.x86.sse41.blendvps(<4 x float> %148, <4 x float> %147, <4 x float> %149)
  %151 = bitcast <4 x float> %150 to <4 x i32>
  %152 = add nsw <4 x i32> %151, %145
  %153 = ashr <4 x i32> %152, <i32 1, i32 1, i32 1, i32 1>
  %154 = add nsw i32 %iSearch.010, 1
  %exitcond = icmp eq i32 %154, 8
  br i1 %exitcond, label %155, label %.preheader3

; <label>:155                                     ; preds = %.preheader3
  %extract108 = extractelement <4 x i32> %153, i32 3
  %extract107 = extractelement <4 x i32> %153, i32 2
  %extract106 = extractelement <4 x i32> %153, i32 1
  %extract105 = extractelement <4 x i32> %153, i32 0
  %156 = getelementptr inbounds [4 x i32]* %1, i32 0, i32 %ch.012
  %157 = getelementptr inbounds [4 x i32]* %2, i32 0, i32 %ch.012
  %158 = getelementptr inbounds [4 x i32]* %3, i32 0, i32 %ch.012
  %159 = getelementptr inbounds [4 x i32]* %4, i32 0, i32 %ch.012
  store i32 %extract105, i32* %156, align 4
  store i32 %extract106, i32* %157, align 4
  store i32 %extract107, i32* %158, align 4
  store i32 %extract108, i32* %159, align 4
  %160 = add nsw i32 %ch.012, 1
  %exitcond14 = icmp eq i32 %160, 3
  br i1 %exitcond14, label %161, label %.preheader

; <label>:161                                     ; preds = %155
  %162 = load i32* %8, align 4
  %163 = load i32* %9, align 4
  %164 = load i32* %10, align 4
  %165 = load i32* %11, align 4
  %temp.vect109 = insertelement <4 x i32> undef, i32 %162, i32 0
  %temp.vect110 = insertelement <4 x i32> %temp.vect109, i32 %163, i32 1
  %temp.vect111 = insertelement <4 x i32> %temp.vect110, i32 %164, i32 2
  %temp.vect112 = insertelement <4 x i32> %temp.vect111, i32 %165, i32 3
  %166 = and <4 x i32> %temp.vect112, <i32 255, i32 255, i32 255, i32 255>
  %167 = load i32* %12, align 4
  %168 = load i32* %13, align 4
  %169 = load i32* %14, align 4
  %170 = load i32* %15, align 4
  %temp.vect113 = insertelement <4 x i32> undef, i32 %167, i32 0
  %temp.vect114 = insertelement <4 x i32> %temp.vect113, i32 %168, i32 1
  %temp.vect115 = insertelement <4 x i32> %temp.vect114, i32 %169, i32 2
  %temp.vect116 = insertelement <4 x i32> %temp.vect115, i32 %170, i32 3
  %171 = shl <4 x i32> %temp.vect116, <i32 8, i32 8, i32 8, i32 8>
  %172 = and <4 x i32> %171, <i32 65280, i32 65280, i32 65280, i32 65280>
  %173 = load i32* %16, align 4
  %174 = load i32* %17, align 4
  %175 = load i32* %18, align 4
  %176 = load i32* %19, align 4
  %temp.vect117 = insertelement <4 x i32> undef, i32 %173, i32 0
  %temp.vect118 = insertelement <4 x i32> %temp.vect117, i32 %174, i32 1
  %temp.vect119 = insertelement <4 x i32> %temp.vect118, i32 %175, i32 2
  %temp.vect120 = insertelement <4 x i32> %temp.vect119, i32 %176, i32 3
  %177 = shl <4 x i32> %temp.vect120, <i32 16, i32 16, i32 16, i32 16>
  %178 = and <4 x i32> %177, <i32 16711680, i32 16711680, i32 16711680, i32 16711680>
  %179 = or <4 x i32> %172, %166
  %180 = or <4 x i32> %179, %178
  %extract125 = extractelement <4 x i32> %180, i32 0
  %extract126 = extractelement <4 x i32> %180, i32 1
  %extract127 = extractelement <4 x i32> %180, i32 2
  %extract128 = extractelement <4 x i32> %180, i32 3
  %181 = add nsw <4 x i32> %vector26, %20
  %extract121 = extractelement <4 x i32> %181, i32 0
  %extract122 = extractelement <4 x i32> %181, i32 1
  %extract123 = extractelement <4 x i32> %181, i32 2
  %extract124 = extractelement <4 x i32> %181, i32 3
  %182 = getelementptr inbounds i32 addrspace(1)* %pDst, i32 %extract121
  %183 = getelementptr inbounds i32 addrspace(1)* %pDst, i32 %extract122
  %184 = getelementptr inbounds i32 addrspace(1)* %pDst, i32 %extract123
  %185 = getelementptr inbounds i32 addrspace(1)* %pDst, i32 %extract124
  store i32 %extract125, i32 addrspace(1)* %182, align 4
  store i32 %extract126, i32 addrspace(1)* %183, align 4
  store i32 %extract127, i32 addrspace(1)* %184, align 4
  store i32 %extract128, i32 addrspace(1)* %185, align 4
  %186 = add nsw i32 %x.013, 1
  %exitcond15 = icmp eq i32 %186, %iImageWidth
  br i1 %exitcond15, label %._crit_edge, label %.preheader11

._crit_edge:                                      ; preds = %161, %0
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
