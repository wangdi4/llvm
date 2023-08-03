; RUN: opt -passes=sycl-kernel-target-ext-type-lower -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-target-ext-type-lower -S %s | FileCheck %s

; Compiled from OpenCL kernel:
; kernel void test(global int* p) {
;   local int val;
;   val = -1.0;
;   event_t ev = async_work_group_copy(p, &val, sizeof(val), 0);
;   wait_group_events(1, &ev);
; }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

@test.val = internal addrspace(3) global i32 undef, align 4

define internal spir_kernel void @test(ptr addrspace(1) noundef align 4 %p) #0 !kernel_arg_addr_space !0 !kernel_arg_access_qual !1 !kernel_arg_type !2 !kernel_arg_base_type !2 !kernel_arg_type_qual !3 !kernel_arg_name !4 !kernel_arg_host_accessible !5 !kernel_arg_pipe_depth !6 !kernel_arg_pipe_io !3 !kernel_arg_buffer_location !3 {
entry:
  %p.addr = alloca ptr addrspace(1), align 8
; CHECK: %ev = alloca ptr, align 8
  %ev = alloca target("spirv.Event"), align 8
  store ptr addrspace(1) %p, ptr %p.addr, align 8, !tbaa !7
  store i32 -1, ptr addrspace(3) @test.val, align 4, !tbaa !11
  %0 = load ptr addrspace(1), ptr %p.addr, align 8, !tbaa !7
; CHECK: %call = call spir_func ptr @_Z21async_work_group_copyPU3AS1iPU3AS3Kim9ocl_event(ptr addrspace(1) noundef {{.*}}, ptr addrspace(3) noundef @test.val, i64 noundef 4, ptr null)
; CHECK: store ptr %call, ptr %ev
  %call = call spir_func target("spirv.Event") @_Z21async_work_group_copyPU3AS1iPU3AS3Kim9ocl_event(ptr addrspace(1) noundef %0, ptr addrspace(3) noundef @test.val, i64 noundef 4, target("spirv.Event") zeroinitializer) #2
  store target("spirv.Event") %call, ptr %ev, align 8, !tbaa !13
  %ev.ascast = addrspacecast ptr %ev to ptr addrspace(4)
  call spir_func void @_Z17wait_group_eventsiPU3AS49ocl_event(i32 noundef 1, ptr addrspace(4) noundef %ev.ascast) #2
  ret void
}

; CHECK: declare spir_func ptr @_Z21async_work_group_copyPU3AS1iPU3AS3Kim9ocl_event(ptr addrspace(1) noundef, ptr addrspace(3) noundef, i64 noundef, ptr)

declare spir_func target("spirv.Event") @_Z21async_work_group_copyPU3AS1iPU3AS3Kim9ocl_event(ptr addrspace(1) noundef, ptr addrspace(3) noundef, i64 noundef, target("spirv.Event")) #2

declare spir_func void @_Z17wait_group_eventsiPU3AS49ocl_event(i32 noundef, ptr addrspace(4) noundef) #2

attributes #0 = { convergent norecurse nounwind }
attributes #1 = { nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { convergent nounwind }

!0 = !{i32 1}
!1 = !{!"none"}
!2 = !{!"int*"}
!3 = !{!""}
!4 = !{!"p"}
!5 = !{i1 false}
!6 = !{i32 0}
!7 = !{!8, !8, i64 0}
!8 = !{!"any pointer", !9, i64 0}
!9 = !{!"omnipotent char", !10, i64 0}
!10 = !{!"Simple C/C++ TBAA"}
!11 = !{!12, !12, i64 0}
!12 = !{!"int", !9, i64 0}
!13 = !{!14, !14, i64 0}
!14 = !{!"event_t", !9, i64 0}

; DEBUGIFY-NOT: WARNING
