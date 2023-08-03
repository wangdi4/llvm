; Compiled from:
; ----------------------------------------------------
; #pragma OPENCL EXTENSION cl_intel_channels: enable
;
; channel int ch[5];
;
; __attribute__((autorun))
; __attribute__((max_global_work_dim(0)))
; __attribute__((num_compute_units(4,1,1)))
; __kernel void plus() {
;   int a = read_channel_intel(ch[get_compute_id(0)]);
;   write_channel_intel(ch[get_compute_id(0) + 1], a + 1);
; }
; ----------------------------------------------------
; Clang options: -cc1 -emit-llvm -triple spir64-unknown-unknown-intelfpga -disable-llvm-passes -x cl -cl-std=CL2.0
; ----------------------------------------------------
; RUN: opt -passes=sycl-kernel-autorun-replicator %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-autorun-replicator %s -S | FileCheck %s --implicit-check-not get_compute_id
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64-unknown-unknown-intelfpga"

@ch = addrspace(1) global [5 x target("spirv.Channel")] zeroinitializer, align 4, !packet_size !0, !packet_align !0

; CHECK: define dso_local void @plus() #0 [[ATTRIBUTES:.*]] {
; CHECK: define dso_local void @plus.[[R1:[0-9]+]]() {{.*}} [[ATTRIBUTES]] {
; CHECK: define dso_local void @plus.[[R2:[0-9]+]]() {{.*}} [[ATTRIBUTES]] {
; CHECK: define dso_local void @plus.[[R3:[0-9]+]]() {{.*}} [[ATTRIBUTES]] {
; CHECK: !sycl.kernels = !{![[K:[0-9]+]]}
; CHECK: ![[K]] = !{{{.*}}@plus, {{.*}}@plus.[[R1]], {{.*}}@plus.[[R2]], {{.*}}@plus.[[R3]]}

; Function Attrs: convergent norecurse nounwind
define dso_local void @plus() #0 !kernel_arg_addr_space !2 !kernel_arg_access_qual !2 !kernel_arg_type !2 !kernel_arg_base_type !2 !kernel_arg_type_qual !2 !kernel_arg_host_accessible !2 !kernel_arg_pipe_depth !2 !kernel_arg_pipe_io !2 !kernel_arg_buffer_location !2 !autorun !5 !num_compute_units !6 !max_global_work_dim !7 !arg_type_null_val !2 {
entry:
  %a = alloca i32, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr %a) #3
  %0 = call i64 @get_compute_id(i32 0)
  %arrayidx = getelementptr inbounds [5 x target("spirv.Channel")], ptr addrspace(1) @ch, i64 0, i64 %0
  %1 = load ptr addrspace(1), ptr addrspace(1) %arrayidx, align 4, !tbaa !8
  %call = call i32 @_Z18read_channel_intel11ocl_channeli(ptr addrspace(1) %1) #4
  store i32 %call, ptr %a, align 4, !tbaa !11
  %2 = call i64 @get_compute_id(i32 0)
  %add = add i64 %2, 1
  %arrayidx1 = getelementptr inbounds [5 x target("spirv.Channel")], ptr addrspace(1) @ch, i64 0, i64 %add
  %3 = load ptr addrspace(1), ptr addrspace(1) %arrayidx1, align 4, !tbaa !8
  %4 = load i32, ptr %a, align 4, !tbaa !11
  %add2 = add nsw i32 %4, 1
  call void @_Z19write_channel_intel11ocl_channelii(ptr addrspace(1) %3, i32 noundef %add2) #4
  call void @llvm.lifetime.end.p0(i64 4, ptr %a) #3
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

declare i64 @get_compute_id(i32)

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: convergent nounwind
declare i32 @_Z18read_channel_intel11ocl_channeli(ptr addrspace(1)) #2

; Function Attrs: convergent nounwind
declare void @_Z19write_channel_intel11ocl_channelii(ptr addrspace(1), i32 noundef) #2

attributes #0 = { convergent norecurse nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "uniform-work-group-size"="false" }
attributes #1 = { nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { convergent nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" }
attributes #3 = { nounwind }
attributes #4 = { convergent nounwind }

!opencl.ocl.version = !{!1}
!opencl.spir.version = !{!1}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}
!sycl.kernels = !{!4}

!0 = !{i32 4}
!1 = !{i32 2, i32 0}
!2 = !{}
!3 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.2.0 (2023.x.0.YYYYMMDD)"}
!4 = !{ptr @plus}
!5 = !{i1 true}
!6 = !{i32 4, i32 1, i32 1}
!7 = !{i32 0}
!8 = !{!9, !9, i64 0}
!9 = !{!"omnipotent char", !10, i64 0}
!10 = !{!"Simple C/C++ TBAA"}
!11 = !{!12, !12, i64 0}
!12 = !{!"int", !9, i64 0}

; get_compute_id is resolved and the corresponding instructions are removed, we ignore the “Missing line xxxx” warning
; DEBUGIFY-NOT: WARNING
; DEBUGIFY: WARNING: Missing line 3
; DEBUGIFY: WARNING: Missing line 8
; DEBUGIFY-NOT: WARNING
