; ModuleID = 'Program'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

define void @intel_median(<4 x i8> addrspace(1)* %pSrc, i32 addrspace(1)* %pDst, i32 %iImageWidth, i32 %iImageHeight) nounwind {
; <label>:0
  %iPixels = alloca [9 x <4 x i32>], align 16
  %1 = call i32 @get_global_id(i32 0) nounwind
  br label %2

; <label>:2                                       ; preds = %101, %0
  %x.0 = phi i32 [ 0, %0 ], [ %116, %101 ]
  %3 = icmp slt i32 %x.0, %iImageWidth
  br i1 %3, label %4, label %117

; <label>:4                                       ; preds = %2
  br label %5

; <label>:5                                       ; preds = %7, %4
  %iPixelCount.0 = phi i32 [ 0, %4 ], [ %58, %7 ]
  %iRow.0 = phi i32 [ -1, %4 ], [ %59, %7 ]
  %6 = icmp slt i32 %iRow.0, 2
  br i1 %6, label %7, label %60

; <label>:7                                       ; preds = %5
  %8 = add nsw i32 %1, %iRow.0
  %9 = add nsw i32 %8, 2
  %10 = mul nsw i32 %9, %iImageWidth
  %11 = add nsw i32 %10, %x.0
  %12 = add nsw i32 %11, -1
  %13 = getelementptr inbounds <4 x i8> addrspace(1)* %pSrc, i32 %12
  %14 = load <4 x i8> addrspace(1)* %13
  %15 = extractelement <4 x i8> %14, i32 0
  %16 = zext i8 %15 to i32
  %17 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 %iPixelCount.0
  %18 = load <4 x i32>* %17
  %19 = insertelement <4 x i32> %18, i32 %16, i32 0
  %20 = extractelement <4 x i8> %14, i32 1
  %21 = zext i8 %20 to i32
  %22 = insertelement <4 x i32> %19, i32 %21, i32 1
  %23 = extractelement <4 x i8> %14, i32 2
  %24 = zext i8 %23 to i32
  %25 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 %iPixelCount.0
  %26 = insertelement <4 x i32> %22, i32 %24, i32 2
  store <4 x i32> %26, <4 x i32>* %25
  %27 = add nsw i32 %iPixelCount.0, 1
  %28 = getelementptr inbounds <4 x i8> addrspace(1)* %pSrc, i32 %11
  %29 = load <4 x i8> addrspace(1)* %28
  %30 = extractelement <4 x i8> %29, i32 0
  %31 = zext i8 %30 to i32
  %32 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 %27
  %33 = load <4 x i32>* %32
  %34 = insertelement <4 x i32> %33, i32 %31, i32 0
  %35 = extractelement <4 x i8> %29, i32 1
  %36 = zext i8 %35 to i32
  %37 = insertelement <4 x i32> %34, i32 %36, i32 1
  %38 = extractelement <4 x i8> %29, i32 2
  %39 = zext i8 %38 to i32
  %40 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 %27
  %41 = insertelement <4 x i32> %37, i32 %39, i32 2
  store <4 x i32> %41, <4 x i32>* %40
  %42 = add nsw i32 %iPixelCount.0, 2
  %43 = add nsw i32 %11, 1
  %44 = getelementptr inbounds <4 x i8> addrspace(1)* %pSrc, i32 %43
  %45 = load <4 x i8> addrspace(1)* %44
  %46 = extractelement <4 x i8> %45, i32 0
  %47 = zext i8 %46 to i32
  %48 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 %42
  %49 = load <4 x i32>* %48
  %50 = insertelement <4 x i32> %49, i32 %47, i32 0
  %51 = extractelement <4 x i8> %45, i32 1
  %52 = zext i8 %51 to i32
  %53 = insertelement <4 x i32> %50, i32 %52, i32 1
  %54 = extractelement <4 x i8> %45, i32 2
  %55 = zext i8 %54 to i32
  %56 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 %42
  %57 = insertelement <4 x i32> %53, i32 %55, i32 2
  store <4 x i32> %57, <4 x i32>* %56
  %58 = add nsw i32 %iPixelCount.0, 3
  %59 = add nsw i32 %iRow.0, 1
  br label %5

; <label>:60                                      ; preds = %5
  br label %61

; <label>:61                                      ; preds = %86, %60
  %iMax.0 = phi <4 x i32> [ <i32 255, i32 255, i32 255, i32 255>, %60 ], [ %97, %86 ]
  %iMin.0 = phi <4 x i32> [ zeroinitializer, %60 ], [ %93, %86 ]
  %iYes.0 = phi <4 x i32> [ <i32 128, i32 128, i32 128, i32 128>, %60 ], [ %99, %86 ]
  %iSearch.0 = phi i32 [ 0, %60 ], [ %100, %86 ]
  %62 = icmp slt i32 %iSearch.0, 8
  br i1 %62, label %63, label %101

; <label>:63                                      ; preds = %61
  br label %64

; <label>:64                                      ; preds = %66, %63
  %iPixelCount.1 = phi i32 [ 0, %63 ], [ %84, %66 ]
  %iHighCount.0 = phi <4 x i32> [ zeroinitializer, %63 ], [ %83, %66 ]
  %iRow1.0 = phi i32 [ -1, %63 ], [ %85, %66 ]
  %65 = icmp slt i32 %iRow1.0, 2
  br i1 %65, label %66, label %86

; <label>:66                                      ; preds = %64
  %67 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 %iPixelCount.1
  %68 = load <4 x i32>* %67
  %69 = icmp slt <4 x i32> %iYes.0, %68
  %70 = sext <4 x i1> %69 to <4 x i32>
  %71 = add nsw <4 x i32> %iHighCount.0, %70
  %72 = add nsw i32 %iPixelCount.1, 1
  %73 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 %72
  %74 = load <4 x i32>* %73
  %75 = icmp slt <4 x i32> %iYes.0, %74
  %76 = sext <4 x i1> %75 to <4 x i32>
  %77 = add nsw <4 x i32> %71, %76
  %78 = add nsw i32 %iPixelCount.1, 2
  %79 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 %78
  %80 = load <4 x i32>* %79
  %81 = icmp slt <4 x i32> %iYes.0, %80
  %82 = sext <4 x i1> %81 to <4 x i32>
  %83 = add nsw <4 x i32> %77, %82
  %84 = add nsw i32 %iPixelCount.1, 3
  %85 = add nsw i32 %iRow1.0, 1
  br label %64

; <label>:86                                      ; preds = %64
  %87 = sub nsw <4 x i32> zeroinitializer, %iHighCount.0
  %88 = icmp sgt <4 x i32> %87, <i32 4, i32 4, i32 4, i32 4>
  %89 = sext <4 x i1> %88 to <4 x i32>
  %90 = and <4 x i32> %iYes.0, %89
  %91 = xor <4 x i32> %89, <i32 -1, i32 -1, i32 -1, i32 -1>
  %92 = and <4 x i32> %iMin.0, %91
  %93 = or <4 x i32> %90, %92
  %94 = xor <4 x i32> %89, <i32 -1, i32 -1, i32 -1, i32 -1>
  %95 = and <4 x i32> %iYes.0, %94
  %96 = and <4 x i32> %iMax.0, %89
  %97 = or <4 x i32> %95, %96
  %98 = add nsw <4 x i32> %97, %93
  %99 = ashr <4 x i32> %98, <i32 1, i32 1, i32 1, i32 1>
  %100 = add nsw i32 %iSearch.0, 1
  br label %61

; <label>:101                                     ; preds = %61
  %102 = extractelement <4 x i32> %iYes.0, i32 0
  %103 = and i32 %102, 255
  %104 = extractelement <4 x i32> %iYes.0, i32 1
  %105 = shl i32 %104, 8
  %106 = and i32 %105, 65280
  %107 = or i32 %103, %106
  %108 = extractelement <4 x i32> %iYes.0, i32 2
  %109 = shl i32 %108, 16
  %110 = and i32 %109, 16711680
  %111 = or i32 %107, %110
  %112 = add nsw i32 %1, 2
  %113 = mul nsw i32 %112, %iImageWidth
  %114 = add nsw i32 %113, %x.0
  %115 = getelementptr inbounds i32 addrspace(1)* %pDst, i32 %114
  store i32 %111, i32 addrspace(1)* %115
  %116 = add nsw i32 %x.0, 1
  br label %2

; <label>:117                                     ; preds = %2
  ret void
}

