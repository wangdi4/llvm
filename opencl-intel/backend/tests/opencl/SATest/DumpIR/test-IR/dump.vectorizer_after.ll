; ModuleID = 'Program'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"
target triple = "i686-pc-win32"

define void @intel_median(<4 x i8> addrspace(1)* nocapture %pSrc, i32 addrspace(1)* nocapture %pDst, i32 %iImageWidth, i32 %iImageHeight) nounwind {
.preheader.lr.ph:
  %iPixels = alloca [9 x <4 x i32>], align 16
  %0 = call i32 @get_global_id(i32 0) nounwind readnone
  %1 = add i32 %0, 2
  %2 = mul nsw i32 %1, %iImageWidth
  br label %.preheader

.preheader:                                       ; preds = %93, %.preheader.lr.ph
  %3 = phi <4 x i32> [ undef, %.preheader.lr.ph ], [ %.pre, %93 ]
  %x.011 = phi i32 [ 0, %.preheader.lr.ph ], [ %106, %93 ]
  br label %4

; <label>:4                                       ; preds = %._crit_edge30, %.preheader
  %5 = phi <4 x i32> [ %3, %.preheader ], [ %.pre32, %._crit_edge30 ]
  %iRow.02 = phi i32 [ -1, %.preheader ], [ %52, %._crit_edge30 ]
  %iPixelCount.01 = phi i32 [ 0, %.preheader ], [ %51, %._crit_edge30 ]
  %6 = add i32 %1, %iRow.02
  %7 = mul nsw i32 %6, %iImageWidth
  %8 = add nsw i32 %7, %x.011
  %9 = add nsw i32 %8, -1
  %10 = getelementptr inbounds <4 x i8> addrspace(1)* %pSrc, i32 %9
  %11 = load <4 x i8> addrspace(1)* %10, align 4
  %12 = extractelement <4 x i8> %11, i32 0
  %13 = zext i8 %12 to i32
  %14 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 %iPixelCount.01
  %15 = insertelement <4 x i32> %5, i32 %13, i32 0
  %16 = extractelement <4 x i8> %11, i32 1
  %17 = zext i8 %16 to i32
  %18 = insertelement <4 x i32> %15, i32 %17, i32 1
  %19 = extractelement <4 x i8> %11, i32 2
  %20 = zext i8 %19 to i32
  %21 = insertelement <4 x i32> %18, i32 %20, i32 2
  store <4 x i32> %21, <4 x i32>* %14, align 16
  %22 = add nsw i32 %iPixelCount.01, 1
  %23 = getelementptr inbounds <4 x i8> addrspace(1)* %pSrc, i32 %8
  %24 = load <4 x i8> addrspace(1)* %23, align 4
  %25 = extractelement <4 x i8> %24, i32 0
  %26 = zext i8 %25 to i32
  %27 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 %22
  %28 = load <4 x i32>* %27, align 16
  %29 = insertelement <4 x i32> %28, i32 %26, i32 0
  %30 = extractelement <4 x i8> %24, i32 1
  %31 = zext i8 %30 to i32
  %32 = insertelement <4 x i32> %29, i32 %31, i32 1
  %33 = extractelement <4 x i8> %24, i32 2
  %34 = zext i8 %33 to i32
  %35 = insertelement <4 x i32> %32, i32 %34, i32 2
  store <4 x i32> %35, <4 x i32>* %27, align 16
  %36 = add nsw i32 %iPixelCount.01, 2
  %37 = add nsw i32 %8, 1
  %38 = getelementptr inbounds <4 x i8> addrspace(1)* %pSrc, i32 %37
  %39 = load <4 x i8> addrspace(1)* %38, align 4
  %40 = extractelement <4 x i8> %39, i32 0
  %41 = zext i8 %40 to i32
  %42 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 %36
  %43 = load <4 x i32>* %42, align 16
  %44 = insertelement <4 x i32> %43, i32 %41, i32 0
  %45 = extractelement <4 x i8> %39, i32 1
  %46 = zext i8 %45 to i32
  %47 = insertelement <4 x i32> %44, i32 %46, i32 1
  %48 = extractelement <4 x i8> %39, i32 2
  %49 = zext i8 %48 to i32
  %50 = insertelement <4 x i32> %47, i32 %49, i32 2
  store <4 x i32> %50, <4 x i32>* %42, align 16
  %51 = add nsw i32 %iPixelCount.01, 3
  %52 = add nsw i32 %iRow.02, 1
  %exitcond = icmp eq i32 %52, 2
  br i1 %exitcond, label %.loopexit, label %._crit_edge30

._crit_edge30:                                    ; preds = %4
  %.phi.trans.insert31 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 %51
  %.pre32 = load <4 x i32>* %.phi.trans.insert31, align 16
  br label %4

.loopexit:                                        ; preds = %4
  %53 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 0
  %.pre = load <4 x i32>* %53, align 16
  %.phi.trans.insert = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 1
  %.pre15 = load <4 x i32>* %.phi.trans.insert, align 16
  %.phi.trans.insert16 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 2
  %.pre17 = load <4 x i32>* %.phi.trans.insert16, align 16
  %.phi.trans.insert18 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 3
  %.pre19 = load <4 x i32>* %.phi.trans.insert18, align 16
  %.phi.trans.insert20 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 4
  %.pre21 = load <4 x i32>* %.phi.trans.insert20, align 16
  %.phi.trans.insert22 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 5
  %.pre23 = load <4 x i32>* %.phi.trans.insert22, align 16
  %.phi.trans.insert24 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 6
  %.pre25 = load <4 x i32>* %.phi.trans.insert24, align 16
  %.phi.trans.insert26 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 7
  %.pre27 = load <4 x i32>* %.phi.trans.insert26, align 16
  %.phi.trans.insert28 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 8
  %.pre29 = load <4 x i32>* %.phi.trans.insert28, align 16
  br label %.preheader3

.preheader3:                                      ; preds = %.preheader3, %.loopexit
  %iSearch.010 = phi i32 [ 0, %.loopexit ], [ %92, %.preheader3 ]
  %iYes.09 = phi <4 x i32> [ <i32 128, i32 128, i32 128, i32 128>, %.loopexit ], [ %91, %.preheader3 ]
  %iMin.08 = phi <4 x i32> [ zeroinitializer, %.loopexit ], [ %86, %.preheader3 ]
  %iMax.07 = phi <4 x i32> [ <i32 255, i32 255, i32 255, i32 255>, %.loopexit ], [ %89, %.preheader3 ]
  %54 = icmp slt <4 x i32> %iYes.09, %.pre
  %55 = sext <4 x i1> %54 to <4 x i32>
  %56 = icmp slt <4 x i32> %iYes.09, %.pre15
  %57 = sext <4 x i1> %56 to <4 x i32>
  %58 = add <4 x i32> %55, %57
  %59 = icmp slt <4 x i32> %iYes.09, %.pre17
  %60 = sext <4 x i1> %59 to <4 x i32>
  %61 = add <4 x i32> %58, %60
  %62 = icmp slt <4 x i32> %iYes.09, %.pre19
  %63 = sext <4 x i1> %62 to <4 x i32>
  %64 = add <4 x i32> %61, %63
  %65 = icmp slt <4 x i32> %iYes.09, %.pre21
  %66 = sext <4 x i1> %65 to <4 x i32>
  %67 = add <4 x i32> %64, %66
  %68 = icmp slt <4 x i32> %iYes.09, %.pre23
  %69 = sext <4 x i1> %68 to <4 x i32>
  %70 = add <4 x i32> %67, %69
  %71 = icmp slt <4 x i32> %iYes.09, %.pre25
  %72 = sext <4 x i1> %71 to <4 x i32>
  %73 = add <4 x i32> %70, %72
  %74 = icmp slt <4 x i32> %iYes.09, %.pre27
  %75 = sext <4 x i1> %74 to <4 x i32>
  %76 = add <4 x i32> %73, %75
  %77 = icmp slt <4 x i32> %iYes.09, %.pre29
  %78 = sext <4 x i1> %77 to <4 x i32>
  %79 = add <4 x i32> %76, %78
  %80 = sub <4 x i32> zeroinitializer, %79
  %81 = icmp sgt <4 x i32> %80, <i32 4, i32 4, i32 4, i32 4>
  %82 = sext <4 x i1> %81 to <4 x i32>
  %83 = and <4 x i32> %iYes.09, %82
  %84 = xor <4 x i32> %82, <i32 -1, i32 -1, i32 -1, i32 -1>
  %85 = and <4 x i32> %iMin.08, %84
  %86 = or <4 x i32> %83, %85
  %87 = and <4 x i32> %iYes.09, %84
  %88 = and <4 x i32> %iMax.07, %82
  %89 = or <4 x i32> %87, %88
  %90 = add <4 x i32> %89, %86
  %91 = ashr <4 x i32> %90, <i32 1, i32 1, i32 1, i32 1>
  %92 = add nsw i32 %iSearch.010, 1
  %exitcond13 = icmp eq i32 %92, 8
  br i1 %exitcond13, label %93, label %.preheader3

; <label>:93                                      ; preds = %.preheader3
  %94 = extractelement <4 x i32> %91, i32 0
  %95 = and i32 %94, 255
  %96 = extractelement <4 x i32> %91, i32 1
  %97 = shl i32 %96, 8
  %98 = and i32 %97, 65280
  %99 = extractelement <4 x i32> %91, i32 2
  %100 = shl i32 %99, 16
  %101 = and i32 %100, 16711680
  %102 = or i32 %101, %95
  %103 = or i32 %102, %98
  %104 = add nsw i32 %x.011, %2
  %105 = getelementptr inbounds i32 addrspace(1)* %pDst, i32 %104
  store i32 %103, i32 addrspace(1)* %105, align 4
  %106 = add nsw i32 %x.011, 1
  %exitcond14 = icmp eq i32 %106, %iImageWidth
  br i1 %exitcond14, label %._crit_edge, label %.preheader

._crit_edge:                                      ; preds = %93
  ret void
}

declare i32 @get_global_id(i32) nounwind readnone

