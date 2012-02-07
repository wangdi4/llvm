; ModuleID = 'C:/Users/nrotem/Desktop/LLVM30_migration/src/backend/tests/opencl/SATest/GenerateIR/intel_median.1.tst.bin'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

define void @intel_median(<4 x i8> addrspace(1)* %pSrc, i32 addrspace(1)* %pDst, i32 %iImageWidth, i32 %iImageHeight) nounwind {
  %1 = alloca <4 x i8> addrspace(1)*, align 4
  %2 = alloca i32 addrspace(1)*, align 4
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %y = alloca i32, align 4
  %x = alloca i32, align 4
  %iYes = alloca <4 x i32>, align 16
  %iMin = alloca <4 x i32>, align 16
  %iMax = alloca <4 x i32>, align 16
  %ucRGBA = alloca <4 x i8>, align 4
  %uiZerro = alloca i32, align 4
  %iZerro = alloca <4 x i32>, align 16
  %iFour = alloca <4 x i32>, align 16
  %iMask = alloca <4 x i32>, align 16
  %iPixels = alloca [9 x <4 x i32>], align 16
  %iPixelCount = alloca i32, align 4
  %iRow = alloca i32, align 4
  %iLocalOffset = alloca i32, align 4
  %iSearch_max = alloca i32, align 4
  %iSearch = alloca i32, align 4
  %iHighCount = alloca <4 x i32>, align 16
  %iRow1 = alloca i32, align 4
  %uiPackedPixel = alloca i32, align 4
  store <4 x i8> addrspace(1)* %pSrc, <4 x i8> addrspace(1)** %1, align 4
  store i32 addrspace(1)* %pDst, i32 addrspace(1)** %2, align 4
  store i32 %iImageWidth, i32* %3, align 4
  store i32 %iImageHeight, i32* %4, align 4
  %5 = call i32 @get_global_id(i32 0)
  store i32 %5, i32* %y, align 4
  store i32 0, i32* %x, align 4
  br label %6

; <label>:6                                       ; preds = %208, %0
  %7 = load i32* %x, align 4
  %8 = load i32* %3, align 4
  %9 = icmp slt i32 %7, %8
  br i1 %9, label %10, label %211

; <label>:10                                      ; preds = %6
  store <4 x i32> <i32 128, i32 128, i32 128, i32 128>, <4 x i32>* %iYes, align 16
  store <4 x i32> zeroinitializer, <4 x i32>* %iMin, align 16
  store <4 x i32> <i32 255, i32 255, i32 255, i32 255>, <4 x i32>* %iMax, align 16
  store i32 0, i32* %uiZerro, align 4
  store <4 x i32> zeroinitializer, <4 x i32>* %iZerro, align 16
  store <4 x i32> <i32 4, i32 4, i32 4, i32 4>, <4 x i32>* %iFour, align 16
  store i32 0, i32* %iPixelCount, align 4
  store i32 -1, i32* %iRow, align 4
  br label %11

; <label>:11                                      ; preds = %106, %10
  %12 = load i32* %iRow, align 4
  %13 = icmp sle i32 %12, 1
  br i1 %13, label %14, label %109

; <label>:14                                      ; preds = %11
  %15 = load i32* %y, align 4
  %16 = load i32* %iRow, align 4
  %17 = add nsw i32 %15, %16
  %18 = add nsw i32 %17, 2
  %19 = load i32* %3, align 4
  %20 = mul nsw i32 %18, %19
  %21 = load i32* %x, align 4
  %22 = add nsw i32 %20, %21
  store i32 %22, i32* %iLocalOffset, align 4
  %23 = load i32* %iLocalOffset, align 4
  %24 = sub nsw i32 %23, 1
  %25 = load <4 x i8> addrspace(1)** %1, align 4
  %26 = getelementptr inbounds <4 x i8> addrspace(1)* %25, i32 %24
  %27 = load <4 x i8> addrspace(1)* %26
  store <4 x i8> %27, <4 x i8>* %ucRGBA, align 4
  %28 = load <4 x i8>* %ucRGBA
  %29 = extractelement <4 x i8> %28, i32 0
  %30 = zext i8 %29 to i32
  %31 = load i32* %iPixelCount, align 4
  %32 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 %31
  %33 = load <4 x i32>* %32
  %34 = insertelement <4 x i32> %33, i32 %30, i32 0
  store <4 x i32> %34, <4 x i32>* %32
  %35 = load <4 x i8>* %ucRGBA
  %36 = extractelement <4 x i8> %35, i32 1
  %37 = zext i8 %36 to i32
  %38 = load i32* %iPixelCount, align 4
  %39 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 %38
  %40 = load <4 x i32>* %39
  %41 = insertelement <4 x i32> %40, i32 %37, i32 1
  store <4 x i32> %41, <4 x i32>* %39
  %42 = load <4 x i8>* %ucRGBA
  %43 = extractelement <4 x i8> %42, i32 2
  %44 = zext i8 %43 to i32
  %45 = load i32* %iPixelCount, align 4
  %46 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 %45
  %47 = load <4 x i32>* %46
  %48 = insertelement <4 x i32> %47, i32 %44, i32 2
  store <4 x i32> %48, <4 x i32>* %46
  %49 = load i32* %iPixelCount, align 4
  %50 = add nsw i32 %49, 1
  store i32 %50, i32* %iPixelCount, align 4
  %51 = load i32* %iLocalOffset, align 4
  %52 = load <4 x i8> addrspace(1)** %1, align 4
  %53 = getelementptr inbounds <4 x i8> addrspace(1)* %52, i32 %51
  %54 = load <4 x i8> addrspace(1)* %53
  store <4 x i8> %54, <4 x i8>* %ucRGBA, align 4
  %55 = load <4 x i8>* %ucRGBA
  %56 = extractelement <4 x i8> %55, i32 0
  %57 = zext i8 %56 to i32
  %58 = load i32* %iPixelCount, align 4
  %59 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 %58
  %60 = load <4 x i32>* %59
  %61 = insertelement <4 x i32> %60, i32 %57, i32 0
  store <4 x i32> %61, <4 x i32>* %59
  %62 = load <4 x i8>* %ucRGBA
  %63 = extractelement <4 x i8> %62, i32 1
  %64 = zext i8 %63 to i32
  %65 = load i32* %iPixelCount, align 4
  %66 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 %65
  %67 = load <4 x i32>* %66
  %68 = insertelement <4 x i32> %67, i32 %64, i32 1
  store <4 x i32> %68, <4 x i32>* %66
  %69 = load <4 x i8>* %ucRGBA
  %70 = extractelement <4 x i8> %69, i32 2
  %71 = zext i8 %70 to i32
  %72 = load i32* %iPixelCount, align 4
  %73 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 %72
  %74 = load <4 x i32>* %73
  %75 = insertelement <4 x i32> %74, i32 %71, i32 2
  store <4 x i32> %75, <4 x i32>* %73
  %76 = load i32* %iPixelCount, align 4
  %77 = add nsw i32 %76, 1
  store i32 %77, i32* %iPixelCount, align 4
  %78 = load i32* %iLocalOffset, align 4
  %79 = add nsw i32 %78, 1
  %80 = load <4 x i8> addrspace(1)** %1, align 4
  %81 = getelementptr inbounds <4 x i8> addrspace(1)* %80, i32 %79
  %82 = load <4 x i8> addrspace(1)* %81
  store <4 x i8> %82, <4 x i8>* %ucRGBA, align 4
  %83 = load <4 x i8>* %ucRGBA
  %84 = extractelement <4 x i8> %83, i32 0
  %85 = zext i8 %84 to i32
  %86 = load i32* %iPixelCount, align 4
  %87 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 %86
  %88 = load <4 x i32>* %87
  %89 = insertelement <4 x i32> %88, i32 %85, i32 0
  store <4 x i32> %89, <4 x i32>* %87
  %90 = load <4 x i8>* %ucRGBA
  %91 = extractelement <4 x i8> %90, i32 1
  %92 = zext i8 %91 to i32
  %93 = load i32* %iPixelCount, align 4
  %94 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 %93
  %95 = load <4 x i32>* %94
  %96 = insertelement <4 x i32> %95, i32 %92, i32 1
  store <4 x i32> %96, <4 x i32>* %94
  %97 = load <4 x i8>* %ucRGBA
  %98 = extractelement <4 x i8> %97, i32 2
  %99 = zext i8 %98 to i32
  %100 = load i32* %iPixelCount, align 4
  %101 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 %100
  %102 = load <4 x i32>* %101
  %103 = insertelement <4 x i32> %102, i32 %99, i32 2
  store <4 x i32> %103, <4 x i32>* %101
  %104 = load i32* %iPixelCount, align 4
  %105 = add nsw i32 %104, 1
  store i32 %105, i32* %iPixelCount, align 4
  br label %106

; <label>:106                                     ; preds = %14
  %107 = load i32* %iRow, align 4
  %108 = add nsw i32 %107, 1
  store i32 %108, i32* %iRow, align 4
  br label %11

; <label>:109                                     ; preds = %11
  store i32 8, i32* %iSearch_max, align 4
  store i32 0, i32* %iSearch, align 4
  br label %110

; <label>:110                                     ; preds = %180, %109
  %111 = load i32* %iSearch, align 4
  %112 = load i32* %iSearch_max, align 4
  %113 = icmp slt i32 %111, %112
  br i1 %113, label %114, label %183

; <label>:114                                     ; preds = %110
  store <4 x i32> zeroinitializer, <4 x i32>* %iHighCount, align 16
  store i32 0, i32* %iPixelCount, align 4
  store i32 -1, i32* %iRow1, align 4
  br label %115

; <label>:115                                     ; preds = %149, %114
  %116 = load i32* %iRow1, align 4
  %117 = icmp sle i32 %116, 1
  br i1 %117, label %118, label %152

; <label>:118                                     ; preds = %115
  %119 = load <4 x i32>* %iYes, align 16
  %120 = load i32* %iPixelCount, align 4
  %121 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 %120
  %122 = load <4 x i32>* %121
  %123 = icmp slt <4 x i32> %119, %122
  %124 = sext <4 x i1> %123 to <4 x i32>
  %125 = load <4 x i32>* %iHighCount, align 16
  %126 = add nsw <4 x i32> %125, %124
  store <4 x i32> %126, <4 x i32>* %iHighCount, align 16
  %127 = load i32* %iPixelCount, align 4
  %128 = add nsw i32 %127, 1
  store i32 %128, i32* %iPixelCount, align 4
  %129 = load <4 x i32>* %iYes, align 16
  %130 = load i32* %iPixelCount, align 4
  %131 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 %130
  %132 = load <4 x i32>* %131
  %133 = icmp slt <4 x i32> %129, %132
  %134 = sext <4 x i1> %133 to <4 x i32>
  %135 = load <4 x i32>* %iHighCount, align 16
  %136 = add nsw <4 x i32> %135, %134
  store <4 x i32> %136, <4 x i32>* %iHighCount, align 16
  %137 = load i32* %iPixelCount, align 4
  %138 = add nsw i32 %137, 1
  store i32 %138, i32* %iPixelCount, align 4
  %139 = load <4 x i32>* %iYes, align 16
  %140 = load i32* %iPixelCount, align 4
  %141 = getelementptr inbounds [9 x <4 x i32>]* %iPixels, i32 0, i32 %140
  %142 = load <4 x i32>* %141
  %143 = icmp slt <4 x i32> %139, %142
  %144 = sext <4 x i1> %143 to <4 x i32>
  %145 = load <4 x i32>* %iHighCount, align 16
  %146 = add nsw <4 x i32> %145, %144
  store <4 x i32> %146, <4 x i32>* %iHighCount, align 16
  %147 = load i32* %iPixelCount, align 4
  %148 = add nsw i32 %147, 1
  store i32 %148, i32* %iPixelCount, align 4
  br label %149

; <label>:149                                     ; preds = %118
  %150 = load i32* %iRow1, align 4
  %151 = add nsw i32 %150, 1
  store i32 %151, i32* %iRow1, align 4
  br label %115

; <label>:152                                     ; preds = %115
  %153 = load <4 x i32>* %iZerro, align 16
  %154 = load <4 x i32>* %iHighCount, align 16
  %155 = sub nsw <4 x i32> %153, %154
  store <4 x i32> %155, <4 x i32>* %iHighCount, align 16
  %156 = load <4 x i32>* %iHighCount, align 16
  %157 = load <4 x i32>* %iFour, align 16
  %158 = icmp sgt <4 x i32> %156, %157
  %159 = sext <4 x i1> %158 to <4 x i32>
  store <4 x i32> %159, <4 x i32>* %iMask, align 16
  %160 = load <4 x i32>* %iYes, align 16
  %161 = load <4 x i32>* %iMask, align 16
  %162 = and <4 x i32> %160, %161
  %163 = load <4 x i32>* %iMin, align 16
  %164 = load <4 x i32>* %iMask, align 16
  %165 = xor <4 x i32> %164, <i32 -1, i32 -1, i32 -1, i32 -1>
  %166 = and <4 x i32> %163, %165
  %167 = or <4 x i32> %162, %166
  store <4 x i32> %167, <4 x i32>* %iMin, align 16
  %168 = load <4 x i32>* %iYes, align 16
  %169 = load <4 x i32>* %iMask, align 16
  %170 = xor <4 x i32> %169, <i32 -1, i32 -1, i32 -1, i32 -1>
  %171 = and <4 x i32> %168, %170
  %172 = load <4 x i32>* %iMax, align 16
  %173 = load <4 x i32>* %iMask, align 16
  %174 = and <4 x i32> %172, %173
  %175 = or <4 x i32> %171, %174
  store <4 x i32> %175, <4 x i32>* %iMax, align 16
  %176 = load <4 x i32>* %iMax, align 16
  %177 = load <4 x i32>* %iMin, align 16
  %178 = add nsw <4 x i32> %176, %177
  %179 = ashr <4 x i32> %178, <i32 1, i32 1, i32 1, i32 1>
  store <4 x i32> %179, <4 x i32>* %iYes, align 16
  br label %180

; <label>:180                                     ; preds = %152
  %181 = load i32* %iSearch, align 4
  %182 = add nsw i32 %181, 1
  store i32 %182, i32* %iSearch, align 4
  br label %110

; <label>:183                                     ; preds = %110
  %184 = load <4 x i32>* %iYes
  %185 = extractelement <4 x i32> %184, i32 0
  %186 = and i32 255, %185
  store i32 %186, i32* %uiPackedPixel, align 4
  %187 = load <4 x i32>* %iYes
  %188 = extractelement <4 x i32> %187, i32 1
  %189 = shl i32 %188, 8
  %190 = and i32 65280, %189
  %191 = load i32* %uiPackedPixel, align 4
  %192 = or i32 %191, %190
  store i32 %192, i32* %uiPackedPixel, align 4
  %193 = load <4 x i32>* %iYes
  %194 = extractelement <4 x i32> %193, i32 2
  %195 = shl i32 %194, 16
  %196 = and i32 16711680, %195
  %197 = load i32* %uiPackedPixel, align 4
  %198 = or i32 %197, %196
  store i32 %198, i32* %uiPackedPixel, align 4
  %199 = load i32* %uiPackedPixel, align 4
  %200 = load i32* %y, align 4
  %201 = add nsw i32 %200, 2
  %202 = load i32* %3, align 4
  %203 = mul nsw i32 %201, %202
  %204 = load i32* %x, align 4
  %205 = add nsw i32 %203, %204
  %206 = load i32 addrspace(1)** %2, align 4
  %207 = getelementptr inbounds i32 addrspace(1)* %206, i32 %205
  store i32 %199, i32 addrspace(1)* %207
  br label %208

; <label>:208                                     ; preds = %183
  %209 = load i32* %x, align 4
  %210 = add nsw i32 %209, 1
  store i32 %210, i32* %x, align 4
  br label %6

; <label>:211                                     ; preds = %6
  ret void
}

