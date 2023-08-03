; RUN: opt -passes=sycl-kernel-target-ext-type-lower -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-target-ext-type-lower -S %s | FileCheck %s

; Check that function argument sret type, that contains target extension type,
; is correctly replaced.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; CHECK: %"class.sycl::_V1::device_event.0" = type { ptr }

%"class.sycl::_V1::h_item" = type { %"class.sycl::_V1::item", %"class.sycl::_V1::item", %"class.sycl::_V1::item" }
%"class.sycl::_V1::item" = type { %"struct.sycl::_V1::detail::ItemBase" }
%"struct.sycl::_V1::detail::ItemBase" = type { %"class.sycl::_V1::range", %"class.sycl::_V1::range" }
%"class.sycl::_V1::range" = type { %"class.sycl::_V1::detail::array" }
%"class.sycl::_V1::detail::array" = type { [1 x i64] }
%"class.sycl::_V1::device_event" = type { target("spirv.Event") }
%"class.sycl::_V1::multi_ptr.7" = type { ptr addrspace(3) }
%union.anon = type { ptr addrspace(1) }

declare void @llvm.memcpy.p4.p4.i64(ptr addrspace(4) noalias nocapture writeonly, ptr addrspace(4) noalias nocapture readonly, i64, i1 immarg) #0

define hidden spir_func void @_ZZZZL26test_unstrided_copy_g_to_lRN4sycl3_V15queueEENKUlRNS0_7handlerEE_clES4_ENKUlNS0_5groupILi1EEEE_clES7_ENKUlNS0_6h_itemILi1EEEE_clESA_(ptr addrspace(4) noundef align 8 dereferenceable_or_null(40) %this, ptr noundef byval(%"class.sycl::_V1::h_item") align 8 %I) {
entry:
; CHECK: define hidden spir_func void @_ZZZZL26test_unstrided_copy_g_to_lRN4sycl3_V15queueEENKUlRNS0_7handlerEE_clES4_ENKUlNS0_5groupILi1EEEE_clES7_ENKUlNS0_6h_itemILi1EEEE_clESA_(ptr addrspace(4) noundef align 8 dereferenceable_or_null(40) %this, ptr noundef byval(%"class.sycl::_V1::h_item") align 8 %I)
; CHECK: %copyin_event = alloca %"class.sycl::_V1::device_event.0", align 8
; CHECK: %agg.tmp4 = alloca %"class.sycl::_V1::device_event.0", align 8
; CHECK: call spir_func void @_ZNK4sycl3_V15groupILi1EE21async_work_group_copyIiEENS0_12device_eventENS0_9multi_ptrIT_LNS0_6access13address_spaceE3ELNS7_9decoratedE2EEENS5_IS6_LS8_1ELS9_2EEEm(ptr addrspace(4) sret(%"class.sycl::_V1::device_event.0") align 8 %copyin_event.ascast, ptr addrspace(4) noundef align 8 dereferenceable_or_null(32) %0, ptr noundef byval(%"class.sycl::_V1::multi_ptr.7") align 8 %agg.tmp.ascast.ascast, ptr noundef byval(%union.anon) align 8 %agg.tmp2.ascast.ascast, i64 noundef 4)
; CHECK: call spir_func void @_ZNK4sycl3_V15groupILi1EE8wait_forIJNS0_12device_eventEEEEvDpT_(ptr addrspace(4) noundef align 8 dereferenceable_or_null(32) %3, ptr noundef byval(%"class.sycl::_V1::device_event.0") align 8 %agg.tmp4.ascast.ascast)

  %copyin_event = alloca %"class.sycl::_V1::device_event", align 8
  %agg.tmp4 = alloca %"class.sycl::_V1::device_event", align 8
  %copyin_event.ascast = addrspacecast ptr null to ptr addrspace(4)
  %0 = load ptr addrspace(4), ptr addrspace(4) null, align 8
  %1 = load ptr addrspace(4), ptr addrspace(4) null, align 8
  %2 = load ptr addrspace(4), ptr addrspace(4) null, align 8
  %agg.tmp.ascast.ascast = addrspacecast ptr addrspace(4) null to ptr
  %agg.tmp2.ascast.ascast = addrspacecast ptr addrspace(4) null to ptr
  call spir_func void @_ZNK4sycl3_V15groupILi1EE21async_work_group_copyIiEENS0_12device_eventENS0_9multi_ptrIT_LNS0_6access13address_spaceE3ELNS7_9decoratedE2EEENS5_IS6_LS8_1ELS9_2EEEm(ptr addrspace(4) sret(%"class.sycl::_V1::device_event") align 8 %copyin_event.ascast, ptr addrspace(4) noundef align 8 dereferenceable_or_null(32) %0, ptr noundef byval(%"class.sycl::_V1::multi_ptr.7") align 8 %agg.tmp.ascast.ascast, ptr noundef byval(%union.anon) align 8 %agg.tmp2.ascast.ascast, i64 noundef 4)
  %3 = load ptr addrspace(4), ptr addrspace(4) null, align 8
  %agg.tmp4.ascast.ascast = addrspacecast ptr addrspace(4) null to ptr
  call spir_func void @_ZNK4sycl3_V15groupILi1EE8wait_forIJNS0_12device_eventEEEEvDpT_(ptr addrspace(4) noundef align 8 dereferenceable_or_null(32) %3, ptr noundef byval(%"class.sycl::_V1::device_event") align 8 %agg.tmp4.ascast.ascast)
  ret void
}

