; CL source code of test 111017_AsyncBug.tst.cl : 
; TODO: When SATest ready to work with cl files used following function as cl test
;
; __kernel void async_copy_test(__global uchar* buf_in1, __global uchar* buf_out1, __local uchar* local1, __local uchar* local2)
; {
;     int checker = 0;
;     event_t evs;
;     if (get_local_id(0) == 0) 
;    {
;        *local1 = 10;
;        *local2 = 11;
;    }
;
;    evs = async_work_group_copy((__local uchar*)local1, (__global uchar*)(buf_in1+2), sizeof(uchar), 0);
;    wait_group_events(1, &evs);
;    checker = *local1;
;    evs = async_work_group_copy((__global uchar*)buf_out1, (__local uchar*)local2, sizeof(uchar), 0);
;    wait_group_events(1, &evs);
;    checker = *buf_out1;
;    if (get_global_id(0) == 0) 
;        if (checker != 11)
;            //THIS LINE SHOULDN'T BE REACHED ALTHOUGH IT DOES
;            printf("2 FAIL \n");
;    checker++;
;}


; ModuleID = 'async_copy_test.bin'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

@.str = private addrspace(2) constant [9 x i8] c"2 FAIL \0A\00"

define void @async_copy_test(i8 addrspace(1)* %buf_in1, i8 addrspace(1)* %buf_out1, i8 addrspace(3)* %local1, i8 addrspace(3)* %local2) nounwind {
  %1 = alloca i8 addrspace(1)*, align 4
  %2 = alloca i8 addrspace(1)*, align 4
  %3 = alloca i8 addrspace(3)*, align 4
  %4 = alloca i8 addrspace(3)*, align 4
  %checker = alloca i32, align 4
  %evs = alloca i32, align 4
  store i8 addrspace(1)* %buf_in1, i8 addrspace(1)** %1, align 4
  store i8 addrspace(1)* %buf_out1, i8 addrspace(1)** %2, align 4
  store i8 addrspace(3)* %local1, i8 addrspace(3)** %3, align 4
  store i8 addrspace(3)* %local2, i8 addrspace(3)** %4, align 4
  store i32 0, i32* %checker, align 4
  %5 = call i32 @get_local_id(i32 0)
  %6 = icmp eq i32 %5, 0
  br i1 %6, label %7, label %10

; <label>:7                                       ; preds = %0
  %8 = load i8 addrspace(3)** %3, align 4
  store i8 10, i8 addrspace(3)* %8
  %9 = load i8 addrspace(3)** %4, align 4
  store i8 11, i8 addrspace(3)* %9
  br label %10

; <label>:10                                      ; preds = %7, %0
  call void @barrier(i32 1)
  %11 = load i8 addrspace(3)** %3, align 4
  %12 = load i8 addrspace(1)** %1, align 4
  %13 = getelementptr inbounds i8 addrspace(1)* %12, i32 2
  %14 = call i32 @_Z21async_work_group_copyPU3AS3hPKU3AS1hjj(i8 addrspace(3)* %11, i8 addrspace(1)* %13, i32 1, i32 0)
  store i32 %14, i32* %evs, align 4
  call void @_Z17wait_group_eventsiPj(i32 1, i32* %evs)
  call void @barrier(i32 1)
  call void @barrier(i32 2)
  %15 = load i8 addrspace(3)** %3, align 4
  %16 = load i8 addrspace(3)* %15
  %17 = zext i8 %16 to i32
  store i32 %17, i32* %checker, align 4
  %18 = load i8 addrspace(1)** %2, align 4
  %19 = load i8 addrspace(3)** %4, align 4
  %20 = call i32 @_Z21async_work_group_copyPU3AS1hPKU3AS3hjj(i8 addrspace(1)* %18, i8 addrspace(3)* %19, i32 1, i32 0)
  store i32 %20, i32* %evs, align 4
  call void @_Z17wait_group_eventsiPj(i32 1, i32* %evs)
  call void @barrier(i32 1)
  call void @barrier(i32 2)
  %21 = load i8 addrspace(1)** %2, align 4
  %22 = load i8 addrspace(1)* %21
  %23 = zext i8 %22 to i32
  store i32 %23, i32* %checker, align 4
  %24 = call i32 @get_global_id(i32 0)
  %25 = icmp eq i32 %24, 0
  br i1 %25, label %26, label %32

; <label>:26                                      ; preds = %10
  %27 = load i32* %checker, align 4
  %28 = icmp ne i32 %27, 11
  br i1 %28, label %29, label %31

; <label>:29                                      ; preds = %26
  %30 = call i32 (i8 addrspace(2)*, ...)* @printf(i8 addrspace(2)* getelementptr inbounds ([9 x i8] addrspace(2)* @.str, i32 0, i32 0))
  br label %31

; <label>:31                                      ; preds = %29, %26
  br label %32

; <label>:32                                      ; preds = %31, %10
  %33 = load i32* %checker, align 4
  %34 = add nsw i32 %33, 1
  store i32 %34, i32* %checker, align 4
  ret void
}

declare i32 @get_local_id(i32)

declare void @barrier(i32)

declare i32 @_Z21async_work_group_copyPU3AS3hPKU3AS1hjj(i8 addrspace(3)*, i8 addrspace(1)*, i32, i32)

