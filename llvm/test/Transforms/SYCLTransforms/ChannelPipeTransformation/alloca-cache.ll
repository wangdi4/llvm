; Compiled from:
; ----------------------------------------------------
; #pragma OPENCL EXTENSION cl_intel_channels : enable
;
; channel int bar;
; channel int far;
;
; channel float a;
; channel float b;
;
; __kernel void foo(__global int *data) {
;   int i = read_channel_intel(bar);
;   bool v;
;   int j = read_channel_nb_intel(far, &v);
;
;   data[0] = i;
;   data[1] = j;
; }
;
; __kernel void foobar() {
;   float p = 3.14;
;   float e = 2.7;
;
;   write_channel_intel(a, p);
;   write_channel_nb_intel(b, e);
; }
; ----------------------------------------------------
; Compile options: -cc1 -emit-llvm -triple spir64-unknown-unknown-intelfpga -disable-llvm-passes -x cl -cl-std=CL2.0
; opt -passes=sycl-kernel-target-ext-type-lower,sycl-kernel-equalizer %s -S
; ----------------------------------------------------

; RUN: opt -passes=sycl-kernel-channel-pipe-transformation -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-channel-pipe-transformation -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl %s -S | FileCheck %s

@bar = addrspace(1) global ptr addrspace(1) null, align 4, !packet_size !0, !packet_align !0
@far = addrspace(1) global ptr addrspace(1) null, align 4, !packet_size !0, !packet_align !0
@a = addrspace(1) global ptr addrspace(1) null, align 4, !packet_size !0, !packet_align !0
@b = addrspace(1) global ptr addrspace(1) null, align 4, !packet_size !0, !packet_align !0