; CHECK: define weak_odr dso_local spir_func void @_ZN4sycl3_V112device_event4waitEv(ptr addrspace(4) noundef align 8 dereferenceable_or_null(8) %this)
; CHECK: %m_Event = getelementptr inbounds %"class.sycl::_V1::device_event.0", ptr addrspace(4) %this1, i32 0, i32 0
; CHECK: call spir_func void @_Z23__spirv_GroupWaitEventsjiP9ocl_event(i32 noundef 2, i32 noundef 1, ptr addrspace(4) noundef %m_Event)

define weak_odr dso_local spir_func void @_ZNK4sycl3_V15groupILi1EE21async_work_group_copyIiEENS0_12device_eventENS0_9multi_ptrIT_LNS0_6access13address_spaceE3ELNS7_9decoratedE2EEENS5_IS6_LS8_1ELS9_2EEEm(ptr addrspace(4) noalias sret(%"class.sycl::_V1::device_event") align 8 %agg.result, ptr addrspace(4) noundef align 8 dereferenceable_or_null(32) %this, ptr noundef byval(%"class.sycl::_V1::multi_ptr.7") align 8 %dest, ptr noundef byval(%union.anon) align 8 %src, i64 noundef %numElements) {
entry:
; CHECK: define weak_odr dso_local spir_func void @_ZNK4sycl3_V15groupILi1EE21async_work_group_copyIiEENS0_12device_eventENS0_9multi_ptrIT_LNS0_6access13address_spaceE3ELNS7_9decoratedE2EEENS5_IS6_LS8_1ELS9_2EEEm(ptr addrspace(4) noalias sret(%"class.sycl::_V1::device_event.0") align 8 %agg.result, ptr addrspace(4) noundef align 8 dereferenceable_or_null(32) %this, ptr noundef byval(%"class.sycl::_V1::multi_ptr.7") align 8 %dest, ptr noundef byval(%union.anon) align 8 %src, i64 noundef %numElements)
; CHECK: !arg_type_null_val
; CHECK: call spir_func void @_ZNK4sycl3_V15groupILi1EE21async_work_group_copyIiEENSt9enable_ifIXntsr6detail7is_boolIT_EE5valueENS0_12device_eventEE4typeENS0_9multi_ptrIS5_LNS0_6access13address_spaceE3ELNSA_9decoratedE2EEENS9_IS5_LSB_1ELSC_2EEEmm(ptr addrspace(4) sret(%"class.sycl::_V1::device_event.0") align 8 %agg.result, ptr addrspace(4) noundef align 8 dereferenceable_or_null(32) %this1, ptr noundef byval(%"class.sycl::_V1::multi_ptr.7") align 8 %agg.tmp.ascast.ascast, ptr noundef byval(%union.anon) align 8 %agg.tmp2.ascast.ascast, i64 noundef %0, i64 noundef 1)

  %this1 = load ptr addrspace(4), ptr addrspace(4) null, align 8
  %0 = load i64, ptr addrspace(4) null, align 8
  %agg.tmp.ascast.ascast = addrspacecast ptr addrspace(4) null to ptr
  %agg.tmp2.ascast.ascast = addrspacecast ptr addrspace(4) null to ptr
  call spir_func void @_ZNK4sycl3_V15groupILi1EE21async_work_group_copyIiEENSt9enable_ifIXntsr6detail7is_boolIT_EE5valueENS0_12device_eventEE4typeENS0_9multi_ptrIS5_LNS0_6access13address_spaceE3ELNSA_9decoratedE2EEENS9_IS5_LSB_1ELSC_2EEEmm(ptr addrspace(4) sret(%"class.sycl::_V1::device_event") align 8 %agg.result, ptr addrspace(4) noundef align 8 dereferenceable_or_null(32) %this1, ptr noundef byval(%"class.sycl::_V1::multi_ptr.7") align 8 %agg.tmp.ascast.ascast, ptr noundef byval(%union.anon) align 8 %agg.tmp2.ascast.ascast, i64 noundef %0, i64 noundef 1)
  ret void
}

