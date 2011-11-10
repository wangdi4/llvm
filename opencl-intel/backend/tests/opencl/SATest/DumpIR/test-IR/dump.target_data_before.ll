; ModuleID = 'Program'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

define <4 x float> @evaluatePixel(<2 x float> %outCrd, <2 x float> %checkerSize, <4 x float> %color1, <4 x float> %color2) nounwind {
; <label>:0
  %1 = fdiv <2 x float> %outCrd, %checkerSize
  %2 = call <2 x float> @_Z5floorDv2_f(<2 x float> %1) nounwind
  %3 = call <2 x float> @_Z4fmodDv2_fS_(<2 x float> %2, <2 x float> <float 2.000000e+000, float 2.000000e+000>) nounwind
  %4 = call <2 x i32> @_Z7isequalDv2_fS_(<2 x float> %3, <2 x float> zeroinitializer) nounwind
  %5 = call i32 @_Z3allDv2_i(<2 x i32> %4) nounwind
  %6 = icmp eq i32 %5, 0
  br i1 %6, label %7, label %11

; <label>:7                                       ; preds = %0
  %8 = call <2 x i32> @_Z7isequalDv2_fS_(<2 x float> %3, <2 x float> <float 1.000000e+000, float 1.000000e+000>) nounwind
  %9 = call i32 @_Z3allDv2_i(<2 x i32> %8) nounwind
  %10 = icmp ne i32 %9, 0
  %phitmp = select i1 %10, <4 x float> %color1, <4 x float> %color2
  br label %11

; <label>:11                                      ; preds = %0, %7
  %12 = phi <4 x float> [ %color1, %0 ], [ %phitmp, %7 ]
  ret <4 x float> %12
}

declare <2 x float> @_Z5floorDv2_f(<2 x float>)

declare <2 x float> @_Z4fmodDv2_fS_(<2 x float>, <2 x float>)

declare i32 @_Z3allDv2_i(<2 x i32>)

declare <2 x i32> @_Z7isequalDv2_fS_(<2 x float>, <2 x float>)

define void @checkerboard2D(<4 x float> addrspace(1)* %output, <2 x float> %checkerSize, <4 x float> %color1, <4 x float> %color2) nounwind {
  %1 = call i32 @get_global_id(i32 0) nounwind
  %2 = call i32 @get_global_id(i32 1) nounwind
  %3 = call i32 @get_global_size(i32 0) nounwind
  %4 = mul nsw i32 %2, %3
  %5 = add nsw i32 %4, %1
  %6 = sitofp i32 %1 to float
  %7 = insertelement <2 x float> undef, float %6, i32 0
  %8 = sitofp i32 %2 to float
  %9 = insertelement <2 x float> %7, float %8, i32 1
  %10 = call <4 x float> @evaluatePixel(<2 x float> %9, <2 x float> %checkerSize, <4 x float> %color1, <4 x float> %color2)
  %11 = getelementptr inbounds <4 x float> addrspace(1)* %output, i32 %5
  store <4 x float> %10, <4 x float> addrspace(1)* %11
  ret void
}

declare i32 @get_global_id(i32)

declare i32 @get_global_size(i32)

!opencl.kernels = !{!0}

!0 = metadata !{void (<4 x float> addrspace(1)*, <2 x float>, <4 x float>, <4 x float>)* @checkerboard2D, metadata !1, metadata !1, metadata !"", metadata !"float4 __attribute__((address_space(1))) *, float2 const, float4 const, float4 const", metadata !"opencl_checkerboard2D_locals_anchor"}
!1 = metadata !{i32 0, i32 0, i32 0}
