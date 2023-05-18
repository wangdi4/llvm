; RUN: opt -passes=sycl-kernel-target-ext-type-lower -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-target-ext-type-lower -S %s | FileCheck %s

; Check that users of function byval argument containing target extension type
; are replaced.
; Also check byval argument in function declaration is replaced with new type.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; CHECK: [[SYCLV1DeviceEvent:%"class.sycl::_V1::device_event.*"]] = type { ptr }

%"class.sycl::_V1::device_event" = type { target("spirv.Event") }

$_ZNK4sycl3_V15groupILi1EE8wait_forIJNS0_12device_eventEEEEvDpT_ = comdat any

declare void @llvm.memcpy.p4.p4.i64(ptr addrspace(4) noalias nocapture writeonly, ptr addrspace(4) noalias nocapture readonly, i64, i1 immarg) #0

define weak_odr dso_local spir_func void @_ZNK4sycl3_V15groupILi1EE8wait_forIJNS0_12device_eventEEEEvDpT_(ptr addrspace(4) noundef align 8 dereferenceable_or_null(32) %this, ptr noundef byval(%"class.sycl::_V1::device_event") align 8 %Events) #1 comdat align 2 !srcloc !1 !work_item_scope !2 {
entry:
; CHECK-LABEL: define weak_odr dso_local spir_func void @_ZNK4sycl3_V15groupILi1EE8wait_forIJNS0_12device_eventEEEEvDpT_(
; CHECK-SAME: ptr addrspace(4) noundef align 8 dereferenceable_or_null(32) %this, ptr noundef byval([[SYCLV1DeviceEvent]]) align 8 %Events)
; CHECK-SAME: !arg_type_null_val [[MD:![0-9]+]]
; CHECK: %agg.tmp = alloca [[SYCLV1DeviceEvent]], align 8
; CHECK: call spir_func void @_ZNK4sycl3_V15groupILi1EE13waitForHelperENS0_12device_eventE(ptr addrspace(4) noundef align 8 dereferenceable_or_null(32) %this1, ptr noundef byval([[SYCLV1DeviceEvent]]) align 8 %agg.tmp.ascast.ascast)

  %this.addr = alloca ptr addrspace(4), align 8
  %agg.tmp = alloca %"class.sycl::_V1::device_event", align 8
  %this.addr.ascast = addrspacecast ptr %this.addr to ptr addrspace(4)
  %agg.tmp.ascast = addrspacecast ptr %agg.tmp to ptr addrspace(4)
  store ptr addrspace(4) %this, ptr addrspace(4) %this.addr.ascast, align 8
  %Events.ascast = addrspacecast ptr %Events to ptr addrspace(4)
  %this1 = load ptr addrspace(4), ptr addrspace(4) %this.addr.ascast, align 8
  call void @llvm.memcpy.p4.p4.i64(ptr addrspace(4) align 8 %agg.tmp.ascast, ptr addrspace(4) align 8 %Events.ascast, i64 8, i1 false)
  %agg.tmp.ascast.ascast = addrspacecast ptr addrspace(4) %agg.tmp.ascast to ptr
  call spir_func void @_ZNK4sycl3_V15groupILi1EE13waitForHelperENS0_12device_eventE(ptr addrspace(4) noundef align 8 dereferenceable_or_null(32) %this1, ptr noundef byval(%"class.sycl::_V1::device_event") align 8 %agg.tmp.ascast.ascast) #2
  ret void
}

; CHECK: declare dso_local spir_func void @_ZNK4sycl3_V15groupILi1EE13waitForHelperENS0_12device_eventE(ptr addrspace(4) noundef align 8 dereferenceable_or_null(32), ptr noundef byval([[SYCLV1DeviceEvent]]) align 8)

declare dso_local spir_func void @_ZNK4sycl3_V15groupILi1EE13waitForHelperENS0_12device_eventE(ptr addrspace(4) noundef align 8 dereferenceable_or_null(32), ptr noundef byval(%"class.sycl::_V1::device_event") align 8) #1 align 2

attributes #0 = { nocallback nofree nounwind willreturn memory(argmem: readwrite) }
attributes #1 = { convergent mustprogress noinline norecurse nounwind optnone }
attributes #2 = { convergent nounwind }

!spirv.Source = !{!0}

!0 = !{i32 4, i32 100000}
!1 = !{i32 9118773}
!2 = !{}

; DEBUGIFY-NOT: WARNING