define weak_odr dso_local spir_func void @_ZNK4sycl3_V15groupILi1EE8wait_forIJNS0_12device_eventEEEEvDpT_(ptr addrspace(4) noundef align 8 dereferenceable_or_null(32) %this, ptr noundef byval(%"class.sycl::_V1::device_event") align 8 %Events) {
entry:
; CHECK: define weak_odr dso_local spir_func void @_ZNK4sycl3_V15groupILi1EE8wait_forIJNS0_12device_eventEEEEvDpT_(ptr addrspace(4) noundef align 8 dereferenceable_or_null(32) %this, ptr noundef byval(%"class.sycl::_V1::device_event.0") align 8 %Events)
; CHECK-SAME: !arg_type_null_val
; CHECK: %agg.tmp = alloca %"class.sycl::_V1::device_event.0", align 8
; CHECK: %Events.ascast = addrspacecast ptr %Events to ptr addrspace(4)
; CHECK: call void @llvm.memcpy.p4.p4.i64(ptr addrspace(4) align 8 %agg.tmp.ascast, ptr addrspace(4) align 8 %Events.ascast, i64 8, i1 false)
; CHECK: call spir_func void @_ZNK4sycl3_V15groupILi1EE13waitForHelperENS0_12device_eventE(ptr addrspace(4) noundef align 8 dereferenceable_or_null(32) %this1, ptr noundef byval(%"class.sycl::_V1::device_event.0") align 8 %agg.tmp.ascast.ascast)

  %agg.tmp = alloca %"class.sycl::_V1::device_event", align 8
  %agg.tmp.ascast = addrspacecast ptr null to ptr addrspace(4)
  %Events.ascast = addrspacecast ptr %Events to ptr addrspace(4)
  %this1 = load ptr addrspace(4), ptr addrspace(4) null, align 8
  call void @llvm.memcpy.p4.p4.i64(ptr addrspace(4) align 8 %agg.tmp.ascast, ptr addrspace(4) align 8 %Events.ascast, i64 8, i1 false)
  %agg.tmp.ascast.ascast = addrspacecast ptr addrspace(4) null to ptr
  call spir_func void @_ZNK4sycl3_V15groupILi1EE13waitForHelperENS0_12device_eventE(ptr addrspace(4) noundef align 8 dereferenceable_or_null(32) %this1, ptr noundef byval(%"class.sycl::_V1::device_event") align 8 %agg.tmp.ascast.ascast)
  ret void
}

declare spir_func void @_ZNK4sycl3_V15groupILi1EE13waitForHelperENS0_12device_eventE(ptr addrspace(4), ptr byval(%"class.sycl::_V1::device_event"))