declare void @_Z17wait_group_eventsiPj(i32, i32*)

declare i32 @_Z21async_work_group_copyPU3AS1hPKU3AS3hjj(i8 addrspace(1)*, i8 addrspace(3)*, i32, i32)

declare i32 @get_global_id(i32)

declare i32 @printf(i8 addrspace(2)*, ...)

define void @Sobel_Kernel_8U(i8 addrspace(1)* %Image, i32 %Image_PitchInChar, i32 %Image_W, i32 %Image_H, i32 %Image_BW, i32 %Image_BH, i8 addrspace(1)* %Out, i32 %Out_PitchInChar, i32 %ROIWidth, i32 %ROIHeight) nounwind {
  %1 = alloca i8 addrspace(1)*, align 4
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  %6 = alloca i32, align 4
  %7 = alloca i8 addrspace(1)*, align 4
  %8 = alloca i32, align 4
  %9 = alloca i32, align 4
  %10 = alloca i32, align 4
  %x = alloca i32, align 4
  %y = alloca i32, align 4
  %x0 = alloca i32, align 4
  %y0 = alloca i32, align 4
  %pSrc0 = alloca i8 addrspace(1)*, align 4
  %pDst = alloca i8 addrspace(1)*, align 4
  %pSrc1 = alloca i8 addrspace(1)*, align 4
  %pSrc2 = alloca i8 addrspace(1)*, align 4
  %v00 = alloca i32, align 4
  %v01 = alloca i32, align 4
  %v02 = alloca i32, align 4
  %v10 = alloca i32, align 4
  %v12 = alloca i32, align 4
  %v20 = alloca i32, align 4
  %v21 = alloca i32, align 4
  %v22 = alloca i32, align 4
  %DX = alloca float, align 4
  %DY = alloca float, align 4
  %res = alloca float, align 4
  store i8 addrspace(1)* %Image, i8 addrspace(1)** %1, align 4
  store i32 %Image_PitchInChar, i32* %2, align 4
  store i32 %Image_W, i32* %3, align 4
  store i32 %Image_H, i32* %4, align 4
  store i32 %Image_BW, i32* %5, align 4
  store i32 %Image_BH, i32* %6, align 4
  store i8 addrspace(1)* %Out, i8 addrspace(1)** %7, align 4
  store i32 %Out_PitchInChar, i32* %8, align 4
  store i32 %ROIWidth, i32* %9, align 4
  store i32 %ROIHeight, i32* %10, align 4
  %11 = call i32 @get_global_id(i32 0)
  %12 = load i32* %9, align 4
  %13 = mul i32 %11, %12
  store i32 %13, i32* %x0, align 4
  %14 = call i32 @get_global_id(i32 1)
  %15 = load i32* %10, align 4
  %16 = mul i32 %14, %15
  store i32 %16, i32* %y0, align 4
  %17 = load i8 addrspace(1)** %1, align 4
  %18 = load i32* %2, align 4
  %19 = load i32* %6, align 4
  %20 = sub nsw i32 %19, 1
  %21 = load i32* %y0, align 4
  %22 = add nsw i32 %20, %21
  %23 = mul nsw i32 %18, %22
  %24 = getelementptr inbounds i8 addrspace(1)* %17, i32 %23
  %25 = load i32* %5, align 4
  %26 = getelementptr inbounds i8 addrspace(1)* %24, i32 %25
  %27 = getelementptr inbounds i8 addrspace(1)* %26, i32 -1
  %28 = load i32* %x0, align 4
  %29 = getelementptr inbounds i8 addrspace(1)* %27, i32 %28
  store i8 addrspace(1)* %29, i8 addrspace(1)** %pSrc0, align 4
  %30 = load i8 addrspace(1)** %7, align 4
  %31 = load i32* %y0, align 4
  %32 = load i32* %8, align 4
  %33 = mul nsw i32 %31, %32
  %34 = getelementptr inbounds i8 addrspace(1)* %30, i32 %33
  %35 = load i32* %x0, align 4
  %36 = getelementptr inbounds i8 addrspace(1)* %34, i32 %35
  store i8 addrspace(1)* %36, i8 addrspace(1)** %pDst, align 4
  store i32 0, i32* %y, align 4
  br label %37

; <label>:37                                      ; preds = %147, %0
  %38 = load i32* %y, align 4
  %39 = load i32* %10, align 4
  %40 = icmp slt i32 %38, %39
  br i1 %40, label %41, label %150

; <label>:41                                      ; preds = %37
  %42 = load i8 addrspace(1)** %pSrc0, align 4
  %43 = load i32* %2, align 4
  %44 = getelementptr inbounds i8 addrspace(1)* %42, i32 %43
  store i8 addrspace(1)* %44, i8 addrspace(1)** %pSrc1, align 4
  %45 = load i8 addrspace(1)** %pSrc0, align 4
  %46 = load i32* %2, align 4
  %47 = mul nsw i32 2, %46
  %48 = getelementptr inbounds i8 addrspace(1)* %45, i32 %47
  store i8 addrspace(1)* %48, i8 addrspace(1)** %pSrc2, align 4
  store i32 0, i32* %x, align 4
  br label %49

; <label>:49                                      ; preds = %137, %41
  %50 = load i32* %x, align 4
  %51 = load i32* %9, align 4
  %52 = icmp slt i32 %50, %51
  br i1 %52, label %53, label %140

; <label>:53                                      ; preds = %49
  %54 = load i8 addrspace(1)** %pSrc0, align 4
  %55 = load i32* %x, align 4
  %56 = getelementptr inbounds i8 addrspace(1)* %54, i32 %55
  %57 = getelementptr inbounds i8 addrspace(1)* %56, i32 0
  %58 = load i8 addrspace(1)* %57
  %59 = call i32 @_Z11convert_inth(i8 zeroext %58)
  store i32 %59, i32* %v00, align 4
  %60 = load i8 addrspace(1)** %pSrc0, align 4
  %61 = load i32* %x, align 4
  %62 = getelementptr inbounds i8 addrspace(1)* %60, i32 %61
  %63 = getelementptr inbounds i8 addrspace(1)* %62, i32 1
  %64 = load i8 addrspace(1)* %63
  %65 = call i32 @_Z11convert_inth(i8 zeroext %64)
  store i32 %65, i32* %v01, align 4
  %66 = load i8 addrspace(1)** %pSrc0, align 4
  %67 = load i32* %x, align 4
  %68 = getelementptr inbounds i8 addrspace(1)* %66, i32 %67
  %69 = getelementptr inbounds i8 addrspace(1)* %68, i32 2
  %70 = load i8 addrspace(1)* %69
  %71 = call i32 @_Z11convert_inth(i8 zeroext %70)
  store i32 %71, i32* %v02, align 4
  %72 = load i8 addrspace(1)** %pSrc1, align 4
  %73 = load i32* %x, align 4
  %74 = getelementptr inbounds i8 addrspace(1)* %72, i32 %73
  %75 = getelementptr inbounds i8 addrspace(1)* %74, i32 0
  %76 = load i8 addrspace(1)* %75
  %77 = call i32 @_Z11convert_inth(i8 zeroext %76)
  store i32 %77, i32* %v10, align 4
  %78 = load i8 addrspace(1)** %pSrc1, align 4
  %79 = load i32* %x, align 4
  %80 = getelementptr inbounds i8 addrspace(1)* %78, i32 %79
  %81 = getelementptr inbounds i8 addrspace(1)* %80, i32 2
  %82 = load i8 addrspace(1)* %81
  %83 = call i32 @_Z11convert_inth(i8 zeroext %82)
  store i32 %83, i32* %v12, align 4
  %84 = load i8 addrspace(1)** %pSrc2, align 4
  %85 = load i32* %x, align 4
  %86 = getelementptr inbounds i8 addrspace(1)* %84, i32 %85
  %87 = getelementptr inbounds i8 addrspace(1)* %86, i32 0
  %88 = load i8 addrspace(1)* %87
  %89 = call i32 @_Z11convert_inth(i8 zeroext %88)
  store i32 %89, i32* %v20, align 4
  %90 = load i8 addrspace(1)** %pSrc2, align 4
  %91 = load i32* %x, align 4
  %92 = getelementptr inbounds i8 addrspace(1)* %90, i32 %91
  %93 = getelementptr inbounds i8 addrspace(1)* %92, i32 1
  %94 = load i8 addrspace(1)* %93
  %95 = call i32 @_Z11convert_inth(i8 zeroext %94)
  store i32 %95, i32* %v21, align 4
  %96 = load i8 addrspace(1)** %pSrc2, align 4
  %97 = load i32* %x, align 4
  %98 = getelementptr inbounds i8 addrspace(1)* %96, i32 %97
  %99 = getelementptr inbounds i8 addrspace(1)* %98, i32 2
  %100 = load i8 addrspace(1)* %99
  %101 = call i32 @_Z11convert_inth(i8 zeroext %100)
  store i32 %101, i32* %v22, align 4
  %102 = load i32* %v02, align 4
  %103 = load i32* %v00, align 4
  %104 = sub nsw i32 %102, %103
  %105 = load i32* %v12, align 4
  %106 = load i32* %v10, align 4
  %107 = sub nsw i32 %105, %106
  %108 = mul nsw i32 2, %107
  %109 = add nsw i32 %104, %108
  %110 = load i32* %v22, align 4
  %111 = load i32* %v20, align 4
  %112 = sub nsw i32 %110, %111
  %113 = add nsw i32 %109, %112
  %114 = call float @_Z13convert_floati(i32 %113)
  store float %114, float* %DX, align 4
  %115 = load i32* %v20, align 4
  %116 = load i32* %v00, align 4
  %117 = sub nsw i32 %115, %116
  %118 = load i32* %v21, align 4
  %119 = load i32* %v01, align 4
  %120 = sub nsw i32 %118, %119
  %121 = mul nsw i32 2, %120
  %122 = add nsw i32 %117, %121
  %123 = load i32* %v22, align 4
  %124 = load i32* %v02, align 4
  %125 = sub nsw i32 %123, %124
  %126 = add nsw i32 %122, %125
  %127 = call float @_Z13convert_floati(i32 %126)
  store float %127, float* %DY, align 4
  %128 = load float* %DX, align 4
  %129 = load float* %DY, align 4
  %130 = call float @_Z5hypotff(float %128, float %129)
  %131 = fmul float %130, 5.000000e-001
  store float %131, float* %res, align 4
  %132 = load float* %res, align 4
  %133 = call zeroext i8 @_Z17convert_uchar_satf(float %132)
  %134 = load i32* %x, align 4
  %135 = load i8 addrspace(1)** %pDst, align 4
  %136 = getelementptr inbounds i8 addrspace(1)* %135, i32 %134
  store i8 %133, i8 addrspace(1)* %136
  br label %137

; <label>:137                                     ; preds = %53
  %138 = load i32* %x, align 4
  %139 = add nsw i32 %138, 1
  store i32 %139, i32* %x, align 4
  br label %49

; <label>:140                                     ; preds = %49
  %141 = load i32* %2, align 4
  %142 = load i8 addrspace(1)** %pSrc0, align 4
  %143 = getelementptr inbounds i8 addrspace(1)* %142, i32 %141
  store i8 addrspace(1)* %143, i8 addrspace(1)** %pSrc0, align 4
  %144 = load i32* %8, align 4
  %145 = load i8 addrspace(1)** %pDst, align 4
  %146 = getelementptr inbounds i8 addrspace(1)* %145, i32 %144
  store i8 addrspace(1)* %146, i8 addrspace(1)** %pDst, align 4
  br label %147

; <label>:147                                     ; preds = %140
  %148 = load i32* %y, align 4
  %149 = add nsw i32 %148, 1
  store i32 %149, i32* %y, align 4
  br label %37

; <label>:150                                     ; preds = %37
  ret void
}

