; RUN: opt -passes=sycl-kernel-target-ext-type-lower -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-target-ext-type-lower -S %s | FileCheck %s

; Check that function argument byval type, that contains target extension type,
; is correctly replaced.

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64_x86_64-unknown-unknown"

; CHECK: [[SYCLV1Accessor:%"class.sycl::_V1::accessor.*"]] = type { [[SYCLV1DetailImageAccessor:%"class.sycl::_V1::detail::image_accessor.*"]] }
; CHECK: [[SYCLV1DetailImageAccessor]] = type { ptr addrspace(1), [24 x i8] }
; CHECK: [[SYCLV1Sampler:%"class.sycl::_V1::sampler.*"]] = type { [[SYCLV1DetailSampler:%"class.sycl::_V1::detail::sampler_impl.*"]], [8 x i8] }
; CHECK: [[SYCLV1DetailSampler]] = type { ptr addrspace(2) }

%"class.sycl::_V1::accessor" = type { %"class.sycl::_V1::detail::image_accessor" }
%"class.sycl::_V1::detail::image_accessor" = type { target("spirv.Image", void, 1, 0, 0, 0, 0, 0, 0), [24 x i8] }
%"class.sycl::_V1::sampler" = type { %"class.sycl::_V1::detail::sampler_impl", [8 x i8] }
%"class.sycl::_V1::detail::sampler_impl" = type { target("spirv.Sampler") }
%"class.sycl::_V1::nd_item" = type { %"class.sycl::_V1::item", %"class.sycl::_V1::item.0", %"class.sycl::_V1::group" }
%"class.sycl::_V1::item" = type { %"struct.sycl::_V1::detail::ItemBase" }
%"struct.sycl::_V1::detail::ItemBase" = type { %"class.sycl::_V1::range", %"class.sycl::_V1::range", %"class.sycl::_V1::range" }
%"class.sycl::_V1::range" = type { %"class.sycl::_V1::detail::array" }
%"class.sycl::_V1::detail::array" = type { [3 x i64] }
%"class.sycl::_V1::item.0" = type { %"struct.sycl::_V1::detail::ItemBase.1" }
%"struct.sycl::_V1::detail::ItemBase.1" = type { %"class.sycl::_V1::range", %"class.sycl::_V1::range" }
%"class.sycl::_V1::group" = type { %"class.sycl::_V1::range", %"class.sycl::_V1::range", %"class.sycl::_V1::range", %"class.sycl::_V1::range" }

