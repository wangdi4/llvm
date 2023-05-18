; RUN: opt -passes=sycl-kernel-target-ext-type-lower -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-target-ext-type-lower -S %s | FileCheck %s

; Compiled from OpenCL kernel:
; kernel void device_kernel(__global int * inout, clk_event_t event) {
;   *inout = is_valid_event(event);
;   release_event(event);
; }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define dso_local spir_kernel void @device_kernel(ptr addrspace(1) noundef align 4 %inout, target("spirv.DeviceEvent") %event) #0 !kernel_arg_addr_space !0 !kernel_arg_access_qual !1 !kernel_arg_type !2 !kernel_arg_base_type !2 !kernel_arg_type_qual !3 !kernel_arg_name !4 !kernel_arg_host_accessible !5 !kernel_arg_pipe_depth !6 !kernel_arg_pipe_io !3 !kernel_arg_buffer_location !3 {
entry:
; CHECK-LABEL: define dso_local spir_kernel void @device_kernel(ptr addrspace(1) noundef align 4 %inout, ptr %event)
; CHECK-SAME: !arg_type_null_val [[MD_ARG_TY_NULL:![0-9]+]]

  %inout.addr = alloca ptr addrspace(1), align 8
; CHECK: %event.addr = alloca ptr, align 8
  %event.addr = alloca target("spirv.DeviceEvent"), align 8
  store ptr addrspace(1) %inout, ptr %inout.addr, align 8, !tbaa !7
; CHECK: store ptr {{.*}}, ptr %event.addr, align 8
; CHECK: load ptr, ptr %event.addr, align 8
; CHECK: call spir_func zeroext i1 @_Z14is_valid_event12ocl_clkevent(ptr
  store target("spirv.DeviceEvent") %event, ptr %event.addr, align 8, !tbaa !11
  %0 = load target("spirv.DeviceEvent"), ptr %event.addr, align 8, !tbaa !11
  %call = call spir_func zeroext i1 @_Z14is_valid_event12ocl_clkevent(target("spirv.DeviceEvent") %0) #1
  %conv = zext i1 %call to i32
  %1 = load ptr addrspace(1), ptr %inout.addr, align 8, !tbaa !7
  store i32 %conv, ptr addrspace(1) %1, align 4, !tbaa !13
; CHECK: load ptr, ptr %event.addr, align 8
; CHECK: call spir_func void @_Z13release_event12ocl_clkevent(ptr
  %2 = load target("spirv.DeviceEvent"), ptr %event.addr, align 8, !tbaa !11
  call spir_func void @_Z13release_event12ocl_clkevent(target("spirv.DeviceEvent") %2) #1
  ret void
}

; CHECK-DAG: declare spir_func zeroext i1 @_Z14is_valid_event12ocl_clkevent(ptr)

declare spir_func zeroext i1 @_Z14is_valid_event12ocl_clkevent(target("spirv.DeviceEvent")) #1

; CHECK-DAG: declare spir_func void @_Z13release_event12ocl_clkevent(ptr)

declare spir_func void @_Z13release_event12ocl_clkevent(target("spirv.DeviceEvent")) #1

attributes #0 = { convergent norecurse nounwind }
attributes #1 = { convergent nounwind }

; CHECK: [[MD_ARG_TY_NULL]] = !{ptr addrspace(1) null, target("spirv.DeviceEvent") zeroinitializer}

!0 = !{i32 1, i32 0}
!1 = !{!"none", !"none"}
!2 = !{!"int*", !"clk_event_t"}
!3 = !{!"", !""}
!4 = !{!"inout", !"event"}
!5 = !{i1 false, i1 false}
!6 = !{i32 0, i32 0}
!7 = !{!8, !8, i64 0}
!8 = !{!"any pointer", !9, i64 0}
!9 = !{!"omnipotent char", !10, i64 0}
!10 = !{!"Simple C/C++ TBAA"}
!11 = !{!12, !12, i64 0}
!12 = !{!"clk_event_t", !9, i64 0}
!13 = !{!14, !14, i64 0}
!14 = !{!"int", !9, i64 0}

; DEBUGIFY-NOT: WARNING