declare i32 @_Z11convert_inth(i8 zeroext)

declare float @_Z13convert_floati(i32)

declare float @_Z5hypotff(float, float)

declare zeroext i8 @_Z17convert_uchar_satf(float)

define void @Sobel_Kernel_Vec_8U(i8 addrspace(1)* %Image, i32 %Image_PitchInChar, i32 %Image_W, i32 %Image_H, i32 %Image_BW, i32 %Image_BH, i8 addrspace(1)* %Out, i32 %Out_PitchInChar, i32 %ROIWidth, i32 %ROIHeight) nounwind {
  %1 = alloca i8 addrspace(1)*, align 4
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  %6 = alloca i32, align 4
  %7 = alloca i8 addrspace(1)*, align 4
  %8 = alloca i32, align 4
  %9 = alloca i32, align 4
  %10 = alloca i32, align 4
  %two = alloca <4 x i32>, align 16
  %11 = alloca <4 x i32>, align 16
  %_half = alloca <4 x float>, align 16
  %12 = alloca <4 x float>, align 16
  %x = alloca i32, align 4
  %y = alloca i32, align 4
  %x0 = alloca i32, align 4
  %y0 = alloca i32, align 4
  %pSrc0 = alloca i8 addrspace(1)*, align 4
  %pDst = alloca i8 addrspace(1)*, align 4
  %pSrc1 = alloca i8 addrspace(1)*, align 4
  %pSrc2 = alloca i8 addrspace(1)*, align 4
  %v00 = alloca <4 x i32>, align 16
  %v01 = alloca <4 x i32>, align 16
  %v02 = alloca <4 x i32>, align 16
  %v10 = alloca <4 x i32>, align 16
  %v12 = alloca <4 x i32>, align 16
  %v20 = alloca <4 x i32>, align 16
  %v21 = alloca <4 x i32>, align 16
  %v22 = alloca <4 x i32>, align 16
  %DX = alloca <4 x float>, align 16
  %DY = alloca <4 x float>, align 16
  %res = alloca <4 x float>, align 16
  %v001 = alloca i32, align 4
  %v012 = alloca i32, align 4
  %v023 = alloca i32, align 4
  %v104 = alloca i32, align 4
  %v125 = alloca i32, align 4
  %v206 = alloca i32, align 4
  %v217 = alloca i32, align 4
  %v228 = alloca i32, align 4
  %DX9 = alloca float, align 4
  %DY10 = alloca float, align 4
  %res11 = alloca float, align 4
  store i8 addrspace(1)* %Image, i8 addrspace(1)** %1, align 4
  store i32 %Image_PitchInChar, i32* %2, align 4
  store i32 %Image_W, i32* %3, align 4
  store i32 %Image_H, i32* %4, align 4
  store i32 %Image_BW, i32* %5, align 4
  store i32 %Image_BH, i32* %6, align 4
  store i8 addrspace(1)* %Out, i8 addrspace(1)** %7, align 4
  store i32 %Out_PitchInChar, i32* %8, align 4
  store i32 %ROIWidth, i32* %9, align 4
  store i32 %ROIHeight, i32* %10, align 4
  store <4 x i32> <i32 2, i32 2, i32 2, i32 2>, <4 x i32>* %11
  %13 = load <4 x i32>* %11
  store <4 x i32> %13, <4 x i32>* %two, align 16
  store <4 x float> <float 5.000000e-001, float 5.000000e-001, float 5.000000e-001, float 5.000000e-001>, <4 x float>* %12
  %14 = load <4 x float>* %12
  store <4 x float> %14, <4 x float>* %_half, align 16
  %15 = call i32 @get_global_id(i32 0)
  %16 = load i32* %9, align 4
  %17 = mul i32 %15, %16
  store i32 %17, i32* %x0, align 4
  %18 = call i32 @get_global_id(i32 1)
  %19 = load i32* %10, align 4
  %20 = mul i32 %18, %19
  store i32 %20, i32* %y0, align 4
  %21 = load i8 addrspace(1)** %1, align 4
  %22 = load i32* %2, align 4
  %23 = load i32* %6, align 4
  %24 = sub nsw i32 %23, 1
  %25 = load i32* %y0, align 4
  %26 = add nsw i32 %24, %25
  %27 = mul nsw i32 %22, %26
  %28 = getelementptr inbounds i8 addrspace(1)* %21, i32 %27
  %29 = load i32* %5, align 4
  %30 = getelementptr inbounds i8 addrspace(1)* %28, i32 %29
  %31 = getelementptr inbounds i8 addrspace(1)* %30, i32 -1
  %32 = load i32* %x0, align 4
  %33 = getelementptr inbounds i8 addrspace(1)* %31, i32 %32
  store i8 addrspace(1)* %33, i8 addrspace(1)** %pSrc0, align 4
  %34 = load i8 addrspace(1)** %7, align 4
  %35 = load i32* %y0, align 4
  %36 = load i32* %8, align 4
  %37 = mul nsw i32 %35, %36
  %38 = getelementptr inbounds i8 addrspace(1)* %34, i32 %37
  %39 = load i32* %x0, align 4
  %40 = getelementptr inbounds i8 addrspace(1)* %38, i32 %39
  store i8 addrspace(1)* %40, i8 addrspace(1)** %pDst, align 4
  store i32 0, i32* %y, align 4
  br label %41

; <label>:41                                      ; preds = %248, %0
  %42 = load i32* %y, align 4
  %43 = load i32* %10, align 4
  %44 = icmp slt i32 %42, %43
  br i1 %44, label %45, label %251

; <label>:45                                      ; preds = %41
  %46 = load i8 addrspace(1)** %pSrc0, align 4
  %47 = load i32* %2, align 4
  %48 = getelementptr inbounds i8 addrspace(1)* %46, i32 %47
  store i8 addrspace(1)* %48, i8 addrspace(1)** %pSrc1, align 4
  %49 = load i8 addrspace(1)** %pSrc0, align 4
  %50 = load i32* %2, align 4
  %51 = mul nsw i32 2, %50
  %52 = getelementptr inbounds i8 addrspace(1)* %49, i32 %51
  store i8 addrspace(1)* %52, i8 addrspace(1)** %pSrc2, align 4
  store i32 0, i32* %x, align 4
  br label %53

; <label>:53                                      ; preds = %146, %45
  %54 = load i32* %x, align 4
  %55 = load i32* %9, align 4
  %56 = sub nsw i32 %55, 3
  %57 = icmp slt i32 %54, %56
  br i1 %57, label %58, label %149

; <label>:58                                      ; preds = %53
  %59 = load i8 addrspace(1)** %pSrc0, align 4
  %60 = load i32* %x, align 4
  %61 = getelementptr inbounds i8 addrspace(1)* %59, i32 %60
  %62 = getelementptr inbounds i8 addrspace(1)* %61, i32 0
  %63 = call <4 x i8> @_Z6vload4jPKU3AS1h(i32 0, i8 addrspace(1)* %62)
  %64 = call <4 x i32> @_Z12convert_int4Dv4_h(<4 x i8> %63)
  store <4 x i32> %64, <4 x i32>* %v00, align 16
  %65 = load i8 addrspace(1)** %pSrc0, align 4
  %66 = load i32* %x, align 4
  %67 = getelementptr inbounds i8 addrspace(1)* %65, i32 %66
  %68 = getelementptr inbounds i8 addrspace(1)* %67, i32 1
  %69 = call <4 x i8> @_Z6vload4jPKU3AS1h(i32 0, i8 addrspace(1)* %68)
  %70 = call <4 x i32> @_Z12convert_int4Dv4_h(<4 x i8> %69)
  store <4 x i32> %70, <4 x i32>* %v01, align 16
  %71 = load i8 addrspace(1)** %pSrc0, align 4
  %72 = load i32* %x, align 4
  %73 = getelementptr inbounds i8 addrspace(1)* %71, i32 %72
  %74 = getelementptr inbounds i8 addrspace(1)* %73, i32 2
  %75 = call <4 x i8> @_Z6vload4jPKU3AS1h(i32 0, i8 addrspace(1)* %74)
  %76 = call <4 x i32> @_Z12convert_int4Dv4_h(<4 x i8> %75)
  store <4 x i32> %76, <4 x i32>* %v02, align 16
  %77 = load i8 addrspace(1)** %pSrc1, align 4
  %78 = load i32* %x, align 4
  %79 = getelementptr inbounds i8 addrspace(1)* %77, i32 %78
  %80 = getelementptr inbounds i8 addrspace(1)* %79, i32 0
  %81 = call <4 x i8> @_Z6vload4jPKU3AS1h(i32 0, i8 addrspace(1)* %80)
  %82 = call <4 x i32> @_Z12convert_int4Dv4_h(<4 x i8> %81)
  store <4 x i32> %82, <4 x i32>* %v10, align 16
  %83 = load i8 addrspace(1)** %pSrc1, align 4
  %84 = load i32* %x, align 4
  %85 = getelementptr inbounds i8 addrspace(1)* %83, i32 %84
  %86 = getelementptr inbounds i8 addrspace(1)* %85, i32 2
  %87 = call <4 x i8> @_Z6vload4jPKU3AS1h(i32 0, i8 addrspace(1)* %86)
  %88 = call <4 x i32> @_Z12convert_int4Dv4_h(<4 x i8> %87)
  store <4 x i32> %88, <4 x i32>* %v12, align 16
  %89 = load i8 addrspace(1)** %pSrc2, align 4
  %90 = load i32* %x, align 4
  %91 = getelementptr inbounds i8 addrspace(1)* %89, i32 %90
  %92 = getelementptr inbounds i8 addrspace(1)* %91, i32 0
  %93 = call <4 x i8> @_Z6vload4jPKU3AS1h(i32 0, i8 addrspace(1)* %92)
  %94 = call <4 x i32> @_Z12convert_int4Dv4_h(<4 x i8> %93)
  store <4 x i32> %94, <4 x i32>* %v20, align 16
  %95 = load i8 addrspace(1)** %pSrc2, align 4
  %96 = load i32* %x, align 4
  %97 = getelementptr inbounds i8 addrspace(1)* %95, i32 %96
  %98 = getelementptr inbounds i8 addrspace(1)* %97, i32 1
  %99 = call <4 x i8> @_Z6vload4jPKU3AS1h(i32 0, i8 addrspace(1)* %98)
  %100 = call <4 x i32> @_Z12convert_int4Dv4_h(<4 x i8> %99)
  store <4 x i32> %100, <4 x i32>* %v21, align 16
  %101 = load i8 addrspace(1)** %pSrc2, align 4
  %102 = load i32* %x, align 4
  %103 = getelementptr inbounds i8 addrspace(1)* %101, i32 %102
  %104 = getelementptr inbounds i8 addrspace(1)* %103, i32 2
  %105 = call <4 x i8> @_Z6vload4jPKU3AS1h(i32 0, i8 addrspace(1)* %104)
  %106 = call <4 x i32> @_Z12convert_int4Dv4_h(<4 x i8> %105)
  store <4 x i32> %106, <4 x i32>* %v22, align 16
  %107 = load <4 x i32>* %v02, align 16
  %108 = load <4 x i32>* %v00, align 16
  %109 = sub nsw <4 x i32> %107, %108
  %110 = load <4 x i32>* %two, align 16
  %111 = load <4 x i32>* %v12, align 16
  %112 = load <4 x i32>* %v10, align 16
  %113 = sub nsw <4 x i32> %111, %112
  %114 = mul nsw <4 x i32> %110, %113
  %115 = add nsw <4 x i32> %109, %114
  %116 = load <4 x i32>* %v22, align 16
  %117 = load <4 x i32>* %v20, align 16
  %118 = sub nsw <4 x i32> %116, %117
  %119 = add nsw <4 x i32> %115, %118
  %120 = call <4 x float> @_Z14convert_float4Dv4_i(<4 x i32> %119)
  store <4 x float> %120, <4 x float>* %DX, align 16
  %121 = load <4 x i32>* %v20, align 16
  %122 = load <4 x i32>* %v00, align 16
  %123 = sub nsw <4 x i32> %121, %122
  %124 = load <4 x i32>* %two, align 16
  %125 = load <4 x i32>* %v21, align 16
  %126 = load <4 x i32>* %v01, align 16
  %127 = sub nsw <4 x i32> %125, %126
  %128 = mul nsw <4 x i32> %124, %127
  %129 = add nsw <4 x i32> %123, %128
  %130 = load <4 x i32>* %v22, align 16
  %131 = load <4 x i32>* %v02, align 16
  %132 = sub nsw <4 x i32> %130, %131
  %133 = add nsw <4 x i32> %129, %132
  %134 = call <4 x float> @_Z14convert_float4Dv4_i(<4 x i32> %133)
  store <4 x float> %134, <4 x float>* %DY, align 16
  %135 = load <4 x float>* %DX, align 16
  %136 = load <4 x float>* %DY, align 16
  %137 = call <4 x float> @_Z5hypotDv4_fS_(<4 x float> %135, <4 x float> %136)
  %138 = load <4 x float>* %_half, align 16
  %139 = fmul <4 x float> %137, %138
  store <4 x float> %139, <4 x float>* %res, align 16
  %140 = load <4 x float>* %res, align 16
  %141 = call <4 x i8> @_Z18convert_uchar4_satDv4_f(<4 x float> %140)
  %142 = load i8 addrspace(1)** %pDst, align 4
  %143 = load i32* %x, align 4
  %144 = getelementptr inbounds i8 addrspace(1)* %142, i32 %143
  %145 = bitcast i8 addrspace(1)* %144 to <4 x i8> addrspace(1)*
  store <4 x i8> %141, <4 x i8> addrspace(1)* %145
  br label %146

; <label>:146                                     ; preds = %58
  %147 = load i32* %x, align 4
  %148 = add nsw i32 %147, 4
  store i32 %148, i32* %x, align 4
  br label %53

; <label>:149                                     ; preds = %53
  br label %150

; <label>:150                                     ; preds = %238, %149
  %151 = load i32* %x, align 4
  %152 = load i32* %9, align 4
  %153 = icmp slt i32 %151, %152
  br i1 %153, label %154, label %241

; <label>:154                                     ; preds = %150
  %155 = load i8 addrspace(1)** %pSrc0, align 4
  %156 = load i32* %x, align 4
  %157 = getelementptr inbounds i8 addrspace(1)* %155, i32 %156
  %158 = getelementptr inbounds i8 addrspace(1)* %157, i32 0
  %159 = load i8 addrspace(1)* %158
  %160 = call i32 @_Z11convert_inth(i8 zeroext %159)
  store i32 %160, i32* %v001, align 4
  %161 = load i8 addrspace(1)** %pSrc0, align 4
  %162 = load i32* %x, align 4
  %163 = getelementptr inbounds i8 addrspace(1)* %161, i32 %162
  %164 = getelementptr inbounds i8 addrspace(1)* %163, i32 1
  %165 = load i8 addrspace(1)* %164
  %166 = call i32 @_Z11convert_inth(i8 zeroext %165)
  store i32 %166, i32* %v012, align 4
  %167 = load i8 addrspace(1)** %pSrc0, align 4
  %168 = load i32* %x, align 4
  %169 = getelementptr inbounds i8 addrspace(1)* %167, i32 %168
  %170 = getelementptr inbounds i8 addrspace(1)* %169, i32 2
  %171 = load i8 addrspace(1)* %170
  %172 = call i32 @_Z11convert_inth(i8 zeroext %171)
  store i32 %172, i32* %v023, align 4
  %173 = load i8 addrspace(1)** %pSrc1, align 4
  %174 = load i32* %x, align 4
  %175 = getelementptr inbounds i8 addrspace(1)* %173, i32 %174
  %176 = getelementptr inbounds i8 addrspace(1)* %175, i32 0
  %177 = load i8 addrspace(1)* %176
  %178 = call i32 @_Z11convert_inth(i8 zeroext %177)
  store i32 %178, i32* %v104, align 4
  %179 = load i8 addrspace(1)** %pSrc1, align 4
  %180 = load i32* %x, align 4
  %181 = getelementptr inbounds i8 addrspace(1)* %179, i32 %180
  %182 = getelementptr inbounds i8 addrspace(1)* %181, i32 2
  %183 = load i8 addrspace(1)* %182
  %184 = call i32 @_Z11convert_inth(i8 zeroext %183)
  store i32 %184, i32* %v125, align 4
  %185 = load i8 addrspace(1)** %pSrc2, align 4
  %186 = load i32* %x, align 4
  %187 = getelementptr inbounds i8 addrspace(1)* %185, i32 %186
  %188 = getelementptr inbounds i8 addrspace(1)* %187, i32 0
  %189 = load i8 addrspace(1)* %188
  %190 = call i32 @_Z11convert_inth(i8 zeroext %189)
  store i32 %190, i32* %v206, align 4
  %191 = load i8 addrspace(1)** %pSrc2, align 4
  %192 = load i32* %x, align 4
  %193 = getelementptr inbounds i8 addrspace(1)* %191, i32 %192
  %194 = getelementptr inbounds i8 addrspace(1)* %193, i32 1
  %195 = load i8 addrspace(1)* %194
  %196 = call i32 @_Z11convert_inth(i8 zeroext %195)
  store i32 %196, i32* %v217, align 4
  %197 = load i8 addrspace(1)** %pSrc2, align 4
  %198 = load i32* %x, align 4
  %199 = getelementptr inbounds i8 addrspace(1)* %197, i32 %198
  %200 = getelementptr inbounds i8 addrspace(1)* %199, i32 2
  %201 = load i8 addrspace(1)* %200
  %202 = call i32 @_Z11convert_inth(i8 zeroext %201)
  store i32 %202, i32* %v228, align 4
  %203 = load i32* %v023, align 4
  %204 = load i32* %v001, align 4
  %205 = sub nsw i32 %203, %204
  %206 = load i32* %v125, align 4
  %207 = load i32* %v104, align 4
  %208 = sub nsw i32 %206, %207
  %209 = mul nsw i32 2, %208
  %210 = add nsw i32 %205, %209
  %211 = load i32* %v228, align 4
  %212 = load i32* %v206, align 4
  %213 = sub nsw i32 %211, %212
  %214 = add nsw i32 %210, %213
  %215 = call float @_Z13convert_floati(i32 %214)
  store float %215, float* %DX9, align 4
  %216 = load i32* %v206, align 4
  %217 = load i32* %v001, align 4
  %218 = sub nsw i32 %216, %217
  %219 = load i32* %v217, align 4
  %220 = load i32* %v012, align 4
  %221 = sub nsw i32 %219, %220
  %222 = mul nsw i32 2, %221
  %223 = add nsw i32 %218, %222
  %224 = load i32* %v228, align 4
  %225 = load i32* %v023, align 4
  %226 = sub nsw i32 %224, %225
  %227 = add nsw i32 %223, %226
  %228 = call float @_Z13convert_floati(i32 %227)
  store float %228, float* %DY10, align 4
  %229 = load float* %DX9, align 4
  %230 = load float* %DY10, align 4
  %231 = call float @_Z5hypotff(float %229, float %230)
  %232 = fmul float %231, 5.000000e-001
  store float %232, float* %res11, align 4
  %233 = load float* %res11, align 4
  %234 = call zeroext i8 @_Z17convert_uchar_satf(float %233)
  %235 = load i32* %x, align 4
  %236 = load i8 addrspace(1)** %pDst, align 4
  %237 = getelementptr inbounds i8 addrspace(1)* %236, i32 %235
  store i8 %234, i8 addrspace(1)* %237
  br label %238

; <label>:238                                     ; preds = %154
  %239 = load i32* %x, align 4
  %240 = add nsw i32 %239, 1
  store i32 %240, i32* %x, align 4
  br label %150

; <label>:241                                     ; preds = %150
  %242 = load i32* %2, align 4
  %243 = load i8 addrspace(1)** %pSrc0, align 4
  %244 = getelementptr inbounds i8 addrspace(1)* %243, i32 %242
  store i8 addrspace(1)* %244, i8 addrspace(1)** %pSrc0, align 4
  %245 = load i32* %8, align 4
  %246 = load i8 addrspace(1)** %pDst, align 4
  %247 = getelementptr inbounds i8 addrspace(1)* %246, i32 %245
  store i8 addrspace(1)* %247, i8 addrspace(1)** %pDst, align 4
  br label %248

; <label>:248                                     ; preds = %241
  %249 = load i32* %y, align 4
  %250 = add nsw i32 %249, 1
  store i32 %250, i32* %y, align 4
  br label %41

; <label>:251                                     ; preds = %41
  ret void
}