define void @intel_median_scalar(i8 addrspace(1)* nocapture %pSrc, i32 addrspace(1)* nocapture %pDst, i32 %iImageWidth, i32 %iImageHeight) nounwind {
.preheader11.lr.ph:
  %iResult = alloca [4 x i32], align 4
  %0 = call i32 @get_global_id(i32 0) nounwind readnone
  %1 = add i32 %0, 2
  %2 = getelementptr inbounds [4 x i32]* %iResult, i32 0, i32 0
  %3 = getelementptr inbounds [4 x i32]* %iResult, i32 0, i32 1
  %4 = getelementptr inbounds [4 x i32]* %iResult, i32 0, i32 2
  %5 = mul nsw i32 %1, %iImageWidth
  br label %.preheader11

.preheader11:                                     ; preds = %90, %.preheader11.lr.ph
  %x.013 = phi i32 [ 0, %.preheader11.lr.ph ], [ %103, %90 ]
  br label %.preheader

.preheader:                                       ; preds = %87, %.preheader11
  %ch.012 = phi i32 [ 0, %.preheader11 ], [ %89, %87 ]
  %6 = add i32 %ch.012, -4
  %7 = add i32 %ch.012, 4
  %8 = add i32 %0, 1
  %9 = mul nsw i32 %8, %iImageWidth
  %10 = add nsw i32 %9, %x.013
  %11 = shl i32 %10, 2
  %12 = add i32 %6, %11
  %13 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %12
  %14 = load i8 addrspace(1)* %13, align 1
  %15 = zext i8 %14 to i32
  %16 = add nsw i32 %11, %ch.012
  %17 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %16
  %18 = load i8 addrspace(1)* %17, align 1
  %19 = zext i8 %18 to i32
  %20 = add i32 %7, %11
  %21 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %20
  %22 = load i8 addrspace(1)* %21, align 1
  %23 = zext i8 %22 to i32
  %24 = add nsw i32 %5, %x.013
  %25 = shl i32 %24, 2
  %26 = add i32 %6, %25
  %27 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %26
  %28 = load i8 addrspace(1)* %27, align 1
  %29 = zext i8 %28 to i32
  %30 = add nsw i32 %25, %ch.012
  %31 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %30
  %32 = load i8 addrspace(1)* %31, align 1
  %33 = zext i8 %32 to i32
  %34 = add i32 %7, %25
  %35 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %34
  %36 = load i8 addrspace(1)* %35, align 1
  %37 = zext i8 %36 to i32
  %38 = add i32 %0, 3
  %39 = mul nsw i32 %38, %iImageWidth
  %40 = add nsw i32 %39, %x.013
  %41 = shl i32 %40, 2
  %42 = add i32 %6, %41
  %43 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %42
  %44 = load i8 addrspace(1)* %43, align 1
  %45 = zext i8 %44 to i32
  %46 = add nsw i32 %41, %ch.012
  %47 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %46
  %48 = load i8 addrspace(1)* %47, align 1
  %49 = zext i8 %48 to i32
  %50 = add i32 %7, %41
  %51 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %50
  %52 = load i8 addrspace(1)* %51, align 1
  %53 = zext i8 %52 to i32
  br label %.preheader3

.preheader3:                                      ; preds = %.preheader3, %.preheader
  %iSearch.010 = phi i32 [ 0, %.preheader ], [ %86, %.preheader3 ]
  %iMax.09 = phi i32 [ 255, %.preheader ], [ %83, %.preheader3 ]
  %iMin.08 = phi i32 [ 0, %.preheader ], [ %81, %.preheader3 ]
  %iYes.07 = phi i32 [ 128, %.preheader ], [ %85, %.preheader3 ]
  %54 = icmp slt i32 %iYes.07, %15
  %55 = zext i1 %54 to i32
  %56 = icmp slt i32 %iYes.07, %19
  %57 = zext i1 %56 to i32
  %58 = icmp slt i32 %iYes.07, %23
  %59 = zext i1 %58 to i32
  %60 = add i32 %55, %57
  %61 = add i32 %60, %59
  %62 = icmp slt i32 %iYes.07, %29
  %63 = zext i1 %62 to i32
  %64 = icmp slt i32 %iYes.07, %33
  %65 = zext i1 %64 to i32
  %66 = icmp slt i32 %iYes.07, %37
  %67 = zext i1 %66 to i32
  %68 = add i32 %63, %61
  %69 = add i32 %68, %65
  %70 = add i32 %69, %67
  %71 = icmp slt i32 %iYes.07, %45
  %72 = zext i1 %71 to i32
  %73 = icmp slt i32 %iYes.07, %49
  %74 = zext i1 %73 to i32
  %75 = icmp slt i32 %iYes.07, %53
  %76 = zext i1 %75 to i32
  %77 = add i32 %72, %70
  %78 = add i32 %77, %74
  %79 = add i32 %78, %76
  %80 = icmp sgt i32 %79, 4
  %81 = select i1 %80, i32 %iYes.07, i32 %iMin.08
  %82 = icmp slt i32 %79, 5
  %83 = select i1 %82, i32 %iYes.07, i32 %iMax.09
  %84 = add nsw i32 %83, %81
  %85 = ashr i32 %84, 1
  %86 = add nsw i32 %iSearch.010, 1
  %exitcond = icmp eq i32 %86, 8
  br i1 %exitcond, label %87, label %.preheader3

; <label>:87                                      ; preds = %.preheader3
  %88 = getelementptr inbounds [4 x i32]* %iResult, i32 0, i32 %ch.012
  store i32 %85, i32* %88, align 4
  %89 = add nsw i32 %ch.012, 1
  %exitcond14 = icmp eq i32 %89, 3
  br i1 %exitcond14, label %90, label %.preheader

; <label>:90                                      ; preds = %87
  %91 = load i32* %2, align 4
  %92 = and i32 %91, 255
  %93 = load i32* %3, align 4
  %94 = shl i32 %93, 8
  %95 = and i32 %94, 65280
  %96 = load i32* %4, align 4
  %97 = shl i32 %96, 16
  %98 = and i32 %97, 16711680
  %99 = or i32 %95, %92
  %100 = or i32 %99, %98
  %101 = add nsw i32 %x.013, %5
  %102 = getelementptr inbounds i32 addrspace(1)* %pDst, i32 %101
  store i32 %100, i32 addrspace(1)* %102, align 4
  %103 = add nsw i32 %x.013, 1
  %exitcond15 = icmp eq i32 %103, %iImageWidth
  br i1 %exitcond15, label %._crit_edge, label %.preheader11

._crit_edge:                                      ; preds = %90
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
.preheader.lr.ph:
  %temp = insertelement <4 x i32> undef, i32 %iImageWidth, i32 0
  %vector = shufflevector <4 x i32> %temp, <4 x i32> undef, <4 x i32> zeroinitializer
  %0 = alloca [9 x <4 x i32>], align 16
  %1 = alloca [9 x <4 x i32>], align 16
  %2 = alloca [9 x <4 x i32>], align 16
  %3 = alloca [9 x <4 x i32>], align 16
  %4 = call i32 @get_global_id(i32 0) nounwind readnone
  %broadcast1 = insertelement <4 x i32> undef, i32 %4, i32 0
  %broadcast2 = shufflevector <4 x i32> %broadcast1, <4 x i32> undef, <4 x i32> zeroinitializer
  %5 = add <4 x i32> %broadcast2, <i32 2, i32 3, i32 4, i32 5>
  %6 = mul nsw <4 x i32> %5, %vector
  br label %.preheader

.preheader:                                       ; preds = %265, %.preheader.lr.ph
  %vectorPHI = phi <4 x i32> [ undef, %.preheader.lr.ph ], [ %shuffleMerge165, %265 ]
  %x.011 = phi i32 [ 0, %.preheader.lr.ph ], [ %278, %265 ]
  %temp86 = insertelement <4 x i32> undef, i32 %x.011, i32 0
  %vector87 = shufflevector <4 x i32> %temp86, <4 x i32> undef, <4 x i32> zeroinitializer
  br label %7

; <label>:7                                       ; preds = %._crit_edge30, %.preheader
  %vectorPHI83 = phi <4 x i32> [ %vectorPHI, %.preheader ], [ %shuffleMerge153, %._crit_edge30 ]
  %iRow.02 = phi i32 [ -1, %.preheader ], [ %69, %._crit_edge30 ]
  %iPixelCount.01 = phi i32 [ 0, %.preheader ], [ %68, %._crit_edge30 ]
  %temp84 = insertelement <4 x i32> undef, i32 %iRow.02, i32 0
  %vector85 = shufflevector <4 x i32> %temp84, <4 x i32> undef, <4 x i32> zeroinitializer
  %8 = add <4 x i32> %5, %vector85
  %9 = mul nsw <4 x i32> %8, %vector
  %10 = add nsw <4 x i32> %9, %vector87
  %extract103 = extractelement <4 x i32> %10, i32 0
  %extract104 = extractelement <4 x i32> %10, i32 1
  %extract105 = extractelement <4 x i32> %10, i32 2
  %extract106 = extractelement <4 x i32> %10, i32 3
  %11 = add nsw <4 x i32> %10, <i32 -1, i32 -1, i32 -1, i32 -1>
  %extract = extractelement <4 x i32> %11, i32 0
  %extract88 = extractelement <4 x i32> %11, i32 1
  %extract89 = extractelement <4 x i32> %11, i32 2
  %extract90 = extractelement <4 x i32> %11, i32 3
  %12 = getelementptr inbounds <4 x i8> addrspace(1)* %pSrc, i32 %extract
  %13 = getelementptr inbounds <4 x i8> addrspace(1)* %pSrc, i32 %extract88
  %14 = getelementptr inbounds <4 x i8> addrspace(1)* %pSrc, i32 %extract89
  %15 = getelementptr inbounds <4 x i8> addrspace(1)* %pSrc, i32 %extract90
  %16 = load <4 x i8> addrspace(1)* %12, align 4
  %17 = load <4 x i8> addrspace(1)* %13, align 4
  %18 = load <4 x i8> addrspace(1)* %14, align 4
  %19 = load <4 x i8> addrspace(1)* %15, align 4
  %shuffle0 = shufflevector <4 x i8> %16, <4 x i8> %17, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffle1 = shufflevector <4 x i8> %18, <4 x i8> %19, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffleMerge = shufflevector <4 x i8> %shuffle0, <4 x i8> %shuffle1, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffle091 = shufflevector <4 x i8> %16, <4 x i8> %17, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffle192 = shufflevector <4 x i8> %18, <4 x i8> %19, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffleMerge93 = shufflevector <4 x i8> %shuffle091, <4 x i8> %shuffle192, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle094 = shufflevector <4 x i8> %16, <4 x i8> %17, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffle195 = shufflevector <4 x i8> %18, <4 x i8> %19, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffleMerge96 = shufflevector <4 x i8> %shuffle094, <4 x i8> %shuffle195, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %20 = zext <4 x i8> %shuffleMerge to <4 x i32>
  %21 = getelementptr inbounds [9 x <4 x i32>]* %0, i32 0, i32 %iPixelCount.01
  %22 = getelementptr inbounds [9 x <4 x i32>]* %1, i32 0, i32 %iPixelCount.01
  %23 = getelementptr inbounds [9 x <4 x i32>]* %2, i32 0, i32 %iPixelCount.01
  %24 = getelementptr inbounds [9 x <4 x i32>]* %3, i32 0, i32 %iPixelCount.01
  %25 = zext <4 x i8> %shuffleMerge93 to <4 x i32>
  %26 = zext <4 x i8> %shuffleMerge96 to <4 x i32>
  %shuf_transpL = shufflevector <4 x i32> %20, <4 x i32> %26, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuf_transpL97 = shufflevector <4 x i32> %25, <4 x i32> %vectorPHI83, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuf_transpH = shufflevector <4 x i32> %20, <4 x i32> %26, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuf_transpH98 = shufflevector <4 x i32> %25, <4 x i32> %vectorPHI83, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuf_transpL99 = shufflevector <4 x i32> %shuf_transpL, <4 x i32> %shuf_transpL97, <4 x i32> <i32 0, i32 4, i32 2, i32 6>
  %shuf_transpH100 = shufflevector <4 x i32> %shuf_transpL, <4 x i32> %shuf_transpL97, <4 x i32> <i32 1, i32 5, i32 3, i32 7>
  %shuf_transpL101 = shufflevector <4 x i32> %shuf_transpH, <4 x i32> %shuf_transpH98, <4 x i32> <i32 0, i32 4, i32 2, i32 6>
  %shuf_transpH102 = shufflevector <4 x i32> %shuf_transpH, <4 x i32> %shuf_transpH98, <4 x i32> <i32 1, i32 5, i32 3, i32 7>
  store <4 x i32> %shuf_transpL99, <4 x i32>* %21, align 16
  store <4 x i32> %shuf_transpH100, <4 x i32>* %22, align 16
  store <4 x i32> %shuf_transpL101, <4 x i32>* %23, align 16
  store <4 x i32> %shuf_transpH102, <4 x i32>* %24, align 16
  %27 = add nsw i32 %iPixelCount.01, 1
  %28 = getelementptr inbounds <4 x i8> addrspace(1)* %pSrc, i32 %extract103
  %29 = getelementptr inbounds <4 x i8> addrspace(1)* %pSrc, i32 %extract104
  %30 = getelementptr inbounds <4 x i8> addrspace(1)* %pSrc, i32 %extract105
  %31 = getelementptr inbounds <4 x i8> addrspace(1)* %pSrc, i32 %extract106
  %32 = load <4 x i8> addrspace(1)* %28, align 4
  %33 = load <4 x i8> addrspace(1)* %29, align 4
  %34 = load <4 x i8> addrspace(1)* %30, align 4
  %35 = load <4 x i8> addrspace(1)* %31, align 4
  %shuffle0107 = shufflevector <4 x i8> %32, <4 x i8> %33, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffle1108 = shufflevector <4 x i8> %34, <4 x i8> %35, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffleMerge109 = shufflevector <4 x i8> %shuffle0107, <4 x i8> %shuffle1108, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffle0110 = shufflevector <4 x i8> %32, <4 x i8> %33, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffle1111 = shufflevector <4 x i8> %34, <4 x i8> %35, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffleMerge112 = shufflevector <4 x i8> %shuffle0110, <4 x i8> %shuffle1111, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle0113 = shufflevector <4 x i8> %32, <4 x i8> %33, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffle1114 = shufflevector <4 x i8> %34, <4 x i8> %35, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffleMerge115 = shufflevector <4 x i8> %shuffle0113, <4 x i8> %shuffle1114, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %36 = zext <4 x i8> %shuffleMerge109 to <4 x i32>
  %37 = getelementptr inbounds [9 x <4 x i32>]* %0, i32 0, i32 %27
  %38 = getelementptr inbounds [9 x <4 x i32>]* %1, i32 0, i32 %27
  %39 = getelementptr inbounds [9 x <4 x i32>]* %2, i32 0, i32 %27
  %40 = getelementptr inbounds [9 x <4 x i32>]* %3, i32 0, i32 %27
  %41 = load <4 x i32>* %37, align 16
  %42 = load <4 x i32>* %38, align 16
  %43 = load <4 x i32>* %39, align 16
  %44 = load <4 x i32>* %40, align 16
  %shuffle0116 = shufflevector <4 x i32> %41, <4 x i32> %42, <4 x i32> <i32 undef, i32 3, i32 undef, i32 7>
  %shuffle1117 = shufflevector <4 x i32> %43, <4 x i32> %44, <4 x i32> <i32 undef, i32 3, i32 undef, i32 7>
  %shuffleMerge118 = shufflevector <4 x i32> %shuffle0116, <4 x i32> %shuffle1117, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %45 = zext <4 x i8> %shuffleMerge112 to <4 x i32>
  %46 = zext <4 x i8> %shuffleMerge115 to <4 x i32>
  %shuf_transpL119 = shufflevector <4 x i32> %36, <4 x i32> %46, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuf_transpL120 = shufflevector <4 x i32> %45, <4 x i32> %shuffleMerge118, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuf_transpH121 = shufflevector <4 x i32> %36, <4 x i32> %46, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuf_transpH122 = shufflevector <4 x i32> %45, <4 x i32> %shuffleMerge118, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuf_transpL123 = shufflevector <4 x i32> %shuf_transpL119, <4 x i32> %shuf_transpL120, <4 x i32> <i32 0, i32 4, i32 2, i32 6>
  %shuf_transpH124 = shufflevector <4 x i32> %shuf_transpL119, <4 x i32> %shuf_transpL120, <4 x i32> <i32 1, i32 5, i32 3, i32 7>
  %shuf_transpL125 = shufflevector <4 x i32> %shuf_transpH121, <4 x i32> %shuf_transpH122, <4 x i32> <i32 0, i32 4, i32 2, i32 6>
  %shuf_transpH126 = shufflevector <4 x i32> %shuf_transpH121, <4 x i32> %shuf_transpH122, <4 x i32> <i32 1, i32 5, i32 3, i32 7>
  store <4 x i32> %shuf_transpL123, <4 x i32>* %37, align 16
  store <4 x i32> %shuf_transpH124, <4 x i32>* %38, align 16
  store <4 x i32> %shuf_transpL125, <4 x i32>* %39, align 16
  store <4 x i32> %shuf_transpH126, <4 x i32>* %40, align 16
  %47 = add nsw i32 %iPixelCount.01, 2
  %48 = add nsw <4 x i32> %10, <i32 1, i32 1, i32 1, i32 1>
  %extract127 = extractelement <4 x i32> %48, i32 0
  %extract128 = extractelement <4 x i32> %48, i32 1
  %extract129 = extractelement <4 x i32> %48, i32 2
  %extract130 = extractelement <4 x i32> %48, i32 3
  %49 = getelementptr inbounds <4 x i8> addrspace(1)* %pSrc, i32 %extract127
  %50 = getelementptr inbounds <4 x i8> addrspace(1)* %pSrc, i32 %extract128
  %51 = getelementptr inbounds <4 x i8> addrspace(1)* %pSrc, i32 %extract129
  %52 = getelementptr inbounds <4 x i8> addrspace(1)* %pSrc, i32 %extract130
  %53 = load <4 x i8> addrspace(1)* %49, align 4
  %54 = load <4 x i8> addrspace(1)* %50, align 4
  %55 = load <4 x i8> addrspace(1)* %51, align 4
  %56 = load <4 x i8> addrspace(1)* %52, align 4
  %shuffle0131 = shufflevector <4 x i8> %53, <4 x i8> %54, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffle1132 = shufflevector <4 x i8> %55, <4 x i8> %56, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffleMerge133 = shufflevector <4 x i8> %shuffle0131, <4 x i8> %shuffle1132, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffle0134 = shufflevector <4 x i8> %53, <4 x i8> %54, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffle1135 = shufflevector <4 x i8> %55, <4 x i8> %56, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffleMerge136 = shufflevector <4 x i8> %shuffle0134, <4 x i8> %shuffle1135, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle0137 = shufflevector <4 x i8> %53, <4 x i8> %54, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffle1138 = shufflevector <4 x i8> %55, <4 x i8> %56, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffleMerge139 = shufflevector <4 x i8> %shuffle0137, <4 x i8> %shuffle1138, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %57 = zext <4 x i8> %shuffleMerge133 to <4 x i32>
  %58 = getelementptr inbounds [9 x <4 x i32>]* %0, i32 0, i32 %47
  %59 = getelementptr inbounds [9 x <4 x i32>]* %1, i32 0, i32 %47
  %60 = getelementptr inbounds [9 x <4 x i32>]* %2, i32 0, i32 %47
  %61 = getelementptr inbounds [9 x <4 x i32>]* %3, i32 0, i32 %47
  %62 = load <4 x i32>* %58, align 16
  %63 = load <4 x i32>* %59, align 16
  %64 = load <4 x i32>* %60, align 16
  %65 = load <4 x i32>* %61, align 16
  %shuffle0140 = shufflevector <4 x i32> %62, <4 x i32> %63, <4 x i32> <i32 undef, i32 3, i32 undef, i32 7>
  %shuffle1141 = shufflevector <4 x i32> %64, <4 x i32> %65, <4 x i32> <i32 undef, i32 3, i32 undef, i32 7>
  %shuffleMerge142 = shufflevector <4 x i32> %shuffle0140, <4 x i32> %shuffle1141, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %66 = zext <4 x i8> %shuffleMerge136 to <4 x i32>
  %67 = zext <4 x i8> %shuffleMerge139 to <4 x i32>
  %shuf_transpL143 = shufflevector <4 x i32> %57, <4 x i32> %67, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuf_transpL144 = shufflevector <4 x i32> %66, <4 x i32> %shuffleMerge142, <4 x i32> <i32 0, i32 1, i32 4, i32 5>
  %shuf_transpH145 = shufflevector <4 x i32> %57, <4 x i32> %67, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuf_transpH146 = shufflevector <4 x i32> %66, <4 x i32> %shuffleMerge142, <4 x i32> <i32 2, i32 3, i32 6, i32 7>
  %shuf_transpL147 = shufflevector <4 x i32> %shuf_transpL143, <4 x i32> %shuf_transpL144, <4 x i32> <i32 0, i32 4, i32 2, i32 6>
  %shuf_transpH148 = shufflevector <4 x i32> %shuf_transpL143, <4 x i32> %shuf_transpL144, <4 x i32> <i32 1, i32 5, i32 3, i32 7>
  %shuf_transpL149 = shufflevector <4 x i32> %shuf_transpH145, <4 x i32> %shuf_transpH146, <4 x i32> <i32 0, i32 4, i32 2, i32 6>
  %shuf_transpH150 = shufflevector <4 x i32> %shuf_transpH145, <4 x i32> %shuf_transpH146, <4 x i32> <i32 1, i32 5, i32 3, i32 7>
  store <4 x i32> %shuf_transpL147, <4 x i32>* %58, align 16
  store <4 x i32> %shuf_transpH148, <4 x i32>* %59, align 16
  store <4 x i32> %shuf_transpL149, <4 x i32>* %60, align 16
  store <4 x i32> %shuf_transpH150, <4 x i32>* %61, align 16
  %68 = add nsw i32 %iPixelCount.01, 3
  %69 = add nsw i32 %iRow.02, 1
  %exitcond = icmp eq i32 %69, 2
  br i1 %exitcond, label %.loopexit, label %._crit_edge30

._crit_edge30:                                    ; preds = %7
  %70 = getelementptr inbounds [9 x <4 x i32>]* %0, i32 0, i32 %68
  %71 = getelementptr inbounds [9 x <4 x i32>]* %1, i32 0, i32 %68
  %72 = getelementptr inbounds [9 x <4 x i32>]* %2, i32 0, i32 %68
  %73 = getelementptr inbounds [9 x <4 x i32>]* %3, i32 0, i32 %68
  %74 = load <4 x i32>* %70, align 16
  %75 = load <4 x i32>* %71, align 16
  %76 = load <4 x i32>* %72, align 16
  %77 = load <4 x i32>* %73, align 16
  %shuffle0151 = shufflevector <4 x i32> %74, <4 x i32> %75, <4 x i32> <i32 undef, i32 3, i32 undef, i32 7>
  %shuffle1152 = shufflevector <4 x i32> %76, <4 x i32> %77, <4 x i32> <i32 undef, i32 3, i32 undef, i32 7>
  %shuffleMerge153 = shufflevector <4 x i32> %shuffle0151, <4 x i32> %shuffle1152, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  br label %7

.loopexit:                                        ; preds = %7
  %78 = getelementptr inbounds [9 x <4 x i32>]* %0, i32 0, i32 0
  %79 = getelementptr inbounds [9 x <4 x i32>]* %1, i32 0, i32 0
  %80 = getelementptr inbounds [9 x <4 x i32>]* %2, i32 0, i32 0
  %81 = getelementptr inbounds [9 x <4 x i32>]* %3, i32 0, i32 0
  %82 = load <4 x i32>* %78, align 16
  %83 = load <4 x i32>* %79, align 16
  %84 = load <4 x i32>* %80, align 16
  %85 = load <4 x i32>* %81, align 16
  %shuffle0154 = shufflevector <4 x i32> %82, <4 x i32> %83, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffle1155 = shufflevector <4 x i32> %84, <4 x i32> %85, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffleMerge156 = shufflevector <4 x i32> %shuffle0154, <4 x i32> %shuffle1155, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffle0157 = shufflevector <4 x i32> %82, <4 x i32> %83, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffle1158 = shufflevector <4 x i32> %84, <4 x i32> %85, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffleMerge159 = shufflevector <4 x i32> %shuffle0157, <4 x i32> %shuffle1158, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle0160 = shufflevector <4 x i32> %82, <4 x i32> %83, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffle1161 = shufflevector <4 x i32> %84, <4 x i32> %85, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffleMerge162 = shufflevector <4 x i32> %shuffle0160, <4 x i32> %shuffle1161, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffle0163 = shufflevector <4 x i32> %82, <4 x i32> %83, <4 x i32> <i32 undef, i32 3, i32 undef, i32 7>
  %shuffle1164 = shufflevector <4 x i32> %84, <4 x i32> %85, <4 x i32> <i32 undef, i32 3, i32 undef, i32 7>
  %shuffleMerge165 = shufflevector <4 x i32> %shuffle0163, <4 x i32> %shuffle1164, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %86 = getelementptr inbounds [9 x <4 x i32>]* %0, i32 0, i32 1
  %87 = getelementptr inbounds [9 x <4 x i32>]* %1, i32 0, i32 1
  %88 = getelementptr inbounds [9 x <4 x i32>]* %2, i32 0, i32 1
  %89 = getelementptr inbounds [9 x <4 x i32>]* %3, i32 0, i32 1
  %90 = load <4 x i32>* %86, align 16
  %91 = load <4 x i32>* %87, align 16
  %92 = load <4 x i32>* %88, align 16
  %93 = load <4 x i32>* %89, align 16
  %shuffle0166 = shufflevector <4 x i32> %90, <4 x i32> %91, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffle1167 = shufflevector <4 x i32> %92, <4 x i32> %93, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffleMerge168 = shufflevector <4 x i32> %shuffle0166, <4 x i32> %shuffle1167, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffle0169 = shufflevector <4 x i32> %90, <4 x i32> %91, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffle1170 = shufflevector <4 x i32> %92, <4 x i32> %93, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffleMerge171 = shufflevector <4 x i32> %shuffle0169, <4 x i32> %shuffle1170, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle0172 = shufflevector <4 x i32> %90, <4 x i32> %91, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffle1173 = shufflevector <4 x i32> %92, <4 x i32> %93, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffleMerge174 = shufflevector <4 x i32> %shuffle0172, <4 x i32> %shuffle1173, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %94 = getelementptr inbounds [9 x <4 x i32>]* %0, i32 0, i32 2
  %95 = getelementptr inbounds [9 x <4 x i32>]* %1, i32 0, i32 2
  %96 = getelementptr inbounds [9 x <4 x i32>]* %2, i32 0, i32 2
  %97 = getelementptr inbounds [9 x <4 x i32>]* %3, i32 0, i32 2
  %98 = load <4 x i32>* %94, align 16
  %99 = load <4 x i32>* %95, align 16
  %100 = load <4 x i32>* %96, align 16
  %101 = load <4 x i32>* %97, align 16
  %shuffle0178 = shufflevector <4 x i32> %98, <4 x i32> %99, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffle1179 = shufflevector <4 x i32> %100, <4 x i32> %101, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffleMerge180 = shufflevector <4 x i32> %shuffle0178, <4 x i32> %shuffle1179, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffle0181 = shufflevector <4 x i32> %98, <4 x i32> %99, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffle1182 = shufflevector <4 x i32> %100, <4 x i32> %101, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffleMerge183 = shufflevector <4 x i32> %shuffle0181, <4 x i32> %shuffle1182, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle0184 = shufflevector <4 x i32> %98, <4 x i32> %99, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffle1185 = shufflevector <4 x i32> %100, <4 x i32> %101, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffleMerge186 = shufflevector <4 x i32> %shuffle0184, <4 x i32> %shuffle1185, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %102 = getelementptr inbounds [9 x <4 x i32>]* %0, i32 0, i32 3
  %103 = getelementptr inbounds [9 x <4 x i32>]* %1, i32 0, i32 3
  %104 = getelementptr inbounds [9 x <4 x i32>]* %2, i32 0, i32 3
  %105 = getelementptr inbounds [9 x <4 x i32>]* %3, i32 0, i32 3
  %106 = load <4 x i32>* %102, align 16
  %107 = load <4 x i32>* %103, align 16
  %108 = load <4 x i32>* %104, align 16
  %109 = load <4 x i32>* %105, align 16
  %shuffle0190 = shufflevector <4 x i32> %106, <4 x i32> %107, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffle1191 = shufflevector <4 x i32> %108, <4 x i32> %109, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffleMerge192 = shufflevector <4 x i32> %shuffle0190, <4 x i32> %shuffle1191, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffle0193 = shufflevector <4 x i32> %106, <4 x i32> %107, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffle1194 = shufflevector <4 x i32> %108, <4 x i32> %109, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffleMerge195 = shufflevector <4 x i32> %shuffle0193, <4 x i32> %shuffle1194, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle0196 = shufflevector <4 x i32> %106, <4 x i32> %107, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffle1197 = shufflevector <4 x i32> %108, <4 x i32> %109, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffleMerge198 = shufflevector <4 x i32> %shuffle0196, <4 x i32> %shuffle1197, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %110 = getelementptr inbounds [9 x <4 x i32>]* %0, i32 0, i32 4
  %111 = getelementptr inbounds [9 x <4 x i32>]* %1, i32 0, i32 4
  %112 = getelementptr inbounds [9 x <4 x i32>]* %2, i32 0, i32 4
  %113 = getelementptr inbounds [9 x <4 x i32>]* %3, i32 0, i32 4
  %114 = load <4 x i32>* %110, align 16
  %115 = load <4 x i32>* %111, align 16
  %116 = load <4 x i32>* %112, align 16
  %117 = load <4 x i32>* %113, align 16
  %shuffle0202 = shufflevector <4 x i32> %114, <4 x i32> %115, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffle1203 = shufflevector <4 x i32> %116, <4 x i32> %117, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffleMerge204 = shufflevector <4 x i32> %shuffle0202, <4 x i32> %shuffle1203, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffle0205 = shufflevector <4 x i32> %114, <4 x i32> %115, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffle1206 = shufflevector <4 x i32> %116, <4 x i32> %117, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffleMerge207 = shufflevector <4 x i32> %shuffle0205, <4 x i32> %shuffle1206, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle0208 = shufflevector <4 x i32> %114, <4 x i32> %115, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffle1209 = shufflevector <4 x i32> %116, <4 x i32> %117, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffleMerge210 = shufflevector <4 x i32> %shuffle0208, <4 x i32> %shuffle1209, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %118 = getelementptr inbounds [9 x <4 x i32>]* %0, i32 0, i32 5
  %119 = getelementptr inbounds [9 x <4 x i32>]* %1, i32 0, i32 5
  %120 = getelementptr inbounds [9 x <4 x i32>]* %2, i32 0, i32 5
  %121 = getelementptr inbounds [9 x <4 x i32>]* %3, i32 0, i32 5
  %122 = load <4 x i32>* %118, align 16
  %123 = load <4 x i32>* %119, align 16
  %124 = load <4 x i32>* %120, align 16
  %125 = load <4 x i32>* %121, align 16
  %shuffle0214 = shufflevector <4 x i32> %122, <4 x i32> %123, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffle1215 = shufflevector <4 x i32> %124, <4 x i32> %125, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffleMerge216 = shufflevector <4 x i32> %shuffle0214, <4 x i32> %shuffle1215, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffle0217 = shufflevector <4 x i32> %122, <4 x i32> %123, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffle1218 = shufflevector <4 x i32> %124, <4 x i32> %125, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffleMerge219 = shufflevector <4 x i32> %shuffle0217, <4 x i32> %shuffle1218, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle0220 = shufflevector <4 x i32> %122, <4 x i32> %123, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffle1221 = shufflevector <4 x i32> %124, <4 x i32> %125, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffleMerge222 = shufflevector <4 x i32> %shuffle0220, <4 x i32> %shuffle1221, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %126 = getelementptr inbounds [9 x <4 x i32>]* %0, i32 0, i32 6
  %127 = getelementptr inbounds [9 x <4 x i32>]* %1, i32 0, i32 6
  %128 = getelementptr inbounds [9 x <4 x i32>]* %2, i32 0, i32 6
  %129 = getelementptr inbounds [9 x <4 x i32>]* %3, i32 0, i32 6
  %130 = load <4 x i32>* %126, align 16
  %131 = load <4 x i32>* %127, align 16
  %132 = load <4 x i32>* %128, align 16
  %133 = load <4 x i32>* %129, align 16
  %shuffle0226 = shufflevector <4 x i32> %130, <4 x i32> %131, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffle1227 = shufflevector <4 x i32> %132, <4 x i32> %133, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffleMerge228 = shufflevector <4 x i32> %shuffle0226, <4 x i32> %shuffle1227, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffle0229 = shufflevector <4 x i32> %130, <4 x i32> %131, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffle1230 = shufflevector <4 x i32> %132, <4 x i32> %133, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffleMerge231 = shufflevector <4 x i32> %shuffle0229, <4 x i32> %shuffle1230, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle0232 = shufflevector <4 x i32> %130, <4 x i32> %131, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffle1233 = shufflevector <4 x i32> %132, <4 x i32> %133, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffleMerge234 = shufflevector <4 x i32> %shuffle0232, <4 x i32> %shuffle1233, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %134 = getelementptr inbounds [9 x <4 x i32>]* %0, i32 0, i32 7
  %135 = getelementptr inbounds [9 x <4 x i32>]* %1, i32 0, i32 7
  %136 = getelementptr inbounds [9 x <4 x i32>]* %2, i32 0, i32 7
  %137 = getelementptr inbounds [9 x <4 x i32>]* %3, i32 0, i32 7
  %138 = load <4 x i32>* %134, align 16
  %139 = load <4 x i32>* %135, align 16
  %140 = load <4 x i32>* %136, align 16
  %141 = load <4 x i32>* %137, align 16
  %shuffle0238 = shufflevector <4 x i32> %138, <4 x i32> %139, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffle1239 = shufflevector <4 x i32> %140, <4 x i32> %141, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffleMerge240 = shufflevector <4 x i32> %shuffle0238, <4 x i32> %shuffle1239, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffle0241 = shufflevector <4 x i32> %138, <4 x i32> %139, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffle1242 = shufflevector <4 x i32> %140, <4 x i32> %141, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffleMerge243 = shufflevector <4 x i32> %shuffle0241, <4 x i32> %shuffle1242, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle0244 = shufflevector <4 x i32> %138, <4 x i32> %139, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffle1245 = shufflevector <4 x i32> %140, <4 x i32> %141, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffleMerge246 = shufflevector <4 x i32> %shuffle0244, <4 x i32> %shuffle1245, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %142 = getelementptr inbounds [9 x <4 x i32>]* %0, i32 0, i32 8
  %143 = getelementptr inbounds [9 x <4 x i32>]* %1, i32 0, i32 8
  %144 = getelementptr inbounds [9 x <4 x i32>]* %2, i32 0, i32 8
  %145 = getelementptr inbounds [9 x <4 x i32>]* %3, i32 0, i32 8
  %146 = load <4 x i32>* %142, align 16
  %147 = load <4 x i32>* %143, align 16
  %148 = load <4 x i32>* %144, align 16
  %149 = load <4 x i32>* %145, align 16
  %shuffle0250 = shufflevector <4 x i32> %146, <4 x i32> %147, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffle1251 = shufflevector <4 x i32> %148, <4 x i32> %149, <4 x i32> <i32 0, i32 undef, i32 4, i32 undef>
  %shuffleMerge252 = shufflevector <4 x i32> %shuffle0250, <4 x i32> %shuffle1251, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  %shuffle0253 = shufflevector <4 x i32> %146, <4 x i32> %147, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffle1254 = shufflevector <4 x i32> %148, <4 x i32> %149, <4 x i32> <i32 undef, i32 1, i32 undef, i32 5>
  %shuffleMerge255 = shufflevector <4 x i32> %shuffle0253, <4 x i32> %shuffle1254, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
  %shuffle0256 = shufflevector <4 x i32> %146, <4 x i32> %147, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffle1257 = shufflevector <4 x i32> %148, <4 x i32> %149, <4 x i32> <i32 2, i32 undef, i32 6, i32 undef>
  %shuffleMerge258 = shufflevector <4 x i32> %shuffle0256, <4 x i32> %shuffle1257, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
  br label %.preheader3

.preheader3:                                      ; preds = %.preheader3, %.loopexit
  %iSearch.010 = phi i32 [ 0, %.loopexit ], [ %264, %.preheader3 ]
  %vectorPHI262 = phi <4 x i32> [ <i32 128, i32 128, i32 128, i32 128>, %.loopexit ], [ %261, %.preheader3 ]
  %vectorPHI263 = phi <4 x i32> [ <i32 128, i32 128, i32 128, i32 128>, %.loopexit ], [ %262, %.preheader3 ]
  %vectorPHI264 = phi <4 x i32> [ <i32 128, i32 128, i32 128, i32 128>, %.loopexit ], [ %263, %.preheader3 ]
  %vectorPHI266 = phi <4 x i32> [ zeroinitializer, %.loopexit ], [ %246, %.preheader3 ]
  %vectorPHI267 = phi <4 x i32> [ zeroinitializer, %.loopexit ], [ %247, %.preheader3 ]
  %vectorPHI268 = phi <4 x i32> [ zeroinitializer, %.loopexit ], [ %248, %.preheader3 ]
  %vectorPHI270 = phi <4 x i32> [ <i32 255, i32 255, i32 255, i32 255>, %.loopexit ], [ %255, %.preheader3 ]
  %vectorPHI271 = phi <4 x i32> [ <i32 255, i32 255, i32 255, i32 255>, %.loopexit ], [ %256, %.preheader3 ]
  %vectorPHI272 = phi <4 x i32> [ <i32 255, i32 255, i32 255, i32 255>, %.loopexit ], [ %257, %.preheader3 ]
  %150 = icmp sgt <4 x i32> %shuffleMerge156, %vectorPHI262
  %151 = icmp sgt <4 x i32> %shuffleMerge159, %vectorPHI263
  %152 = icmp sgt <4 x i32> %shuffleMerge162, %vectorPHI264
  %153 = sext <4 x i1> %150 to <4 x i32>
  %154 = sext <4 x i1> %151 to <4 x i32>
  %155 = sext <4 x i1> %152 to <4 x i32>
  %156 = icmp sgt <4 x i32> %shuffleMerge168, %vectorPHI262
  %157 = icmp sgt <4 x i32> %shuffleMerge171, %vectorPHI263
  %158 = icmp sgt <4 x i32> %shuffleMerge174, %vectorPHI264
  %159 = sext <4 x i1> %156 to <4 x i32>
  %160 = sext <4 x i1> %157 to <4 x i32>
  %161 = sext <4 x i1> %158 to <4 x i32>
  %162 = add <4 x i32> %153, %159
  %163 = add <4 x i32> %154, %160
  %164 = add <4 x i32> %155, %161
  %165 = icmp sgt <4 x i32> %shuffleMerge180, %vectorPHI262
  %166 = icmp sgt <4 x i32> %shuffleMerge183, %vectorPHI263
  %167 = icmp sgt <4 x i32> %shuffleMerge186, %vectorPHI264
  %168 = sext <4 x i1> %165 to <4 x i32>
  %169 = sext <4 x i1> %166 to <4 x i32>
  %170 = sext <4 x i1> %167 to <4 x i32>
  %171 = add <4 x i32> %162, %168
  %172 = add <4 x i32> %163, %169
  %173 = add <4 x i32> %164, %170
  %174 = icmp sgt <4 x i32> %shuffleMerge192, %vectorPHI262
  %175 = icmp sgt <4 x i32> %shuffleMerge195, %vectorPHI263
  %176 = icmp sgt <4 x i32> %shuffleMerge198, %vectorPHI264
  %177 = sext <4 x i1> %174 to <4 x i32>
  %178 = sext <4 x i1> %175 to <4 x i32>
  %179 = sext <4 x i1> %176 to <4 x i32>
  %180 = add <4 x i32> %171, %177
  %181 = add <4 x i32> %172, %178
  %182 = add <4 x i32> %173, %179
  %183 = icmp sgt <4 x i32> %shuffleMerge204, %vectorPHI262
  %184 = icmp sgt <4 x i32> %shuffleMerge207, %vectorPHI263
  %185 = icmp sgt <4 x i32> %shuffleMerge210, %vectorPHI264
  %186 = sext <4 x i1> %183 to <4 x i32>
  %187 = sext <4 x i1> %184 to <4 x i32>
  %188 = sext <4 x i1> %185 to <4 x i32>
  %189 = add <4 x i32> %180, %186
  %190 = add <4 x i32> %181, %187
  %191 = add <4 x i32> %182, %188
  %192 = icmp sgt <4 x i32> %shuffleMerge216, %vectorPHI262
  %193 = icmp sgt <4 x i32> %shuffleMerge219, %vectorPHI263
  %194 = icmp sgt <4 x i32> %shuffleMerge222, %vectorPHI264
  %195 = sext <4 x i1> %192 to <4 x i32>
  %196 = sext <4 x i1> %193 to <4 x i32>
  %197 = sext <4 x i1> %194 to <4 x i32>
  %198 = add <4 x i32> %189, %195
  %199 = add <4 x i32> %190, %196
  %200 = add <4 x i32> %191, %197
  %201 = icmp sgt <4 x i32> %shuffleMerge228, %vectorPHI262
  %202 = icmp sgt <4 x i32> %shuffleMerge231, %vectorPHI263
  %203 = icmp sgt <4 x i32> %shuffleMerge234, %vectorPHI264
  %204 = sext <4 x i1> %201 to <4 x i32>
  %205 = sext <4 x i1> %202 to <4 x i32>
  %206 = sext <4 x i1> %203 to <4 x i32>
  %207 = add <4 x i32> %198, %204
  %208 = add <4 x i32> %199, %205
  %209 = add <4 x i32> %200, %206
  %210 = icmp sgt <4 x i32> %shuffleMerge240, %vectorPHI262
  %211 = icmp sgt <4 x i32> %shuffleMerge243, %vectorPHI263
  %212 = icmp sgt <4 x i32> %shuffleMerge246, %vectorPHI264
  %213 = sext <4 x i1> %210 to <4 x i32>
  %214 = sext <4 x i1> %211 to <4 x i32>
  %215 = sext <4 x i1> %212 to <4 x i32>
  %216 = add <4 x i32> %207, %213
  %217 = add <4 x i32> %208, %214
  %218 = add <4 x i32> %209, %215
  %219 = icmp sgt <4 x i32> %shuffleMerge252, %vectorPHI262
  %220 = icmp sgt <4 x i32> %shuffleMerge255, %vectorPHI263
  %221 = icmp sgt <4 x i32> %shuffleMerge258, %vectorPHI264
  %222 = sext <4 x i1> %219 to <4 x i32>
  %223 = sext <4 x i1> %220 to <4 x i32>
  %224 = sext <4 x i1> %221 to <4 x i32>
  %225 = add <4 x i32> %216, %222
  %226 = add <4 x i32> %217, %223
  %227 = add <4 x i32> %218, %224
  %228 = sub <4 x i32> zeroinitializer, %225
  %229 = sub <4 x i32> zeroinitializer, %226
  %230 = sub <4 x i32> zeroinitializer, %227
  %231 = icmp sgt <4 x i32> %228, <i32 4, i32 4, i32 4, i32 4>
  %232 = icmp sgt <4 x i32> %229, <i32 4, i32 4, i32 4, i32 4>
  %233 = icmp sgt <4 x i32> %230, <i32 4, i32 4, i32 4, i32 4>
  %234 = sext <4 x i1> %231 to <4 x i32>
  %235 = sext <4 x i1> %232 to <4 x i32>
  %236 = sext <4 x i1> %233 to <4 x i32>
  %237 = and <4 x i32> %vectorPHI262, %234
  %238 = and <4 x i32> %vectorPHI263, %235
  %239 = and <4 x i32> %vectorPHI264, %236
  %240 = xor <4 x i32> %234, <i32 -1, i32 -1, i32 -1, i32 -1>
  %241 = xor <4 x i32> %235, <i32 -1, i32 -1, i32 -1, i32 -1>
  %242 = xor <4 x i32> %236, <i32 -1, i32 -1, i32 -1, i32 -1>
  %243 = and <4 x i32> %vectorPHI266, %240
  %244 = and <4 x i32> %vectorPHI267, %241
  %245 = and <4 x i32> %vectorPHI268, %242
  %246 = or <4 x i32> %237, %243
  %247 = or <4 x i32> %238, %244
  %248 = or <4 x i32> %239, %245
  %249 = and <4 x i32> %vectorPHI262, %240
  %250 = and <4 x i32> %vectorPHI263, %241
  %251 = and <4 x i32> %vectorPHI264, %242
  %252 = and <4 x i32> %vectorPHI270, %234
  %253 = and <4 x i32> %vectorPHI271, %235
  %254 = and <4 x i32> %vectorPHI272, %236
  %255 = or <4 x i32> %249, %252
  %256 = or <4 x i32> %250, %253
  %257 = or <4 x i32> %251, %254
  %258 = add <4 x i32> %255, %246
  %259 = add <4 x i32> %256, %247
  %260 = add <4 x i32> %257, %248
  %261 = ashr <4 x i32> %258, <i32 1, i32 1, i32 1, i32 1>
  %262 = ashr <4 x i32> %259, <i32 1, i32 1, i32 1, i32 1>
  %263 = ashr <4 x i32> %260, <i32 1, i32 1, i32 1, i32 1>
  %264 = add nsw i32 %iSearch.010, 1
  %exitcond13 = icmp eq i32 %264, 8
  br i1 %exitcond13, label %265, label %.preheader3

; <label>:265                                     ; preds = %.preheader3
  %266 = and <4 x i32> %261, <i32 255, i32 255, i32 255, i32 255>
  %267 = shl <4 x i32> %262, <i32 8, i32 8, i32 8, i32 8>
  %268 = and <4 x i32> %267, <i32 65280, i32 65280, i32 65280, i32 65280>
  %269 = shl <4 x i32> %263, <i32 16, i32 16, i32 16, i32 16>
  %270 = and <4 x i32> %269, <i32 16711680, i32 16711680, i32 16711680, i32 16711680>
  %271 = or <4 x i32> %270, %266
  %272 = or <4 x i32> %271, %268
  %extract278 = extractelement <4 x i32> %272, i32 0
  %extract279 = extractelement <4 x i32> %272, i32 1
  %extract280 = extractelement <4 x i32> %272, i32 2
  %extract281 = extractelement <4 x i32> %272, i32 3
  %273 = add nsw <4 x i32> %vector87, %6
  %extract274 = extractelement <4 x i32> %273, i32 0
  %extract275 = extractelement <4 x i32> %273, i32 1
  %extract276 = extractelement <4 x i32> %273, i32 2
  %extract277 = extractelement <4 x i32> %273, i32 3
  %274 = getelementptr inbounds i32 addrspace(1)* %pDst, i32 %extract274
  %275 = getelementptr inbounds i32 addrspace(1)* %pDst, i32 %extract275
  %276 = getelementptr inbounds i32 addrspace(1)* %pDst, i32 %extract276
  %277 = getelementptr inbounds i32 addrspace(1)* %pDst, i32 %extract277
  store i32 %extract278, i32 addrspace(1)* %274, align 4
  store i32 %extract279, i32 addrspace(1)* %275, align 4
  store i32 %extract280, i32 addrspace(1)* %276, align 4
  store i32 %extract281, i32 addrspace(1)* %277, align 4
  %278 = add nsw i32 %x.011, 1
  %exitcond14 = icmp eq i32 %278, %iImageWidth
  br i1 %exitcond14, label %._crit_edge, label %.preheader

._crit_edge:                                      ; preds = %265
  ret void
}