declare i32 @get_global_id(i32)

define void @intel_median_scalar(i8 addrspace(1)* %pSrc, i32 addrspace(1)* %pDst, i32 %iImageWidth, i32 %iImageHeight) nounwind {
  %1 = alloca i8 addrspace(1)*, align 4
  %2 = alloca i32 addrspace(1)*, align 4
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %y = alloca i32, align 4
  %x = alloca i32, align 4
  %uiZerro = alloca i32, align 4
  %iZerro = alloca i32, align 4
  %iFour = alloca i32, align 4
  %iResult = alloca [4 x i32], align 4
  %ch = alloca i32, align 4
  %iYes = alloca i32, align 4
  %iMin = alloca i32, align 4
  %iMax = alloca i32, align 4
  %iMask = alloca i32, align 4
  %iPixels = alloca [9 x i32], align 4
  %iPixelCount = alloca i32, align 4
  %iRow = alloca i32, align 4
  %iLocalOffset = alloca i32, align 4
  %iSearch_max = alloca i32, align 4
  %iSearch = alloca i32, align 4
  %iHighCount = alloca i32, align 4
  %iRow1 = alloca i32, align 4
  %uiPackedPixel = alloca i32, align 4
  store i8 addrspace(1)* %pSrc, i8 addrspace(1)** %1, align 4
  store i32 addrspace(1)* %pDst, i32 addrspace(1)** %2, align 4
  store i32 %iImageWidth, i32* %3, align 4
  store i32 %iImageHeight, i32* %4, align 4
  %5 = call i32 @get_global_id(i32 0)
  store i32 %5, i32* %y, align 4
  store i32 0, i32* %x, align 4
  br label %6

; <label>:6                                       ; preds = %163, %0
  %7 = load i32* %x, align 4
  %8 = load i32* %3, align 4
  %9 = icmp slt i32 %7, %8
  br i1 %9, label %10, label %166

; <label>:10                                      ; preds = %6
  store i32 0, i32* %uiZerro, align 4
  store i32 0, i32* %iZerro, align 4
  store i32 4, i32* %iFour, align 4
  store i32 0, i32* %ch, align 4
  br label %11

; <label>:11                                      ; preds = %135, %10
  %12 = load i32* %ch, align 4
  %13 = icmp slt i32 %12, 3
  br i1 %13, label %14, label %138

; <label>:14                                      ; preds = %11
  store i32 128, i32* %iYes, align 4
  store i32 0, i32* %iMin, align 4
  store i32 255, i32* %iMax, align 4
  store i32 0, i32* %iPixelCount, align 4
  store i32 -1, i32* %iRow, align 4
  br label %15

; <label>:15                                      ; preds = %65, %14
  %16 = load i32* %iRow, align 4
  %17 = icmp sle i32 %16, 1
  br i1 %17, label %18, label %68

; <label>:18                                      ; preds = %15
  %19 = load i32* %y, align 4
  %20 = load i32* %iRow, align 4
  %21 = add nsw i32 %19, %20
  %22 = add nsw i32 %21, 2
  %23 = load i32* %3, align 4
  %24 = mul nsw i32 %22, %23
  %25 = load i32* %x, align 4
  %26 = add nsw i32 %24, %25
  store i32 %26, i32* %iLocalOffset, align 4
  %27 = load i32* %iLocalOffset, align 4
  %28 = sub nsw i32 %27, 1
  %29 = mul nsw i32 %28, 4
  %30 = load i32* %ch, align 4
  %31 = add nsw i32 %29, %30
  %32 = load i8 addrspace(1)** %1, align 4
  %33 = getelementptr inbounds i8 addrspace(1)* %32, i32 %31
  %34 = load i8 addrspace(1)* %33
  %35 = zext i8 %34 to i32
  %36 = load i32* %iPixelCount, align 4
  %37 = getelementptr inbounds [9 x i32]* %iPixels, i32 0, i32 %36
  store i32 %35, i32* %37
  %38 = load i32* %iPixelCount, align 4
  %39 = add nsw i32 %38, 1
  store i32 %39, i32* %iPixelCount, align 4
  %40 = load i32* %iLocalOffset, align 4
  %41 = mul nsw i32 %40, 4
  %42 = load i32* %ch, align 4
  %43 = add nsw i32 %41, %42
  %44 = load i8 addrspace(1)** %1, align 4
  %45 = getelementptr inbounds i8 addrspace(1)* %44, i32 %43
  %46 = load i8 addrspace(1)* %45
  %47 = zext i8 %46 to i32
  %48 = load i32* %iPixelCount, align 4
  %49 = getelementptr inbounds [9 x i32]* %iPixels, i32 0, i32 %48
  store i32 %47, i32* %49
  %50 = load i32* %iPixelCount, align 4
  %51 = add nsw i32 %50, 1
  store i32 %51, i32* %iPixelCount, align 4
  %52 = load i32* %iLocalOffset, align 4
  %53 = add nsw i32 %52, 1
  %54 = mul nsw i32 %53, 4
  %55 = load i32* %ch, align 4
  %56 = add nsw i32 %54, %55
  %57 = load i8 addrspace(1)** %1, align 4
  %58 = getelementptr inbounds i8 addrspace(1)* %57, i32 %56
  %59 = load i8 addrspace(1)* %58
  %60 = zext i8 %59 to i32
  %61 = load i32* %iPixelCount, align 4
  %62 = getelementptr inbounds [9 x i32]* %iPixels, i32 0, i32 %61
  store i32 %60, i32* %62
  %63 = load i32* %iPixelCount, align 4
  %64 = add nsw i32 %63, 1
  store i32 %64, i32* %iPixelCount, align 4
  br label %65

; <label>:65                                      ; preds = %18
  %66 = load i32* %iRow, align 4
  %67 = add nsw i32 %66, 1
  store i32 %67, i32* %iRow, align 4
  br label %15

; <label>:68                                      ; preds = %15
  store i32 8, i32* %iSearch_max, align 4
  store i32 0, i32* %iSearch, align 4
  br label %69

; <label>:69                                      ; preds = %128, %68
  %70 = load i32* %iSearch, align 4
  %71 = load i32* %iSearch_max, align 4
  %72 = icmp slt i32 %70, %71
  br i1 %72, label %73, label %131

; <label>:73                                      ; preds = %69
  store i32 0, i32* %iHighCount, align 4
  store i32 0, i32* %iPixelCount, align 4
  store i32 -1, i32* %iRow1, align 4
  br label %74

; <label>:74                                      ; preds = %108, %73
  %75 = load i32* %iRow1, align 4
  %76 = icmp sle i32 %75, 1
  br i1 %76, label %77, label %111

; <label>:77                                      ; preds = %74
  %78 = load i32* %iYes, align 4
  %79 = load i32* %iPixelCount, align 4
  %80 = getelementptr inbounds [9 x i32]* %iPixels, i32 0, i32 %79
  %81 = load i32* %80
  %82 = icmp slt i32 %78, %81
  %83 = zext i1 %82 to i32
  %84 = load i32* %iHighCount, align 4
  %85 = add nsw i32 %84, %83
  store i32 %85, i32* %iHighCount, align 4
  %86 = load i32* %iPixelCount, align 4
  %87 = add nsw i32 %86, 1
  store i32 %87, i32* %iPixelCount, align 4
  %88 = load i32* %iYes, align 4
  %89 = load i32* %iPixelCount, align 4
  %90 = getelementptr inbounds [9 x i32]* %iPixels, i32 0, i32 %89
  %91 = load i32* %90
  %92 = icmp slt i32 %88, %91
  %93 = zext i1 %92 to i32
  %94 = load i32* %iHighCount, align 4
  %95 = add nsw i32 %94, %93
  store i32 %95, i32* %iHighCount, align 4
  %96 = load i32* %iPixelCount, align 4
  %97 = add nsw i32 %96, 1
  store i32 %97, i32* %iPixelCount, align 4
  %98 = load i32* %iYes, align 4
  %99 = load i32* %iPixelCount, align 4
  %100 = getelementptr inbounds [9 x i32]* %iPixels, i32 0, i32 %99
  %101 = load i32* %100
  %102 = icmp slt i32 %98, %101
  %103 = zext i1 %102 to i32
  %104 = load i32* %iHighCount, align 4
  %105 = add nsw i32 %104, %103
  store i32 %105, i32* %iHighCount, align 4
  %106 = load i32* %iPixelCount, align 4
  %107 = add nsw i32 %106, 1
  store i32 %107, i32* %iPixelCount, align 4
  br label %108

; <label>:108                                     ; preds = %77
  %109 = load i32* %iRow1, align 4
  %110 = add nsw i32 %109, 1
  store i32 %110, i32* %iRow1, align 4
  br label %74

; <label>:111                                     ; preds = %74
  %112 = load i32* %iHighCount, align 4
  %113 = load i32* %iFour, align 4
  %114 = icmp sgt i32 %112, %113
  %115 = load i32* %iYes, align 4
  %116 = load i32* %iMin, align 4
  %117 = select i1 %114, i32 %115, i32 %116
  store i32 %117, i32* %iMin, align 4
  %118 = load i32* %iHighCount, align 4
  %119 = load i32* %iFour, align 4
  %120 = icmp sle i32 %118, %119
  %121 = load i32* %iYes, align 4
  %122 = load i32* %iMax, align 4
  %123 = select i1 %120, i32 %121, i32 %122
  store i32 %123, i32* %iMax, align 4
  %124 = load i32* %iMax, align 4
  %125 = load i32* %iMin, align 4
  %126 = add nsw i32 %124, %125
  %127 = ashr i32 %126, 1
  store i32 %127, i32* %iYes, align 4
  br label %128

; <label>:128                                     ; preds = %111
  %129 = load i32* %iSearch, align 4
  %130 = add nsw i32 %129, 1
  store i32 %130, i32* %iSearch, align 4
  br label %69

; <label>:131                                     ; preds = %69
  %132 = load i32* %iYes, align 4
  %133 = load i32* %ch, align 4
  %134 = getelementptr inbounds [4 x i32]* %iResult, i32 0, i32 %133
  store i32 %132, i32* %134
  br label %135

; <label>:135                                     ; preds = %131
  %136 = load i32* %ch, align 4
  %137 = add nsw i32 %136, 1
  store i32 %137, i32* %ch, align 4
  br label %11

; <label>:138                                     ; preds = %11
  %139 = getelementptr inbounds [4 x i32]* %iResult, i32 0, i32 0
  %140 = load i32* %139
  %141 = and i32 255, %140
  store i32 %141, i32* %uiPackedPixel, align 4
  %142 = getelementptr inbounds [4 x i32]* %iResult, i32 0, i32 1
  %143 = load i32* %142
  %144 = shl i32 %143, 8
  %145 = and i32 65280, %144
  %146 = load i32* %uiPackedPixel, align 4
  %147 = or i32 %146, %145
  store i32 %147, i32* %uiPackedPixel, align 4
  %148 = getelementptr inbounds [4 x i32]* %iResult, i32 0, i32 2
  %149 = load i32* %148
  %150 = shl i32 %149, 16
  %151 = and i32 16711680, %150
  %152 = load i32* %uiPackedPixel, align 4
  %153 = or i32 %152, %151
  store i32 %153, i32* %uiPackedPixel, align 4
  %154 = load i32* %uiPackedPixel, align 4
  %155 = load i32* %y, align 4
  %156 = add nsw i32 %155, 2
  %157 = load i32* %3, align 4
  %158 = mul nsw i32 %156, %157
  %159 = load i32* %x, align 4
  %160 = add nsw i32 %158, %159
  %161 = load i32 addrspace(1)** %2, align 4
  %162 = getelementptr inbounds i32 addrspace(1)* %161, i32 %160
  store i32 %154, i32 addrspace(1)* %162
  br label %163

; <label>:163                                     ; preds = %138
  %164 = load i32* %x, align 4
  %165 = add nsw i32 %164, 1
  store i32 %165, i32* %x, align 4
  br label %6

; <label>:166                                     ; preds = %6
  ret void
}

!opencl.kernels = !{!0, !2}

!0 = metadata !{void (<4 x i8> addrspace(1)*, i32 addrspace(1)*, i32, i32)* @intel_median, metadata !1, metadata !1, metadata !"", metadata !"uchar4 __attribute__((address_space(1))) *, unsigned int __attribute__((address_space(1))) *, int, int", metadata !"opencl_intel_median_locals_anchor"}
!1 = metadata !{i32 0, i32 0, i32 0}
!2 = metadata !{void (i8 addrspace(1)*, i32 addrspace(1)*, i32, i32)* @intel_median_scalar, metadata !1, metadata !1, metadata !"", metadata !"uchar __attribute__((address_space(1))) *, unsigned int __attribute__((address_space(1))) *, int, int", metadata !"opencl_intel_median_scalar_locals_anchor"}