declare <4 x i32> @_Z12convert_int4Dv4_h(<4 x i8>)

declare <4 x i8> @_Z6vload4jPKU3AS1h(i32, i8 addrspace(1)*)

declare <4 x float> @_Z14convert_float4Dv4_i(<4 x i32>)

declare <4 x float> @_Z5hypotDv4_fS_(<4 x float>, <4 x float>)

declare <4 x i8> @_Z18convert_uchar4_satDv4_f(<4 x float>)

!opencl.kernels = !{!0, !6, !11}
!opencl.build.options = !{}

!0 = metadata !{void (i8 addrspace(1)*, i8 addrspace(1)*, i8 addrspace(3)*, i8 addrspace(3)*)* @async_copy_test, metadata !1, metadata !1, metadata !"", metadata !"uchar __attribute__((address_space(1))) *, uchar __attribute__((address_space(1))) *, uchar __attribute__((address_space(3))) *, uchar __attribute__((address_space(3))) *", metadata !"opencl_async_copy_test_locals_anchor", metadata !2, metadata !3, metadata !4, metadata !5, metadata !""}
!1 = metadata !{i32 0, i32 0, i32 0}
!2 = metadata !{i32 1, i32 1, i32 3, i32 3}
!3 = metadata !{i32 3, i32 3, i32 3, i32 3}
!4 = metadata !{metadata !"uchar*", metadata !"uchar*", metadata !"uchar*", metadata !"uchar*"}
!5 = metadata !{metadata !"buf_in1", metadata !"buf_out1", metadata !"local1", metadata !"local2"}
!6 = metadata !{void (i8 addrspace(1)*, i32, i32, i32, i32, i32, i8 addrspace(1)*, i32, i32, i32)* @Sobel_Kernel_8U, metadata !1, metadata !1, metadata !"", metadata !"uchar __attribute__((address_space(1))) *, int, int, int, int, int, uchar __attribute__((address_space(1))) *, int, int, int", metadata !"opencl_Sobel_Kernel_8U_locals_anchor", metadata !7, metadata !8, metadata !9, metadata !10, metadata !""}
!7 = metadata !{i32 1, i32 0, i32 0, i32 0, i32 0, i32 0, i32 1, i32 0, i32 0, i32 0}
!8 = metadata !{i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3}
!9 = metadata !{metadata !"uchar*", metadata !"int", metadata !"int", metadata !"int", metadata !"int", metadata !"int", metadata !"uchar*", metadata !"int", metadata !"int", metadata !"int"}
!10 = metadata !{metadata !"Image", metadata !"Image_PitchInChar", metadata !"Image_W", metadata !"Image_H", metadata !"Image_BW", metadata !"Image_BH", metadata !"Out", metadata !"Out_PitchInChar", metadata !"ROIWidth", metadata !"ROIHeight"}
!11 = metadata !{void (i8 addrspace(1)*, i32, i32, i32, i32, i32, i8 addrspace(1)*, i32, i32, i32)* @Sobel_Kernel_Vec_8U, metadata !1, metadata !1, metadata !"", metadata !"uchar __attribute__((address_space(1))) *, int, int, int, int, int, uchar __attribute__((address_space(1))) *, int, int, int", metadata !"opencl_Sobel_Kernel_Vec_8U_locals_anchor", metadata !7, metadata !8, metadata !9, metadata !10, metadata !""}
