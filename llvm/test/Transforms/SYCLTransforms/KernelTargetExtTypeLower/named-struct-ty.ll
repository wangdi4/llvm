; RUN: opt -passes=sycl-kernel-target-ext-type-lower -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-target-ext-type-lower -S %s | FileCheck %s

; Check that target extension type in named struct type is replaced.

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64_x86_64-unknown-unknown"

; CHECK: [[ACCESSOR:%"class.sycl::_V1::detail::image_accessor.*"]] = type { ptr addrspace(1), [24 x i8] }

%"class.sycl::_V1::vec" = type { <4 x float> }
%"class.sycl::_V1::detail::image_accessor" = type { target("spirv.Image", void, 1, 0, 0, 0, 0, 0, 0), [24 x i8] }
%"class.sycl::_V1::vec.3" = type { <2 x i32> }

define weak_odr dso_local spir_func void @_ZNK4sycl3_V16detail14image_accessorINS0_3vecIfLi4EEELi2ELNS0_6access4modeE1024ELNS5_6targetE2017ELNS5_11placeholderE0EE4readINS3_IiLi2EEELi2EvEES4_RKT_(ptr addrspace(4) noalias sret(%"class.sycl::_V1::vec") align 16 %agg.result, ptr addrspace(4) noundef align 8 dereferenceable_or_null(32) %this, ptr addrspace(4) noundef align 8 dereferenceable(8) %Coords) {
entry:
; CHECK-LABEL: define weak_odr dso_local spir_func void @_ZNK4sycl3_V16detail14image_accessorINS0_3vecIfLi4EEELi2ELNS0_6access4modeE1024ELNS5_6targetE2017ELNS5_11placeholderE0EE4readINS3_IiLi2EEELi2EvEES4_RKT_(
; CHECK: %MImageObj = getelementptr inbounds [[ACCESSOR]], ptr addrspace(4) %this1, i32 0, i32 0
; CHECK: load ptr addrspace(1), ptr addrspace(4) %MImageObj, align 8
; CHECK: call spir_func void @_ZL19__invoke__ImageReadIN4sycl3_V13vecIfLi4EEE14ocl_image2d_roNS2_IiLi2EEEET_T0_T1_(ptr addrspace(4) sret(%"class.sycl::_V1::vec") align 16 %agg.result, ptr addrspace(1) %0, ptr noundef byval(%"class.sycl::_V1::vec.3") align 8 %agg.tmp.ascast.ascast)

  %this1 = load ptr addrspace(4), ptr addrspace(4) null, align 8
  %MImageObj = getelementptr inbounds %"class.sycl::_V1::detail::image_accessor", ptr addrspace(4) %this1, i32 0, i32 0
  %0 = load target("spirv.Image", void, 1, 0, 0, 0, 0, 0, 0), ptr addrspace(4) %MImageObj, align 8
  %agg.tmp.ascast.ascast = addrspacecast ptr addrspace(4) null to ptr
  call spir_func void @_ZL19__invoke__ImageReadIN4sycl3_V13vecIfLi4EEE14ocl_image2d_roNS2_IiLi2EEEET_T0_T1_(ptr addrspace(4) sret(%"class.sycl::_V1::vec") align 16 %agg.result, target("spirv.Image", void, 1, 0, 0, 0, 0, 0, 0) %0, ptr noundef byval(%"class.sycl::_V1::vec.3") align 8 %agg.tmp.ascast.ascast)
  ret void
}

define weak_odr dso_local void @_ZN4sycl3_V16detail14image_accessorINS0_3vecIfLi4EEELi2ELNS0_6access4modeE1024ELNS5_6targetE2017ELNS5_11placeholderE0EEC2Ev(ptr addrspace(4) noundef align 8 dereferenceable_or_null(32) %this) {
entry:
; CHECK-LABEL: define weak_odr dso_local void @_ZN4sycl3_V16detail14image_accessorINS0_3vecIfLi4EEELi2ELNS0_6access4modeE1024ELNS5_6targetE2017ELNS5_11placeholderE0EEC2Ev(
; CHECK: %MImageObj = getelementptr inbounds [[ACCESSOR]], ptr addrspace(4) %this1, i32 0, i32 0
; CHECK: store ptr addrspace(1) null, ptr addrspace(4) %MImageObj, align 8

  %this.addr = alloca ptr addrspace(4), align 8
  %this.addr.ascast = addrspacecast ptr %this.addr to ptr addrspace(4)
  store ptr addrspace(4) %this, ptr addrspace(4) %this.addr.ascast, align 8
  %this1 = load ptr addrspace(4), ptr addrspace(4) %this.addr.ascast, align 8
  %MImageObj = getelementptr inbounds %"class.sycl::_V1::detail::image_accessor", ptr addrspace(4) %this1, i32 0, i32 0
  store target("spirv.Image", void, 1, 0, 0, 0, 0, 0, 0) zeroinitializer, ptr addrspace(4) %MImageObj, align 8
  ret void
}

; CHECK: declare spir_func void @_ZL19__invoke__ImageReadIN4sycl3_V13vecIfLi4EEE14ocl_image2d_roNS2_IiLi2EEEET_T0_T1_(ptr addrspace(4), ptr addrspace(1), ptr)

declare spir_func void @_ZL19__invoke__ImageReadIN4sycl3_V13vecIfLi4EEE14ocl_image2d_roNS2_IiLi2EEEET_T0_T1_(ptr addrspace(4), target("spirv.Image", void, 1, 0, 0, 0, 0, 0, 0), ptr)

!spirv.Source = !{!0}

!0 = !{i32 4, i32 100000}

; DEBUGIFY-NOT: WARNING