; CHECK: define {{.*}} @foo
; CHECK: %[[READ_DST_ALLOCA:read.dst.*]] = alloca i32
; CHECK: %[[BAR_PIPE_LOAD:.*]] = load {{.*}} @bar
; CHECK: %[[READ_DST:.*]] = addrspacecast {{.*}} %[[READ_DST_ALLOCA]]
; CHECK: call i32 @__read_pipe_2{{.*}}({{.*}} %[[BAR_PIPE_LOAD]], {{.*}} %[[READ_DST]]
; CHECK: load {{.*}} %[[READ_DST_ALLOCA]]
;
; CHECK: %[[FAR_PIPE_LOAD:.*]] = load {{.*}} @far
; CHECK: %[[READ_DST1:.*]] = addrspacecast {{.*}} %[[READ_DST_ALLOCA]]
; CHECK: call i32 @__read_pipe_2{{.*}}({{.*}} %[[FAR_PIPE_LOAD]], {{.*}} %[[READ_DST1]]
; CHECK: load {{.*}} %[[READ_DST_ALLOCA]]
;
; CHECK: define {{.*}} @foobar
; CHECK: %[[WRITE_SRC_ALLOCA:write.src.*]] = alloca float
; CHECK: %[[A_PIPE_LOAD:.*]] = load {{.*}} @a
; CHECK: store {{.*}} %[[WRITE_SRC_ALLOCA]]
; CHECK: %[[WRITE_SRC:.*]] = addrspacecast {{.*}} %[[WRITE_SRC_ALLOCA]]
; CHECK: call i32 @__write_pipe_2{{.*}}({{.*}} %[[A_PIPE_LOAD]], {{.*}} %[[WRITE_SRC]]
;
; CHECK: %[[B_PIPE_LOAD:.*]] = load {{.*}} @b
; CHECK: store {{.*}} %[[WRITE_SRC_ALLOCA]]
; CHECK: %[[WRITE_SRC1:.*]] = addrspacecast {{.*}} %[[WRITE_SRC_ALLOCA]]
; CHECK: call i32 @__write_pipe_2{{.*}}({{.*}} %[[B_PIPE_LOAD]], {{.*}} %[[WRITE_SRC1]]

; Function Attrs: convergent norecurse nounwind
define dso_local void @foo(ptr addrspace(1) noundef align 4 %data) #0 !kernel_arg_addr_space !5 !kernel_arg_access_qual !6 !kernel_arg_type !7 !kernel_arg_base_type !7 !kernel_arg_type_qual !8 !kernel_arg_host_accessible !9 !kernel_arg_pipe_depth !10 !kernel_arg_pipe_io !8 !kernel_arg_buffer_location !8 !arg_type_null_val !11 {
entry:
  %data.addr = alloca ptr addrspace(1), align 8
  %i = alloca i32, align 4
  %v = alloca i8, align 1
  %j = alloca i32, align 4
  store ptr addrspace(1) %data, ptr %data.addr, align 8, !tbaa !12
  call void @llvm.lifetime.start.p0(i64 4, ptr %i) #3
  %0 = load ptr addrspace(1), ptr addrspace(1) @bar, align 4, !tbaa !16
  %call = call i32 @_Z18read_channel_intel11ocl_channeli(ptr addrspace(1) %0) #4
  store i32 %call, ptr %i, align 4, !tbaa !17
  call void @llvm.lifetime.start.p0(i64 1, ptr %v) #3
  call void @llvm.lifetime.start.p0(i64 4, ptr %j) #3
  %1 = load ptr addrspace(1), ptr addrspace(1) @far, align 4, !tbaa !16
  %v.ascast = addrspacecast ptr %v to ptr addrspace(4)
  %call1 = call i32 @_Z21read_channel_nb_intel11ocl_channeliPU3AS4b(ptr addrspace(1) %1, ptr addrspace(4) noundef %v.ascast) #4
  store i32 %call1, ptr %j, align 4, !tbaa !17
  %2 = load i32, ptr %i, align 4, !tbaa !17
  %3 = load ptr addrspace(1), ptr %data.addr, align 8, !tbaa !12
  %arrayidx = getelementptr inbounds i32, ptr addrspace(1) %3, i64 0
  store i32 %2, ptr addrspace(1) %arrayidx, align 4, !tbaa !17
  %4 = load i32, ptr %j, align 4, !tbaa !17
  %5 = load ptr addrspace(1), ptr %data.addr, align 8, !tbaa !12
  %arrayidx2 = getelementptr inbounds i32, ptr addrspace(1) %5, i64 1
  store i32 %4, ptr addrspace(1) %arrayidx2, align 4, !tbaa !17
  call void @llvm.lifetime.end.p0(i64 4, ptr %j) #3
  call void @llvm.lifetime.end.p0(i64 1, ptr %v) #3
  call void @llvm.lifetime.end.p0(i64 4, ptr %i) #3
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: convergent norecurse nounwind
define dso_local void @foobar() #0 !kernel_arg_addr_space !2 !kernel_arg_access_qual !2 !kernel_arg_type !2 !kernel_arg_base_type !2 !kernel_arg_type_qual !2 !kernel_arg_host_accessible !2 !kernel_arg_pipe_depth !2 !kernel_arg_pipe_io !2 !kernel_arg_buffer_location !2 !arg_type_null_val !2 {
entry:
  %p = alloca float, align 4
  %e = alloca float, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr %p) #3
  store float 0x40091EB860000000, ptr %p, align 4, !tbaa !19
  call void @llvm.lifetime.start.p0(i64 4, ptr %e) #3
  store float 0x40059999A0000000, ptr %e, align 4, !tbaa !19
  %0 = load ptr addrspace(1), ptr addrspace(1) @a, align 4, !tbaa !16
  %1 = load float, ptr %p, align 4, !tbaa !19
  call void @_Z19write_channel_intel11ocl_channelff(ptr addrspace(1) %0, float noundef %1) #4
  %2 = load ptr addrspace(1), ptr addrspace(1) @b, align 4, !tbaa !16
  %3 = load float, ptr %e, align 4, !tbaa !19
  %call = call zeroext i1 @_Z22write_channel_nb_intel11ocl_channelff(ptr addrspace(1) %2, float noundef %3) #4
  call void @llvm.lifetime.end.p0(i64 4, ptr %e) #3
  call void @llvm.lifetime.end.p0(i64 4, ptr %p) #3
  ret void
}

; Function Attrs: convergent nounwind
declare i32 @_Z18read_channel_intel11ocl_channeli(ptr addrspace(1)) #2

; Function Attrs: convergent nounwind
declare i32 @_Z21read_channel_nb_intel11ocl_channeliPU3AS4b(ptr addrspace(1), ptr addrspace(4) noundef) #2

; Function Attrs: convergent nounwind
declare void @_Z19write_channel_intel11ocl_channelff(ptr addrspace(1), float noundef) #2

; Function Attrs: convergent nounwind
declare zeroext i1 @_Z22write_channel_nb_intel11ocl_channelff(ptr addrspace(1), float noundef) #2

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
!3 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2024.0.0 (2024.x.0.YYYYMMDD)"}
!4 = !{ptr @foo, ptr @foobar}
!5 = !{i32 1}
!6 = !{!"none"}
!7 = !{!"int*"}
!8 = !{!""}
!9 = !{i1 false}
!10 = !{i32 0}
!11 = !{ptr addrspace(1) null}
!12 = !{!13, !13, i64 0}
!13 = !{!"any pointer", !14, i64 0}
!14 = !{!"omnipotent char", !15, i64 0}
!15 = !{!"Simple C/C++ TBAA"}
!16 = !{!14, !14, i64 0}
!17 = !{!18, !18, i64 0}
!18 = !{!"int", !14, i64 0}
!19 = !{!20, !20, i64 0}
!20 = !{!"float", !14, i64 0}

; DEBUGIFY-NOT: WARNING: Missing line
