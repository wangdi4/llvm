; RUN: opt -passes=sycl-kernel-target-ext-type-lower -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-target-ext-type-lower -S %s | FileCheck %s

; This SPIR-V friendly IR is from non-spirv test.
; Check that target extension type is replaced with layout type in image builtin.

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64_x86_64-unknown-unknown"

$_ZTSZZN12sampler_copy23check_image_sampler_useERN4sycl3_V17samplerEENKUlRNS1_7handlerEE_clES5_E17image_kernel_read = comdat any

@__spirv_BuiltInGlobalInvocationId = external dso_local local_unnamed_addr addrspace(1) constant <3 x i64>, align 32

; Function Attrs: convergent norecurse nounwind
define weak_odr dso_local spir_kernel void @_ZTSZZN12sampler_copy23check_image_sampler_useERN4sycl3_V17samplerEENKUlRNS1_7handlerEE_clES5_E17image_kernel_read(target("spirv.Image", void, 2, 0, 0, 0, 0, 0, 0) %_arg_img_acc, target("spirv.Sampler") %_arg_sampler) local_unnamed_addr #0 comdat !srcloc !1 !kernel_arg_buffer_location !2 !sycl_fixed_targets !3 !sycl_kernel_omit_args !4 {
entry:
; CHECK-LABEL: define weak_odr dso_local spir_kernel void @_ZTSZZN12sampler_copy23check_image_sampler_useERN4sycl3_V17samplerEENKUlRNS1_7handlerEE_clES5_E17image_kernel_read(
; CHECK-SAME: ptr addrspace(1) %_arg_img_acc, ptr addrspace(2) %_arg_sampler
; CHECK-SAME: !arg_type_null_val [[MD:![0-9]+]]

  %0 = load i64, ptr addrspace(1) getelementptr inbounds (i8, ptr addrspace(1) @__spirv_BuiltInGlobalInvocationId, i64 16), align 16, !noalias !5
  %1 = load i64, ptr addrspace(1) getelementptr inbounds (i8, ptr addrspace(1) @__spirv_BuiltInGlobalInvocationId, i64 8), align 8, !noalias !5
  %2 = load i64, ptr addrspace(1) @__spirv_BuiltInGlobalInvocationId, align 32, !noalias !5
  %conv.i = uitofp i64 %0 to float
  %conv3.i = uitofp i64 %1 to float
  %conv5.i = uitofp i64 %2 to float
  %3 = insertelement <4 x float> <float poison, float poison, float poison, float 0.000000e+00>, float %conv.i, i64 0
  %4 = insertelement <4 x float> %3, float %conv3.i, i64 1
  %vecinit7.i.i = insertelement <4 x float> %4, float %conv5.i, i64 2
; CHECK: tail call spir_func ptr addrspace(1) @_Z20__spirv_SampledImageI14ocl_image3d_ro32__spirv_SampledImage__image3d_roET0_T_11ocl_sampler(ptr addrspace(1) %_arg_img_acc, ptr addrspace(2) %_arg_sampler)
; CHECK: tail call fast spir_func noundef nofpclass(nan inf) <4 x float> @_Z30__spirv_ImageSampleExplicitLodI32__spirv_SampledImage__image3d_roDv4_fS1_ET0_T_T1_if(ptr addrspace(1) %call1.i.i.i,
  %call1.i.i.i = tail call spir_func target("spirv.SampledImage", void, 2, 0, 0, 0, 0, 0, 0) @_Z20__spirv_SampledImageI14ocl_image3d_ro32__spirv_SampledImage__image3d_roET0_T_11ocl_sampler(target("spirv.Image", void, 2, 0, 0, 0, 0, 0, 0) %_arg_img_acc, target("spirv.Sampler") %_arg_sampler) #1, !noalias !14
  %call2.i.i.i = tail call fast spir_func noundef nofpclass(nan inf) <4 x float> @_Z30__spirv_ImageSampleExplicitLodI32__spirv_SampledImage__image3d_roDv4_fS1_ET0_T_T1_if(target("spirv.SampledImage", void, 2, 0, 0, 0, 0, 0, 0) %call1.i.i.i, <4 x float> noundef nofpclass(nan inf) %vecinit7.i.i, i32 noundef 2, float noundef nofpclass(nan inf) 0.000000e+00) #1, !noalias !14
  ret void
}

; CHECK-DAG: declare dso_local spir_func ptr addrspace(1) @_Z20__spirv_SampledImageI14ocl_image3d_ro32__spirv_SampledImage__image3d_roET0_T_11ocl_sampler(ptr addrspace(1), ptr addrspace(2))

; CHECK-DAG: declare dso_local spir_func noundef nofpclass(nan inf) <4 x float> @_Z30__spirv_ImageSampleExplicitLodI32__spirv_SampledImage__image3d_roDv4_fS1_ET0_T_T1_if(ptr addrspace(1), <4 x float> noundef nofpclass(nan inf), i32 noundef, float noundef nofpclass(nan inf))

declare dso_local spir_func target("spirv.SampledImage", void, 2, 0, 0, 0, 0, 0, 0) @_Z20__spirv_SampledImageI14ocl_image3d_ro32__spirv_SampledImage__image3d_roET0_T_11ocl_sampler(target("spirv.Image", void, 2, 0, 0, 0, 0, 0, 0), target("spirv.Sampler")) local_unnamed_addr #1

declare dso_local spir_func noundef nofpclass(nan inf) <4 x float> @_Z30__spirv_ImageSampleExplicitLodI32__spirv_SampledImage__image3d_roDv4_fS1_ET0_T_T1_if(target("spirv.SampledImage", void, 2, 0, 0, 0, 0, 0, 0), <4 x float> noundef nofpclass(nan inf), i32 noundef, float noundef nofpclass(nan inf)) local_unnamed_addr #1

attributes #0 = { convergent norecurse nounwind }
attributes #1 = { convergent nounwind }

; CHECK: [[MD]] = !{target("spirv.Image", void, 2, 0, 0, 0, 0, 0, 0) zeroinitializer, target("spirv.Sampler") zeroinitializer}

!spirv.Source = !{!0}

!0 = !{i32 4, i32 100000}
!1 = !{i32 2139}
!2 = !{i32 -1, i32 -1}
!3 = !{}
!4 = !{i1 false, i1 false}
!5 = !{!6, !8, !10, !12}
!6 = distinct !{!6, !7, !"_ZN7__spirv29InitSizesSTGlobalInvocationIdILi3EN4sycl3_V12idILi3EEEE8initSizeEv: %agg.result"}
!7 = distinct !{!7, !"_ZN7__spirv29InitSizesSTGlobalInvocationIdILi3EN4sycl3_V12idILi3EEEE8initSizeEv"}
!8 = distinct !{!8, !9, !"_ZN7__spirvL22initGlobalInvocationIdILi3EN4sycl3_V12idILi3EEEEET0_v: %agg.result"}
!9 = distinct !{!9, !"_ZN7__spirvL22initGlobalInvocationIdILi3EN4sycl3_V12idILi3EEEEET0_v"}
!10 = distinct !{!10, !11, !"_ZN4sycl3_V16detail7Builder7getItemILi3ELb1EEENSt9enable_ifIXT0_EKNS0_4itemIXT_EXT0_EEEE4typeEv: %agg.result"}
!11 = distinct !{!11, !"_ZN4sycl3_V16detail7Builder7getItemILi3ELb1EEENSt9enable_ifIXT0_EKNS0_4itemIXT_EXT0_EEEE4typeEv"}
!12 = distinct !{!12, !13, !"_ZN4sycl3_V16detail7Builder10getElementILi3ELb1EEEDTcl7getItemIXT_EXT0_EEEEPNS0_4itemIXT_EXT0_EEE: %agg.result"}
!13 = distinct !{!13, !"_ZN4sycl3_V16detail7Builder10getElementILi3ELb1EEEDTcl7getItemIXT_EXT0_EEEEPNS0_4itemIXT_EXT0_EEE"}
!14 = !{!15, !17}
!15 = distinct !{!15, !16, !"_ZL26__invoke__ImageReadSamplerIN4sycl3_V13vecIfLi4EEE14ocl_image3d_roS3_ET_T0_T1_RK11ocl_sampler: %agg.result"}
!16 = distinct !{!16, !"_ZL26__invoke__ImageReadSamplerIN4sycl3_V13vecIfLi4EEE14ocl_image3d_roS3_ET_T0_T1_RK11ocl_sampler"}
!17 = distinct !{!17, !18, !"_ZNK4sycl3_V16detail14image_accessorINS0_3vecIfLi4EEELi3ELNS0_6access4modeE1024ELNS5_6targetE2017ELNS5_11placeholderE0EE4readIS4_Li3EvEES4_RKT_RKNS0_7samplerE: %agg.result"}
!18 = distinct !{!18, !"_ZNK4sycl3_V16detail14image_accessorINS0_3vecIfLi4EEELi3ELNS0_6access4modeE1024ELNS5_6targetE2017ELNS5_11placeholderE0EE4readIS4_Li3EvEES4_RKT_RKNS0_7samplerE"}

; DEBUGIFY-NOT: WARNING