define weak_odr dso_local spir_kernel void @_ZTSZZL18ComputeDerivativesPKfS0_iiiPfS1_S1_N4sycl3_V15queueEENKUlRNS3_7handlerEE_clES6_EUlNS3_7nd_itemILi3EEEE_(i32 noundef %_arg_w, i32 noundef %_arg_h, i32 noundef %_arg_s, ptr addrspace(1) noundef align 4 %_arg_Ix, ptr addrspace(1) noundef align 4 %_arg_Iy, ptr addrspace(1) noundef align 4 %_arg_Iz, target("spirv.Image", void, 1, 0, 0, 0, 0, 0, 0) %_arg_texSource_acc, target("spirv.Image", void, 1, 0, 0, 0, 0, 0, 0) %_arg_texTarget_acc, target("spirv.Sampler") %_arg_texDescr) {
entry:
; CHECK: define weak_odr dso_local spir_kernel void @_ZTSZZL18ComputeDerivativesPKfS0_iiiPfS1_S1_N4sycl3_V15queueEENKUlRNS3_7handlerEE_clES6_EUlNS3_7nd_itemILi3EEEE_(i32 noundef %_arg_w, i32 noundef %_arg_h, i32 noundef %_arg_s, ptr addrspace(1) noundef align 4 %_arg_Ix, ptr addrspace(1) noundef align 4 %_arg_Iy, ptr addrspace(1) noundef align 4 %_arg_Iz, ptr addrspace(1) %_arg_texSource_acc, ptr addrspace(1) %_arg_texTarget_acc, ptr addrspace(2) %_arg_texDescr)
; CHECK-SAME: !arg_type_null_val
; CHECK: %agg.tmp.i = alloca [[SYCLV1Accessor]], align 8
; CHECK: %agg.tmp2.i = alloca [[SYCLV1Accessor]], align 8
; CHECK: %agg.tmp3.i = alloca [[SYCLV1Sampler]], align 8
; CHECK: %__SYCLKernel.sroa.209 = alloca ptr addrspace(2), align 8
; CHECK: store ptr addrspace(1) %_arg_texSource_acc, ptr %agg.tmp.i, align 8
; CHECK: store ptr addrspace(1) %_arg_texTarget_acc, ptr %agg.tmp2.i, align 8
; CHECK: tail call spir_func void @_Z24ComputeDerivativesKerneliiiPfS_S_N4sycl3_V18accessorINS1_3vecIfLi4EEELi2ELNS1_6access4modeE1024ELNS5_6targetE2017ELNS5_11placeholderE0ENS1_3ext6oneapi22accessor_property_listIJEEEEESD_NS1_7samplerENS1_7nd_itemILi3EEE(i32 noundef %_arg_w, i32 noundef %_arg_h, i32 noundef %_arg_s, ptr addrspace(4) noundef %0, ptr addrspace(4) noundef %1, ptr addrspace(4) noundef %2, ptr noundef nonnull byval([[SYCLV1Accessor]]) align 8 %agg.tmp.i, ptr noundef nonnull byval([[SYCLV1Accessor]]) align 8 %agg.tmp2.i, ptr noundef nonnull byval([[SYCLV1Sampler]]) align 8 %agg.tmp3.i, ptr noundef nonnull byval(%"class.sycl::_V1::nd_item") align 8 %agg.tmp3)

  %agg.tmp.i = alloca %"class.sycl::_V1::accessor", align 8
  %agg.tmp2.i = alloca %"class.sycl::_V1::accessor", align 8
  %agg.tmp3.i = alloca %"class.sycl::_V1::sampler", align 8
  %agg.tmp3 = alloca %"class.sycl::_V1::nd_item", i32 0, align 8
  %__SYCLKernel.sroa.209 = alloca target("spirv.Sampler"), align 8
  %0 = addrspacecast ptr addrspace(1) null to ptr addrspace(4)
  %1 = addrspacecast ptr addrspace(1) null to ptr addrspace(4)
  %2 = addrspacecast ptr addrspace(1) null to ptr addrspace(4)
  store target("spirv.Image", void, 1, 0, 0, 0, 0, 0, 0) %_arg_texSource_acc, ptr %agg.tmp.i, align 8
  store target("spirv.Image", void, 1, 0, 0, 0, 0, 0, 0) %_arg_texTarget_acc, ptr %agg.tmp2.i, align 8
  tail call spir_func void @_Z24ComputeDerivativesKerneliiiPfS_S_N4sycl3_V18accessorINS1_3vecIfLi4EEELi2ELNS1_6access4modeE1024ELNS5_6targetE2017ELNS5_11placeholderE0ENS1_3ext6oneapi22accessor_property_listIJEEEEESD_NS1_7samplerENS1_7nd_itemILi3EEE(i32 noundef %_arg_w, i32 noundef %_arg_h, i32 noundef %_arg_s, ptr addrspace(4) noundef %0, ptr addrspace(4) noundef %1, ptr addrspace(4) noundef %2, ptr noundef nonnull byval(%"class.sycl::_V1::accessor") align 8 %agg.tmp.i, ptr noundef nonnull byval(%"class.sycl::_V1::accessor") align 8 %agg.tmp2.i, ptr noundef nonnull byval(%"class.sycl::_V1::sampler") align 8 %agg.tmp3.i, ptr noundef nonnull byval(%"class.sycl::_V1::nd_item") align 8 %agg.tmp3)
  ret void
}