define void @__Vectorized_.intel_median_scalar(i8 addrspace(1)* nocapture %pSrc, i32 addrspace(1)* nocapture %pDst, i32 %iImageWidth, i32 %iImageHeight) nounwind {
.preheader11.lr.ph:
  %temp = insertelement <4 x i32> undef, i32 %iImageWidth, i32 0
  %vector = shufflevector <4 x i32> %temp, <4 x i32> undef, <4 x i32> zeroinitializer
  %0 = alloca [4 x i32], align 4
  %1 = alloca [4 x i32], align 4
  %2 = alloca [4 x i32], align 4
  %3 = alloca [4 x i32], align 4
  %4 = call i32 @get_global_id(i32 0) nounwind readnone
  %broadcast1 = insertelement <4 x i32> undef, i32 %4, i32 0
  %broadcast2 = shufflevector <4 x i32> %broadcast1, <4 x i32> undef, <4 x i32> zeroinitializer
  %5 = add <4 x i32> %broadcast2, <i32 2, i32 3, i32 4, i32 5>
  %6 = getelementptr inbounds [4 x i32]* %0, i32 0, i32 0
  %7 = getelementptr inbounds [4 x i32]* %1, i32 0, i32 0
  %8 = getelementptr inbounds [4 x i32]* %2, i32 0, i32 0
  %9 = getelementptr inbounds [4 x i32]* %3, i32 0, i32 0
  %10 = getelementptr inbounds [4 x i32]* %0, i32 0, i32 1
  %11 = getelementptr inbounds [4 x i32]* %1, i32 0, i32 1
  %12 = getelementptr inbounds [4 x i32]* %2, i32 0, i32 1
  %13 = getelementptr inbounds [4 x i32]* %3, i32 0, i32 1
  %14 = getelementptr inbounds [4 x i32]* %0, i32 0, i32 2
  %15 = getelementptr inbounds [4 x i32]* %1, i32 0, i32 2
  %16 = getelementptr inbounds [4 x i32]* %2, i32 0, i32 2
  %17 = getelementptr inbounds [4 x i32]* %3, i32 0, i32 2
  %18 = mul nsw <4 x i32> %5, %vector
  br label %.preheader11

.preheader11:                                     ; preds = %159, %.preheader11.lr.ph
  %x.013 = phi i32 [ 0, %.preheader11.lr.ph ], [ %184, %159 ]
  %temp1 = insertelement <4 x i32> undef, i32 %x.013, i32 0
  %vector2 = shufflevector <4 x i32> %temp1, <4 x i32> undef, <4 x i32> zeroinitializer
  br label %.preheader

.preheader:                                       ; preds = %153, %.preheader11
  %ch.012 = phi i32 [ 0, %.preheader11 ], [ %158, %153 ]
  %temp11 = insertelement <4 x i32> undef, i32 %ch.012, i32 0
  %vector12 = shufflevector <4 x i32> %temp11, <4 x i32> undef, <4 x i32> zeroinitializer
  %19 = add i32 %ch.012, -4
  %temp3 = insertelement <4 x i32> undef, i32 %19, i32 0
  %vector4 = shufflevector <4 x i32> %temp3, <4 x i32> undef, <4 x i32> zeroinitializer
  %20 = add i32 %ch.012, 4
  %temp21 = insertelement <4 x i32> undef, i32 %20, i32 0
  %vector22 = shufflevector <4 x i32> %temp21, <4 x i32> undef, <4 x i32> zeroinitializer
  %21 = add <4 x i32> %broadcast2, <i32 1, i32 2, i32 3, i32 4>
  %22 = mul nsw <4 x i32> %21, %vector
  %23 = add nsw <4 x i32> %22, %vector2
  %24 = shl <4 x i32> %23, <i32 2, i32 2, i32 2, i32 2>
  %25 = add <4 x i32> %vector4, %24
  %extract = extractelement <4 x i32> %25, i32 0
  %extract5 = extractelement <4 x i32> %25, i32 1
  %extract6 = extractelement <4 x i32> %25, i32 2
  %extract7 = extractelement <4 x i32> %25, i32 3
  %26 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract
  %27 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract5
  %28 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract6
  %29 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract7
  %30 = load i8 addrspace(1)* %26, align 1
  %31 = load i8 addrspace(1)* %27, align 1
  %32 = load i8 addrspace(1)* %28, align 1
  %33 = load i8 addrspace(1)* %29, align 1
  %temp.vect = insertelement <4 x i8> undef, i8 %30, i32 0
  %temp.vect8 = insertelement <4 x i8> %temp.vect, i8 %31, i32 1
  %temp.vect9 = insertelement <4 x i8> %temp.vect8, i8 %32, i32 2
  %temp.vect10 = insertelement <4 x i8> %temp.vect9, i8 %33, i32 3
  %34 = zext <4 x i8> %temp.vect10 to <4 x i32>
  %35 = add nsw <4 x i32> %24, %vector12
  %extract13 = extractelement <4 x i32> %35, i32 0
  %extract14 = extractelement <4 x i32> %35, i32 1
  %extract15 = extractelement <4 x i32> %35, i32 2
  %extract16 = extractelement <4 x i32> %35, i32 3
  %36 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract13
  %37 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract14
  %38 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract15
  %39 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract16
  %40 = load i8 addrspace(1)* %36, align 1
  %41 = load i8 addrspace(1)* %37, align 1
  %42 = load i8 addrspace(1)* %38, align 1
  %43 = load i8 addrspace(1)* %39, align 1
  %temp.vect17 = insertelement <4 x i8> undef, i8 %40, i32 0
  %temp.vect18 = insertelement <4 x i8> %temp.vect17, i8 %41, i32 1
  %temp.vect19 = insertelement <4 x i8> %temp.vect18, i8 %42, i32 2
  %temp.vect20 = insertelement <4 x i8> %temp.vect19, i8 %43, i32 3
  %44 = zext <4 x i8> %temp.vect20 to <4 x i32>
  %45 = add <4 x i32> %vector22, %24
  %extract23 = extractelement <4 x i32> %45, i32 0
  %extract24 = extractelement <4 x i32> %45, i32 1
  %extract25 = extractelement <4 x i32> %45, i32 2
  %extract26 = extractelement <4 x i32> %45, i32 3
  %46 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract23
  %47 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract24
  %48 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract25
  %49 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract26
  %50 = load i8 addrspace(1)* %46, align 1
  %51 = load i8 addrspace(1)* %47, align 1
  %52 = load i8 addrspace(1)* %48, align 1
  %53 = load i8 addrspace(1)* %49, align 1
  %temp.vect27 = insertelement <4 x i8> undef, i8 %50, i32 0
  %temp.vect28 = insertelement <4 x i8> %temp.vect27, i8 %51, i32 1
  %temp.vect29 = insertelement <4 x i8> %temp.vect28, i8 %52, i32 2
  %temp.vect30 = insertelement <4 x i8> %temp.vect29, i8 %53, i32 3
  %54 = zext <4 x i8> %temp.vect30 to <4 x i32>
  %55 = add nsw <4 x i32> %18, %vector2
  %56 = shl <4 x i32> %55, <i32 2, i32 2, i32 2, i32 2>
  %57 = add <4 x i32> %vector4, %56
  %extract31 = extractelement <4 x i32> %57, i32 0
  %extract32 = extractelement <4 x i32> %57, i32 1
  %extract33 = extractelement <4 x i32> %57, i32 2
  %extract34 = extractelement <4 x i32> %57, i32 3
  %58 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract31
  %59 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract32
  %60 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract33
  %61 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract34
  %62 = load i8 addrspace(1)* %58, align 1
  %63 = load i8 addrspace(1)* %59, align 1
  %64 = load i8 addrspace(1)* %60, align 1
  %65 = load i8 addrspace(1)* %61, align 1
  %temp.vect35 = insertelement <4 x i8> undef, i8 %62, i32 0
  %temp.vect36 = insertelement <4 x i8> %temp.vect35, i8 %63, i32 1
  %temp.vect37 = insertelement <4 x i8> %temp.vect36, i8 %64, i32 2
  %temp.vect38 = insertelement <4 x i8> %temp.vect37, i8 %65, i32 3
  %66 = zext <4 x i8> %temp.vect38 to <4 x i32>
  %67 = add nsw <4 x i32> %56, %vector12
  %extract39 = extractelement <4 x i32> %67, i32 0
  %extract40 = extractelement <4 x i32> %67, i32 1
  %extract41 = extractelement <4 x i32> %67, i32 2
  %extract42 = extractelement <4 x i32> %67, i32 3
  %68 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract39
  %69 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract40
  %70 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract41
  %71 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract42
  %72 = load i8 addrspace(1)* %68, align 1
  %73 = load i8 addrspace(1)* %69, align 1
  %74 = load i8 addrspace(1)* %70, align 1
  %75 = load i8 addrspace(1)* %71, align 1
  %temp.vect43 = insertelement <4 x i8> undef, i8 %72, i32 0
  %temp.vect44 = insertelement <4 x i8> %temp.vect43, i8 %73, i32 1
  %temp.vect45 = insertelement <4 x i8> %temp.vect44, i8 %74, i32 2
  %temp.vect46 = insertelement <4 x i8> %temp.vect45, i8 %75, i32 3
  %76 = zext <4 x i8> %temp.vect46 to <4 x i32>
  %77 = add <4 x i32> %vector22, %56
  %extract47 = extractelement <4 x i32> %77, i32 0
  %extract48 = extractelement <4 x i32> %77, i32 1
  %extract49 = extractelement <4 x i32> %77, i32 2
  %extract50 = extractelement <4 x i32> %77, i32 3
  %78 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract47
  %79 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract48
  %80 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract49
  %81 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract50
  %82 = load i8 addrspace(1)* %78, align 1
  %83 = load i8 addrspace(1)* %79, align 1
  %84 = load i8 addrspace(1)* %80, align 1
  %85 = load i8 addrspace(1)* %81, align 1
  %temp.vect51 = insertelement <4 x i8> undef, i8 %82, i32 0
  %temp.vect52 = insertelement <4 x i8> %temp.vect51, i8 %83, i32 1
  %temp.vect53 = insertelement <4 x i8> %temp.vect52, i8 %84, i32 2
  %temp.vect54 = insertelement <4 x i8> %temp.vect53, i8 %85, i32 3
  %86 = zext <4 x i8> %temp.vect54 to <4 x i32>
  %87 = add <4 x i32> %broadcast2, <i32 3, i32 4, i32 5, i32 6>
  %88 = mul nsw <4 x i32> %87, %vector
  %89 = add nsw <4 x i32> %88, %vector2
  %90 = shl <4 x i32> %89, <i32 2, i32 2, i32 2, i32 2>
  %91 = add <4 x i32> %vector4, %90
  %extract55 = extractelement <4 x i32> %91, i32 0
  %extract56 = extractelement <4 x i32> %91, i32 1
  %extract57 = extractelement <4 x i32> %91, i32 2
  %extract58 = extractelement <4 x i32> %91, i32 3
  %92 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract55
  %93 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract56
  %94 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract57
  %95 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract58
  %96 = load i8 addrspace(1)* %92, align 1
  %97 = load i8 addrspace(1)* %93, align 1
  %98 = load i8 addrspace(1)* %94, align 1
  %99 = load i8 addrspace(1)* %95, align 1
  %temp.vect59 = insertelement <4 x i8> undef, i8 %96, i32 0
  %temp.vect60 = insertelement <4 x i8> %temp.vect59, i8 %97, i32 1
  %temp.vect61 = insertelement <4 x i8> %temp.vect60, i8 %98, i32 2
  %temp.vect62 = insertelement <4 x i8> %temp.vect61, i8 %99, i32 3
  %100 = zext <4 x i8> %temp.vect62 to <4 x i32>
  %101 = add nsw <4 x i32> %90, %vector12
  %extract63 = extractelement <4 x i32> %101, i32 0
  %extract64 = extractelement <4 x i32> %101, i32 1
  %extract65 = extractelement <4 x i32> %101, i32 2
  %extract66 = extractelement <4 x i32> %101, i32 3
  %102 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract63
  %103 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract64
  %104 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract65
  %105 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract66
  %106 = load i8 addrspace(1)* %102, align 1
  %107 = load i8 addrspace(1)* %103, align 1
  %108 = load i8 addrspace(1)* %104, align 1
  %109 = load i8 addrspace(1)* %105, align 1
  %temp.vect67 = insertelement <4 x i8> undef, i8 %106, i32 0
  %temp.vect68 = insertelement <4 x i8> %temp.vect67, i8 %107, i32 1
  %temp.vect69 = insertelement <4 x i8> %temp.vect68, i8 %108, i32 2
  %temp.vect70 = insertelement <4 x i8> %temp.vect69, i8 %109, i32 3
  %110 = zext <4 x i8> %temp.vect70 to <4 x i32>
  %111 = add <4 x i32> %vector22, %90
  %extract71 = extractelement <4 x i32> %111, i32 0
  %extract72 = extractelement <4 x i32> %111, i32 1
  %extract73 = extractelement <4 x i32> %111, i32 2
  %extract74 = extractelement <4 x i32> %111, i32 3
  %112 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract71
  %113 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract72
  %114 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract73
  %115 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %extract74
  %116 = load i8 addrspace(1)* %112, align 1
  %117 = load i8 addrspace(1)* %113, align 1
  %118 = load i8 addrspace(1)* %114, align 1
  %119 = load i8 addrspace(1)* %115, align 1
  %temp.vect75 = insertelement <4 x i8> undef, i8 %116, i32 0
  %temp.vect76 = insertelement <4 x i8> %temp.vect75, i8 %117, i32 1
  %temp.vect77 = insertelement <4 x i8> %temp.vect76, i8 %118, i32 2
  %temp.vect78 = insertelement <4 x i8> %temp.vect77, i8 %119, i32 3
  %120 = zext <4 x i8> %temp.vect78 to <4 x i32>
  br label %.preheader3

.preheader3:                                      ; preds = %.preheader3, %.preheader
  %iSearch.010 = phi i32 [ 0, %.preheader ], [ %152, %.preheader3 ]
  %vectorPHI = phi <4 x i32> [ <i32 255, i32 255, i32 255, i32 255>, %.preheader ], [ %149, %.preheader3 ]
  %vectorPHI79 = phi <4 x i32> [ zeroinitializer, %.preheader ], [ %143, %.preheader3 ]
  %vectorPHI80 = phi <4 x i32> [ <i32 128, i32 128, i32 128, i32 128>, %.preheader ], [ %151, %.preheader3 ]
  %121 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %34, <4 x i32> %vectorPHI80)
  %msb = and <4 x i32> %121, <i32 1, i32 1, i32 1, i32 1>
  %122 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %44, <4 x i32> %vectorPHI80)
  %msb118 = and <4 x i32> %122, <i32 1, i32 1, i32 1, i32 1>
  %123 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %54, <4 x i32> %vectorPHI80)
  %msb120 = and <4 x i32> %123, <i32 1, i32 1, i32 1, i32 1>
  %124 = add <4 x i32> %msb, %msb118
  %125 = add <4 x i32> %124, %msb120
  %126 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %66, <4 x i32> %vectorPHI80)
  %msb122 = and <4 x i32> %126, <i32 1, i32 1, i32 1, i32 1>
  %127 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %76, <4 x i32> %vectorPHI80)
  %msb124 = and <4 x i32> %127, <i32 1, i32 1, i32 1, i32 1>
  %128 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %86, <4 x i32> %vectorPHI80)
  %msb126 = and <4 x i32> %128, <i32 1, i32 1, i32 1, i32 1>
  %129 = add <4 x i32> %msb122, %125
  %130 = add <4 x i32> %129, %msb124
  %131 = add <4 x i32> %130, %msb126
  %132 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %100, <4 x i32> %vectorPHI80)
  %msb128 = and <4 x i32> %132, <i32 1, i32 1, i32 1, i32 1>
  %133 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %110, <4 x i32> %vectorPHI80)
  %msb130 = and <4 x i32> %133, <i32 1, i32 1, i32 1, i32 1>
  %134 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %120, <4 x i32> %vectorPHI80)
  %msb132 = and <4 x i32> %134, <i32 1, i32 1, i32 1, i32 1>
  %135 = add <4 x i32> %msb128, %131
  %136 = add <4 x i32> %135, %msb130
  %137 = add <4 x i32> %136, %msb132
  %138 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> %137, <4 x i32> <i32 4, i32 4, i32 4, i32 4>)
  %139 = bitcast <4 x i32> %vectorPHI80 to <4 x float>
  %140 = bitcast <4 x i32> %vectorPHI79 to <4 x float>
  %141 = bitcast <4 x i32> %138 to <4 x float>
  %142 = call <4 x float> @llvm.x86.sse41.blendvps(<4 x float> %140, <4 x float> %139, <4 x float> %141)
  %143 = bitcast <4 x float> %142 to <4 x i32>
  %144 = call <4 x i32> @llvm.x86.sse2.pcmpgt.d(<4 x i32> <i32 5, i32 5, i32 5, i32 5>, <4 x i32> %137)
  %145 = bitcast <4 x i32> %vectorPHI80 to <4 x float>
  %146 = bitcast <4 x i32> %vectorPHI to <4 x float>
  %147 = bitcast <4 x i32> %144 to <4 x float>
  %148 = call <4 x float> @llvm.x86.sse41.blendvps(<4 x float> %146, <4 x float> %145, <4 x float> %147)
  %149 = bitcast <4 x float> %148 to <4 x i32>
  %150 = add nsw <4 x i32> %149, %143
  %151 = ashr <4 x i32> %150, <i32 1, i32 1, i32 1, i32 1>
  %152 = add nsw i32 %iSearch.010, 1
  %exitcond = icmp eq i32 %152, 8
  br i1 %exitcond, label %153, label %.preheader3

