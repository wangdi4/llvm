; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define <4 x float> @evaluatePixel(<2 x float> %outCrd, <2 x float> %checkerSize, <4 x float> %color1, <4 x float> %color2) nounwind {
entry:
  %div = fdiv <2 x float> %outCrd, %checkerSize
  %call = call <2 x float> @_Z5floorDv2_f(<2 x float> %div) nounwind readnone
  %call1 = call <2 x float> @_Z4fmodDv2_fS_(<2 x float> %call, <2 x float> <float 2.000000e+00, float 2.000000e+00>) nounwind readnone
  %call2 = call <2 x i32> @_Z7isequalDv2_fS_(<2 x float> %call1, <2 x float> zeroinitializer) nounwind readnone
  %call3 = call i32 @_Z3allDv2_i(<2 x i32> %call2) nounwind readnone
  %tobool = icmp eq i32 %call3, 0
  br i1 %tobool, label %lor.rhs, label %lor.end

lor.rhs:                                          ; preds = %entry
  %call4 = call <2 x i32> @_Z7isequalDv2_fS_(<2 x float> %call1, <2 x float> <float 1.000000e+00, float 1.000000e+00>) nounwind readnone
  %call5 = call i32 @_Z3allDv2_i(<2 x i32> %call4) nounwind readnone
  %tobool6 = icmp ne i32 %call5, 0
  %phitmp = select i1 %tobool6, <4 x float> %color1, <4 x float> %color2
  br label %lor.end

lor.end:                                          ; preds = %entry, %lor.rhs
  %0 = phi <4 x float> [ %color1, %entry ], [ %phitmp, %lor.rhs ]
  ret <4 x float> %0
}

declare <2 x float> @_Z5floorDv2_f(<2 x float>) nounwind readnone

declare <2 x float> @_Z4fmodDv2_fS_(<2 x float>, <2 x float>) nounwind readnone

declare i32 @_Z3allDv2_i(<2 x i32>) nounwind readnone

declare <2 x i32> @_Z7isequalDv2_fS_(<2 x float>, <2 x float>) nounwind readnone

define void @checkerboard2D(<4 x float> addrspace(1)* %output, <2 x float> %checkerSize, <4 x float> %color1, <4 x float> %color2) nounwind {
entry:
  %call = call i64 @get_global_id(i32 0) nounwind readnone
  %conv = trunc i64 %call to i32
  %call1 = call i64 @get_global_id(i32 1) nounwind readnone
  %conv2 = trunc i64 %call1 to i32
  %call3 = call i64 @get_global_size(i32 0) nounwind readnone
  %conv4 = trunc i64 %call3 to i32
  %mul = mul nsw i32 %conv2, %conv4
  %add = add nsw i32 %mul, %conv
  %conv5 = sitofp i32 %conv to float
  %vecinit = insertelement <2 x float> undef, float %conv5, i32 0
  %conv6 = sitofp i32 %conv2 to float
  %vecinit7 = insertelement <2 x float> %vecinit, float %conv6, i32 1
  %call8 = call <4 x float> @evaluatePixel(<2 x float> %vecinit7, <2 x float> %checkerSize, <4 x float> %color1, <4 x float> %color2)
  %idxprom = sext i32 %add to i64
  %arrayidx = getelementptr inbounds <4 x float> addrspace(1)* %output, i64 %idxprom
  store <4 x float> %call8, <4 x float> addrspace(1)* %arrayidx, align 16
  ret void
}

declare i64 @get_global_id(i32) nounwind readnone

declare i64 @get_global_size(i32) nounwind readnone

!opencl.kernels = !{!0}
!opencl.build.options = !{!8}

!0 = metadata !{void (<4 x float> addrspace(1)*, <2 x float>, <4 x float>, <4 x float>)* @checkerboard2D, metadata !1, metadata !2}
!1 = metadata !{metadata !"image_access_qualifier", i32 3, i32 3, i32 3, i32 3}
!2 = metadata !{metadata !"cl_kernel_arg_info", metadata !3, metadata !4, metadata !5, metadata !6, metadata !7}
!3 = metadata !{i32 0, i32 3, i32 3, i32 3}
!4 = metadata !{i32 3, i32 3, i32 3, i32 3}
!5 = metadata !{metadata !"float4 *", metadata !"float2", metadata !"float4", metadata !"float4"}
!6 = metadata !{i32 0, i32 1, i32 1, i32 1}
!7 = metadata !{metadata !"output", metadata !"checkerSize", metadata !"color1", metadata !"color2"}
!8 = metadata !{metadata !"-cl-kernel-arg-info"}