define weak_odr dso_local spir_func void @_ZN4sycl3_V112device_event4waitEv(ptr addrspace(4) noundef align 8 dereferenceable_or_null(8) %this) {
entry:
  %this1 = load ptr addrspace(4), ptr addrspace(4) null, align 8
  %m_Event = getelementptr inbounds %"class.sycl::_V1::device_event", ptr addrspace(4) %this1, i32 0, i32 0
  call spir_func void @_Z23__spirv_GroupWaitEventsjiP9ocl_event(i32 noundef 2, i32 noundef 1, ptr addrspace(4) noundef %m_Event)
  ret void
}

declare spir_func void @_Z23__spirv_GroupWaitEventsjiP9ocl_event(i32, i32, ptr addrspace(4))

define weak_odr dso_local spir_func void @_ZNK4sycl3_V15groupILi1EE21async_work_group_copyIiEENSt9enable_ifIXntsr6detail7is_boolIT_EE5valueENS0_12device_eventEE4typeENS0_9multi_ptrIS5_LNS0_6access13address_spaceE3ELNSA_9decoratedE2EEENS9_IS5_LSB_1ELSC_2EEEmm(ptr addrspace(4) noalias sret(%"class.sycl::_V1::device_event") align 8 %agg.result, ptr addrspace(4) noundef align 8 dereferenceable_or_null(32) %this, ptr noundef byval(%"class.sycl::_V1::multi_ptr.7") align 8 %dest, ptr noundef byval(%union.anon) align 8 %src, i64 noundef %numElements, i64 noundef %srcStride) {
entry:
; CHECK: define weak_odr dso_local spir_func void @_ZNK4sycl3_V15groupILi1EE21async_work_group_copyIiEENSt9enable_ifIXntsr6detail7is_boolIT_EE5valueENS0_12device_eventEE4typeENS0_9multi_ptrIS5_LNS0_6access13address_spaceE3ELNSA_9decoratedE2EEENS9_IS5_LSB_1ELSC_2EEEmm(ptr addrspace(4) noalias sret(%"class.sycl::_V1::device_event.0") align 8 %agg.result, ptr addrspace(4) noundef align 8 dereferenceable_or_null(32) %this, ptr noundef byval(%"class.sycl::_V1::multi_ptr.7") align 8 %dest, ptr noundef byval(%union.anon) align 8 %src, i64 noundef %numElements, i64 noundef %srcStride)
; CHECK-SAME: !arg_type_null_val
; CHECK: %E = alloca ptr, align 8
; CHECK: %call = call spir_func noundef ptr addrspace(3) @_ZNK4sycl3_V19multi_ptrIiLNS0_6access13address_spaceE3ELNS2_9decoratedE2EE3getEv(ptr addrspace(4) noundef align 8 dereferenceable_or_null(8) %dest.ascast)
; CHECK: %call2 = call spir_func noundef ptr addrspace(1) @_ZNK4sycl3_V19multi_ptrIiLNS0_6access13address_spaceE1ELNS2_9decoratedE2EE3getEv(ptr addrspace(4) noundef align 8 dereferenceable_or_null(8) %src.ascast)
; CHECK: %call3 = call spir_func ptr @_Z22__spirv_GroupAsyncCopyjPU3AS3iPU3AS1Kimm9ocl_event(i32 noundef 2, ptr addrspace(3) noundef %call, ptr addrspace(1) noundef %call2, i64 noundef %0, i64 noundef %1, ptr null)
; CHECK: store ptr %call3, ptr addrspace(4) %E.ascast, align 8
; CHECK: load ptr, ptr addrspace(4) %E.ascast, align 8
; CHECK: call spir_func void @_ZN4sycl3_V112device_eventC2E9ocl_event(ptr addrspace(4) noundef align 8 dereferenceable_or_null(8) %agg.result, ptr %2)

  %E = alloca target("spirv.Event"), align 8
  %E.ascast = addrspacecast ptr null to ptr addrspace(4)
  %dest.ascast = addrspacecast ptr null to ptr addrspace(4)
  %src.ascast = addrspacecast ptr null to ptr addrspace(4)
  %call = call spir_func noundef ptr addrspace(3) @_ZNK4sycl3_V19multi_ptrIiLNS0_6access13address_spaceE3ELNS2_9decoratedE2EE3getEv(ptr addrspace(4) noundef align 8 dereferenceable_or_null(8) %dest.ascast)
  %call2 = call spir_func noundef ptr addrspace(1) @_ZNK4sycl3_V19multi_ptrIiLNS0_6access13address_spaceE1ELNS2_9decoratedE2EE3getEv(ptr addrspace(4) noundef align 8 dereferenceable_or_null(8) %src.ascast)
  %0 = load i64, ptr addrspace(4) null, align 8
  %1 = load i64, ptr addrspace(4) null, align 8
  %call3 = call spir_func target("spirv.Event") @_Z22__spirv_GroupAsyncCopyjPU3AS3iPU3AS1Kimm9ocl_event(i32 noundef 2, ptr addrspace(3) noundef %call, ptr addrspace(1) noundef %call2, i64 noundef %0, i64 noundef %1, target("spirv.Event") zeroinitializer)
  store target("spirv.Event") %call3, ptr addrspace(4) %E.ascast, align 8
  %2 = load target("spirv.Event"), ptr addrspace(4) %E.ascast, align 8
  call spir_func void @_ZN4sycl3_V112device_eventC2E9ocl_event(ptr addrspace(4) noundef align 8 dereferenceable_or_null(8) %agg.result, target("spirv.Event") %2)
  ret void
}