define dso_local spir_func void @_Z24ComputeDerivativesKerneliiiPfS_S_N4sycl3_V18accessorINS1_3vecIfLi4EEELi2ELNS1_6access4modeE1024ELNS5_6targetE2017ELNS5_11placeholderE0ENS1_3ext6oneapi22accessor_property_listIJEEEEESD_NS1_7samplerENS1_7nd_itemILi3EEE(i32 noundef %width, i32 noundef %height, i32 noundef %stride, ptr addrspace(4) nocapture noundef writeonly %Ix, ptr addrspace(4) nocapture noundef writeonly %Iy, ptr addrspace(4) nocapture noundef writeonly %Iz, ptr nocapture noundef readonly byval(%"class.sycl::_V1::accessor") align 8 %texSource, ptr nocapture noundef readonly byval(%"class.sycl::_V1::accessor") align 8 %texTarget, ptr nocapture noundef readonly byval(%"class.sycl::_V1::sampler") align 8 %texDesc, ptr nocapture noundef readonly byval(%"class.sycl::_V1::nd_item") align 8 %item_ct1) {
entry:
; CHECK: define dso_local spir_func void @_Z24ComputeDerivativesKerneliiiPfS_S_N4sycl3_V18accessorINS1_3vecIfLi4EEELi2ELNS1_6access4modeE1024ELNS5_6targetE2017ELNS5_11placeholderE0ENS1_3ext6oneapi22accessor_property_listIJEEEEESD_NS1_7samplerENS1_7nd_itemILi3EEE(i32 noundef %width, i32 noundef %height, i32 noundef %stride, ptr addrspace(4) nocapture noundef writeonly %Ix, ptr addrspace(4) nocapture noundef writeonly %Iy, ptr addrspace(4) nocapture noundef writeonly %Iz, ptr nocapture noundef readonly byval([[SYCLV1Accessor]]) align 8 %texSource, ptr nocapture noundef readonly byval([[SYCLV1Accessor]]) align 8 %texTarget, ptr nocapture noundef readonly byval([[SYCLV1Sampler]]) align 8 %texDesc, ptr nocapture noundef readonly byval(%"class.sycl::_V1::nd_item") align 8 %item_ct1)
; CHECK-SAME: !arg_type_null_val
; CHECK: %call1.i.i = tail call spir_func ptr addrspace(1) @_Z20__spirv_SampledImageI14ocl_image2d_ro32__spirv_SampledImage__image2d_roET0_T_11ocl_sampler(ptr addrspace(1) %4, ptr addrspace(2) %Smpl.val.i)
; CHECK: %call2.i.i = tail call fast spir_func noundef nofpclass(nan inf) <4 x float> @_Z30__spirv_ImageSampleExplicitLodI32__spirv_SampledImage__image2d_roDv4_fDv2_fET0_T_T1_if(ptr addrspace(1) %call1.i.i, <2 x float> noundef nofpclass(nan inf) %vecinit3.i.i, i32 noundef 2, float noundef nofpclass(nan inf) 0.000000e+00)
; CHECK: %call1.i.i150 = tail call spir_func ptr addrspace(1) @_Z20__spirv_SampledImageI14ocl_image2d_ro32__spirv_SampledImage__image2d_roET0_T_11ocl_sampler(ptr addrspace(1) %4, ptr addrspace(2) %Smpl.val.i)

  %0 = load i64, ptr null, align 8
  %1 = load i64, ptr null, align 8
  %2 = load i64, ptr null, align 8
  %3 = load i64, ptr null, align 8
  %vecinit3.i.i = insertelement <2 x float> zeroinitializer, float 0.000000e+00, i64 0
  %4 = load target("spirv.Image", void, 1, 0, 0, 0, 0, 0, 0), ptr null, align 8
  %Smpl.val.i = load target("spirv.Sampler"), ptr null, align 8
  %call1.i.i = tail call spir_func target("spirv.SampledImage", void, 1, 0, 0, 0, 0, 0, 0) @_Z20__spirv_SampledImageI14ocl_image2d_ro32__spirv_SampledImage__image2d_roET0_T_11ocl_sampler(target("spirv.Image", void, 1, 0, 0, 0, 0, 0, 0) %4, target("spirv.Sampler") %Smpl.val.i)
  %call2.i.i = tail call fast spir_func noundef nofpclass(nan inf) <4 x float> @_Z30__spirv_ImageSampleExplicitLodI32__spirv_SampledImage__image2d_roDv4_fDv2_fET0_T_T1_if(target("spirv.SampledImage", void, 1, 0, 0, 0, 0, 0, 0) %call1.i.i, <2 x float> noundef nofpclass(nan inf) %vecinit3.i.i, i32 noundef 2, float noundef nofpclass(nan inf) 0.000000e+00)
  %call1.i.i150 = tail call spir_func target("spirv.SampledImage", void, 1, 0, 0, 0, 0, 0, 0) @_Z20__spirv_SampledImageI14ocl_image2d_ro32__spirv_SampledImage__image2d_roET0_T_11ocl_sampler(target("spirv.Image", void, 1, 0, 0, 0, 0, 0, 0) %4, target("spirv.Sampler") %Smpl.val.i)
  ret void
}

; CHECK-DAG: declare spir_func ptr addrspace(1) @_Z20__spirv_SampledImageI14ocl_image2d_ro32__spirv_SampledImage__image2d_roET0_T_11ocl_sampler(ptr addrspace(1), ptr addrspace(2))

; CHECK-DAG: declare spir_func <4 x float> @_Z30__spirv_ImageSampleExplicitLodI32__spirv_SampledImage__image2d_roDv4_fDv2_fET0_T_T1_if(ptr addrspace(1), <2 x float>, i32, float)

declare spir_func target("spirv.SampledImage", void, 1, 0, 0, 0, 0, 0, 0) @_Z20__spirv_SampledImageI14ocl_image2d_ro32__spirv_SampledImage__image2d_roET0_T_11ocl_sampler(target("spirv.Image", void, 1, 0, 0, 0, 0, 0, 0), target("spirv.Sampler"))

declare spir_func <4 x float> @_Z30__spirv_ImageSampleExplicitLodI32__spirv_SampledImage__image2d_roDv4_fDv2_fET0_T_T1_if(target("spirv.SampledImage", void, 1, 0, 0, 0, 0, 0, 0), <2 x float>, i32, float)

uselistorder ptr @_Z20__spirv_SampledImageI14ocl_image2d_ro32__spirv_SampledImage__image2d_roET0_T_11ocl_sampler, { 1, 0 }

!spirv.Source = !{!0}

!0 = !{i32 4, i32 100000}

; DEBUGIFY-NOT: WARNING