; <label>:153                                     ; preds = %.preheader3
  %extract84 = extractelement <4 x i32> %151, i32 3
  %extract83 = extractelement <4 x i32> %151, i32 2
  %extract82 = extractelement <4 x i32> %151, i32 1
  %extract81 = extractelement <4 x i32> %151, i32 0
  %154 = getelementptr inbounds [4 x i32]* %0, i32 0, i32 %ch.012
  %155 = getelementptr inbounds [4 x i32]* %1, i32 0, i32 %ch.012
  %156 = getelementptr inbounds [4 x i32]* %2, i32 0, i32 %ch.012
  %157 = getelementptr inbounds [4 x i32]* %3, i32 0, i32 %ch.012
  store i32 %extract81, i32* %154, align 4
  store i32 %extract82, i32* %155, align 4
  store i32 %extract83, i32* %156, align 4
  store i32 %extract84, i32* %157, align 4
  %158 = add nsw i32 %ch.012, 1
  %exitcond14 = icmp eq i32 %158, 3
  br i1 %exitcond14, label %159, label %.preheader

; <label>:159                                     ; preds = %153
  %160 = load i32* %6, align 4
  %161 = load i32* %7, align 4
  %162 = load i32* %8, align 4
  %163 = load i32* %9, align 4
  %temp.vect85 = insertelement <4 x i32> undef, i32 %160, i32 0
  %temp.vect86 = insertelement <4 x i32> %temp.vect85, i32 %161, i32 1
  %temp.vect87 = insertelement <4 x i32> %temp.vect86, i32 %162, i32 2
  %temp.vect88 = insertelement <4 x i32> %temp.vect87, i32 %163, i32 3
  %164 = and <4 x i32> %temp.vect88, <i32 255, i32 255, i32 255, i32 255>
  %165 = load i32* %10, align 4
  %166 = load i32* %11, align 4
  %167 = load i32* %12, align 4
  %168 = load i32* %13, align 4
  %temp.vect89 = insertelement <4 x i32> undef, i32 %165, i32 0
  %temp.vect90 = insertelement <4 x i32> %temp.vect89, i32 %166, i32 1
  %temp.vect91 = insertelement <4 x i32> %temp.vect90, i32 %167, i32 2
  %temp.vect92 = insertelement <4 x i32> %temp.vect91, i32 %168, i32 3
  %169 = shl <4 x i32> %temp.vect92, <i32 8, i32 8, i32 8, i32 8>
  %170 = and <4 x i32> %169, <i32 65280, i32 65280, i32 65280, i32 65280>
  %171 = load i32* %14, align 4
  %172 = load i32* %15, align 4
  %173 = load i32* %16, align 4
  %174 = load i32* %17, align 4
  %temp.vect93 = insertelement <4 x i32> undef, i32 %171, i32 0
  %temp.vect94 = insertelement <4 x i32> %temp.vect93, i32 %172, i32 1
  %temp.vect95 = insertelement <4 x i32> %temp.vect94, i32 %173, i32 2
  %temp.vect96 = insertelement <4 x i32> %temp.vect95, i32 %174, i32 3
  %175 = shl <4 x i32> %temp.vect96, <i32 16, i32 16, i32 16, i32 16>
  %176 = and <4 x i32> %175, <i32 16711680, i32 16711680, i32 16711680, i32 16711680>
  %177 = or <4 x i32> %170, %164
  %178 = or <4 x i32> %177, %176
  %extract101 = extractelement <4 x i32> %178, i32 0
  %extract102 = extractelement <4 x i32> %178, i32 1
  %extract103 = extractelement <4 x i32> %178, i32 2
  %extract104 = extractelement <4 x i32> %178, i32 3
  %179 = add nsw <4 x i32> %vector2, %18
  %extract97 = extractelement <4 x i32> %179, i32 0
  %extract98 = extractelement <4 x i32> %179, i32 1
  %extract99 = extractelement <4 x i32> %179, i32 2
  %extract100 = extractelement <4 x i32> %179, i32 3
  %180 = getelementptr inbounds i32 addrspace(1)* %pDst, i32 %extract97
  %181 = getelementptr inbounds i32 addrspace(1)* %pDst, i32 %extract98
  %182 = getelementptr inbounds i32 addrspace(1)* %pDst, i32 %extract99
  %183 = getelementptr inbounds i32 addrspace(1)* %pDst, i32 %extract100
  store i32 %extract101, i32 addrspace(1)* %180, align 4
  store i32 %extract102, i32 addrspace(1)* %181, align 4
  store i32 %extract103, i32 addrspace(1)* %182, align 4
  store i32 %extract104, i32 addrspace(1)* %183, align 4
  %184 = add nsw i32 %x.013, 1
  %exitcond15 = icmp eq i32 %184, %iImageWidth
  br i1 %exitcond15, label %._crit_edge, label %.preheader11

._crit_edge:                                      ; preds = %159
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
!5 = metadata !{metadata !"uchar4*", metadata !"uint*", metadata !"int", metadata !"int"}
!6 = metadata !{i32 0, i32 0, i32 0, i32 0}
!7 = metadata !{metadata !"pSrc", metadata !"pDst", metadata !"iImageWidth", metadata !"iImageHeight"}
!8 = metadata !{void (i8 addrspace(1)*, i32 addrspace(1)*, i32, i32)* @intel_median_scalar, metadata !1, metadata !9}
!9 = metadata !{metadata !"cl_kernel_arg_info", metadata !3, metadata !4, metadata !10, metadata !6, metadata !7}
!10 = metadata !{metadata !"uchar*", metadata !"uint*", metadata !"int", metadata !"int"}
!11 = metadata !{metadata !"-cl-std=CL1.2", metadata !"-cl-kernel-arg-info"}
!12 = metadata !{metadata !"intel_median", metadata !"intel_median_scalar"}