declare i32 @get_global_id(i32)

define void @intel_median_scalar(i8 addrspace(1)* %pSrc, i32 addrspace(1)* %pDst, i32 %iImageWidth, i32 %iImageHeight) nounwind {
; <label>:0
  %iResult = alloca [4 x i32], align 4
  %iPixels = alloca [9 x i32], align 4
  %1 = call i32 @get_global_id(i32 0) nounwind
  br label %2

; <label>:2                                       ; preds = %76, %0
  %x.0 = phi i32 [ 0, %0 ], [ %94, %76 ]
  %3 = icmp slt i32 %x.0, %iImageWidth
  br i1 %3, label %4, label %95

; <label>:4                                       ; preds = %2
  br label %5

; <label>:5                                       ; preds = %73, %4
  %ch.0 = phi i32 [ 0, %4 ], [ %75, %73 ]
  %6 = icmp slt i32 %ch.0, 3
  br i1 %6, label %7, label %76

; <label>:7                                       ; preds = %5
  br label %8

; <label>:8                                       ; preds = %10, %7
  %iPixelCount.0 = phi i32 [ 0, %7 ], [ %37, %10 ]
  %iRow.0 = phi i32 [ -1, %7 ], [ %38, %10 ]
  %9 = icmp slt i32 %iRow.0, 2
  br i1 %9, label %10, label %39

; <label>:10                                      ; preds = %8
  %11 = add nsw i32 %1, %iRow.0
  %12 = add nsw i32 %11, 2
  %13 = mul nsw i32 %12, %iImageWidth
  %14 = add nsw i32 %13, %x.0
  %15 = shl i32 %14, 2
  %16 = add i32 %15, -4
  %17 = add nsw i32 %16, %ch.0
  %18 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %17
  %19 = load i8 addrspace(1)* %18
  %20 = zext i8 %19 to i32
  %21 = getelementptr inbounds [9 x i32]* %iPixels, i32 0, i32 %iPixelCount.0
  store i32 %20, i32* %21
  %22 = add nsw i32 %iPixelCount.0, 1
  %23 = shl i32 %14, 2
  %24 = add nsw i32 %23, %ch.0
  %25 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %24
  %26 = load i8 addrspace(1)* %25
  %27 = zext i8 %26 to i32
  %28 = getelementptr inbounds [9 x i32]* %iPixels, i32 0, i32 %22
  store i32 %27, i32* %28
  %29 = add nsw i32 %iPixelCount.0, 2
  %30 = shl i32 %14, 2
  %31 = add i32 %30, 4
  %32 = add nsw i32 %31, %ch.0
  %33 = getelementptr inbounds i8 addrspace(1)* %pSrc, i32 %32
  %34 = load i8 addrspace(1)* %33
  %35 = zext i8 %34 to i32
  %36 = getelementptr inbounds [9 x i32]* %iPixels, i32 0, i32 %29
  store i32 %35, i32* %36
  %37 = add nsw i32 %iPixelCount.0, 3
  %38 = add nsw i32 %iRow.0, 1
  br label %8

; <label>:39                                      ; preds = %8
  br label %40

; <label>:40                                      ; preds = %65, %39
  %iYes.0 = phi i32 [ 128, %39 ], [ %71, %65 ]
  %iMin.0 = phi i32 [ 0, %39 ], [ %67, %65 ]
  %iMax.0 = phi i32 [ 255, %39 ], [ %69, %65 ]
  %iSearch.0 = phi i32 [ 0, %39 ], [ %72, %65 ]
  %41 = icmp slt i32 %iSearch.0, 8
  br i1 %41, label %42, label %73

; <label>:42                                      ; preds = %40
  br label %43

; <label>:43                                      ; preds = %45, %42
  %iPixelCount.1 = phi i32 [ 0, %42 ], [ %63, %45 ]
  %iHighCount.0 = phi i32 [ 0, %42 ], [ %62, %45 ]
  %iRow1.0 = phi i32 [ -1, %42 ], [ %64, %45 ]
  %44 = icmp slt i32 %iRow1.0, 2
  br i1 %44, label %45, label %65

; <label>:45                                      ; preds = %43
  %46 = getelementptr inbounds [9 x i32]* %iPixels, i32 0, i32 %iPixelCount.1
  %47 = load i32* %46
  %48 = icmp slt i32 %iYes.0, %47
  %49 = zext i1 %48 to i32
  %50 = add nsw i32 %iHighCount.0, %49
  %51 = add nsw i32 %iPixelCount.1, 1
  %52 = getelementptr inbounds [9 x i32]* %iPixels, i32 0, i32 %51
  %53 = load i32* %52
  %54 = icmp slt i32 %iYes.0, %53
  %55 = zext i1 %54 to i32
  %56 = add nsw i32 %50, %55
  %57 = add nsw i32 %iPixelCount.1, 2
  %58 = getelementptr inbounds [9 x i32]* %iPixels, i32 0, i32 %57
  %59 = load i32* %58
  %60 = icmp slt i32 %iYes.0, %59
  %61 = zext i1 %60 to i32
  %62 = add nsw i32 %56, %61
  %63 = add nsw i32 %iPixelCount.1, 3
  %64 = add nsw i32 %iRow1.0, 1
  br label %43

; <label>:65                                      ; preds = %43
  %66 = icmp sgt i32 %iHighCount.0, 4
  %67 = select i1 %66, i32 %iYes.0, i32 %iMin.0
  %68 = icmp slt i32 %iHighCount.0, 5
  %69 = select i1 %68, i32 %iYes.0, i32 %iMax.0
  %70 = add nsw i32 %69, %67
  %71 = ashr i32 %70, 1
  %72 = add nsw i32 %iSearch.0, 1
  br label %40

; <label>:73                                      ; preds = %40
  %74 = getelementptr inbounds [4 x i32]* %iResult, i32 0, i32 %ch.0
  store i32 %iYes.0, i32* %74
  %75 = add nsw i32 %ch.0, 1
  br label %5

; <label>:76                                      ; preds = %5
  %77 = getelementptr inbounds [4 x i32]* %iResult, i32 0, i32 0
  %78 = load i32* %77
  %79 = and i32 %78, 255
  %80 = getelementptr inbounds [4 x i32]* %iResult, i32 0, i32 1
  %81 = load i32* %80
  %82 = shl i32 %81, 8
  %83 = and i32 %82, 65280
  %84 = or i32 %79, %83
  %85 = getelementptr inbounds [4 x i32]* %iResult, i32 0, i32 2
  %86 = load i32* %85
  %87 = shl i32 %86, 16
  %88 = and i32 %87, 16711680
  %89 = or i32 %84, %88
  %90 = add nsw i32 %1, 2
  %91 = mul nsw i32 %90, %iImageWidth
  %92 = add nsw i32 %91, %x.0
  %93 = getelementptr inbounds i32 addrspace(1)* %pDst, i32 %92
  store i32 %89, i32 addrspace(1)* %93
  %94 = add nsw i32 %x.0, 1
  br label %2

; <label>:95                                      ; preds = %2
  ret void
}

!opencl.kernels = !{!0, !2}

!0 = metadata !{void (<4 x i8> addrspace(1)*, i32 addrspace(1)*, i32, i32)* @intel_median, metadata !1, metadata !1, metadata !"", metadata !"uchar4 __attribute__((address_space(1))) *, unsigned int __attribute__((address_space(1))) *, int, int", metadata !"opencl_intel_median_locals_anchor"}
!1 = metadata !{i32 0, i32 0, i32 0}
!2 = metadata !{void (i8 addrspace(1)*, i32 addrspace(1)*, i32, i32)* @intel_median_scalar, metadata !1, metadata !1, metadata !"", metadata !"uchar __attribute__((address_space(1))) *, unsigned int __attribute__((address_space(1))) *, int, int", metadata !"opencl_intel_median_scalar_locals_anchor"}