declare spir_func ptr addrspace(3) @_ZNK4sycl3_V19multi_ptrIiLNS0_6access13address_spaceE3ELNS2_9decoratedE2EE3getEv(ptr addrspace(4))

declare spir_func ptr addrspace(1) @_ZNK4sycl3_V19multi_ptrIiLNS0_6access13address_spaceE1ELNS2_9decoratedE2EE3getEv(ptr addrspace(4))

declare spir_func target("spirv.Event") @_Z22__spirv_GroupAsyncCopyjPU3AS3iPU3AS1Kimm9ocl_event(i32, ptr addrspace(3), ptr addrspace(1), i64, i64, target("spirv.Event"))

define weak_odr dso_local spir_func void @_ZN4sycl3_V112device_eventC2E9ocl_event(ptr addrspace(4) noundef align 8 dereferenceable_or_null(8) %this, target("spirv.Event") %Event) {
entry:
; CHECK: define weak_odr dso_local spir_func void @_ZN4sycl3_V112device_eventC2E9ocl_event(ptr addrspace(4) noundef align 8 dereferenceable_or_null(8) %this, ptr %Event)
; CHECK-SAME: !arg_type_null_val
; CHECK: %Event.addr = alloca ptr, align 8
; CHECK: store ptr %Event, ptr addrspace(4) %Event.addr.ascast, align 8
; CHECK: %m_Event = getelementptr inbounds %"class.sycl::_V1::device_event.0", ptr addrspace(4) %this1, i32 0, i32 0
; CHECK: load ptr, ptr addrspace(4) %Event.addr.ascast, align 8
; CHECK: store ptr %0, ptr addrspace(4) %m_Event, align 8

  %Event.addr = alloca target("spirv.Event"), align 8
  %Event.addr.ascast = addrspacecast ptr null to ptr addrspace(4)
  store target("spirv.Event") %Event, ptr addrspace(4) %Event.addr.ascast, align 8
  %this1 = load ptr addrspace(4), ptr addrspace(4) null, align 8
  %m_Event = getelementptr inbounds %"class.sycl::_V1::device_event", ptr addrspace(4) %this1, i32 0, i32 0
  %0 = load target("spirv.Event"), ptr addrspace(4) %Event.addr.ascast, align 8
  store target("spirv.Event") %0, ptr addrspace(4) %m_Event, align 8
  ret void
}

attributes #0 = { nocallback nofree nounwind willreturn memory(argmem: readwrite) }

!spirv.Source = !{!0}

!0 = !{i32 4, i32 100000}

; DEBUGIFY-NOT: WARNING
