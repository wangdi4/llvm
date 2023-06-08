; Compiled from:
; ----------------------------------------------------
; #pragma OPENCL EXTENSION cl_intel_channels: enable
;
; channel int ch[4];
;
; __attribute__((autorun))
; __attribute__((max_global_work_dim(0)))
; __attribute__((num_compute_units(2,2,2)))
; __kernel void plus() {
;   if (get_compute_id(2) == 0) {
;     int i = read_channel_intel(ch[0]);
;     write_channel_intel(ch[1], i + 1);
;   } else if (get_compute_id(1) == 0 || get_compute_id(0) == 0) {
;     int j = read_channel_intel(ch[1]);
;     write_channel_intel(ch[2], j + 2);
;   } else {
;     int k = read_channel_intel(ch[2]);
;     write_channel_intel(ch[3], k + 3);
;   }
; }
; ----------------------------------------------------
; Clang options: -cc1 -emit-llvm -triple spir64-unknown-unknown-intelfpga -disable-llvm-passes -x cl -cl-std=CL2.0
; ----------------------------------------------------
; RUN: opt -passes=sycl-kernel-autorun-replicator %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-autorun-replicator %s -S | FileCheck %s --implicit-check-not get_compute_id
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64-unknown-unknown-intelfpga"

@ch = addrspace(1) global [4 x target("spirv.Channel")] zeroinitializer, align 4, !packet_size !0, !packet_align !0

; CHECK: define {{.*}}@plus
; CHECK: %cmp = icmp eq i64 0, 0
; CHECK: %cmp1 = icmp eq i64 0, 0
; CHECK: %cmp2 = icmp eq i64 0, 0

; CHECK: define {{.*}}@plus.1
; CHECK: %cmp = icmp eq i64 1, 0
; CHECK: %cmp1 = icmp eq i64 0, 0
; CHECK: %cmp2 = icmp eq i64 0, 0

; CHECK: define {{.*}}@plus.2
; CHECK: %cmp = icmp eq i64 0, 0
; CHECK: %cmp1 = icmp eq i64 1, 0
; CHECK: %cmp2 = icmp eq i64 0, 0

; CHECK: define {{.*}}@plus.3
; CHECK: %cmp = icmp eq i64 1, 0
; CHECK: %cmp1 = icmp eq i64 1, 0
; CHECK: %cmp2 = icmp eq i64 0, 0

; CHECK: define {{.*}}@plus.4
; CHECK: %cmp = icmp eq i64 0, 0
; CHECK: %cmp1 = icmp eq i64 0, 0
; CHECK: %cmp2 = icmp eq i64 1, 0

; CHECK: define {{.*}}@plus.5
; CHECK: %cmp = icmp eq i64 1, 0
; CHECK: %cmp1 = icmp eq i64 0, 0
; CHECK: %cmp2 = icmp eq i64 1, 0

; CHECK: define {{.*}}@plus.6
; CHECK: %cmp = icmp eq i64 0, 0
; CHECK: %cmp1 = icmp eq i64 1, 0
; CHECK: %cmp2 = icmp eq i64 1, 0

; CHECK: define {{.*}}@plus.7
; CHECK: %cmp = icmp eq i64 1, 0
; CHECK: %cmp1 = icmp eq i64 1, 0
; CHECK: %cmp2 = icmp eq i64 1, 0

; CHECK-NOT: define {{.*}}@plus

; Function Attrs: convergent norecurse nounwind
define dso_local void @plus() #0 !kernel_arg_addr_space !2 !kernel_arg_access_qual !2 !kernel_arg_type !2 !kernel_arg_base_type !2 !kernel_arg_type_qual !2 !kernel_arg_host_accessible !2 !kernel_arg_pipe_depth !2 !kernel_arg_pipe_io !2 !kernel_arg_buffer_location !2 !autorun !5 !num_compute_units !6 !max_global_work_dim !7 !arg_type_null_val !2 {
entry:
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  %k = alloca i32, align 4
  %0 = call i64 @get_compute_id(i32 2)
  %cmp = icmp eq i64 %0, 0
  br i1 %cmp, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  call void @llvm.lifetime.start.p0(i64 4, ptr %i) #3
  %1 = load ptr addrspace(1), ptr addrspace(1) @ch, align 4, !tbaa !8
  %call = call i32 @_Z18read_channel_intel11ocl_channeli(ptr addrspace(1) %1) #4
  store i32 %call, ptr %i, align 4, !tbaa !11
  %2 = load ptr addrspace(1), ptr addrspace(1) getelementptr inbounds ([4 x target("spirv.Channel")], ptr addrspace(1) @ch, i64 0, i64 1), align 4, !tbaa !8
  %3 = load i32, ptr %i, align 4, !tbaa !11
  %add = add nsw i32 %3, 1
  call void @_Z19write_channel_intel11ocl_channelii(ptr addrspace(1) %2, i32 noundef %add) #4
  call void @llvm.lifetime.end.p0(i64 4, ptr %i) #3
  br label %if.end9

if.else:                                          ; preds = %entry
  %4 = call i64 @get_compute_id(i32 1)
  %cmp1 = icmp eq i64 %4, 0
  br i1 %cmp1, label %if.then3, label %lor.lhs.false

lor.lhs.false:                                    ; preds = %if.else
  %5 = call i64 @get_compute_id(i32 0)
  %cmp2 = icmp eq i64 %5, 0
  br i1 %cmp2, label %if.then3, label %if.else6

if.then3:                                         ; preds = %lor.lhs.false, %if.else
  call void @llvm.lifetime.start.p0(i64 4, ptr %j) #3
  %6 = load ptr addrspace(1), ptr addrspace(1) getelementptr inbounds ([4 x target("spirv.Channel")], ptr addrspace(1) @ch, i64 0, i64 1), align 4, !tbaa !8
  %call4 = call i32 @_Z18read_channel_intel11ocl_channeli(ptr addrspace(1) %6) #4
  store i32 %call4, ptr %j, align 4, !tbaa !11
  %7 = load ptr addrspace(1), ptr addrspace(1) getelementptr inbounds ([4 x target("spirv.Channel")], ptr addrspace(1) @ch, i64 0, i64 2), align 4, !tbaa !8
  %8 = load i32, ptr %j, align 4, !tbaa !11
  %add5 = add nsw i32 %8, 2
  call void @_Z19write_channel_intel11ocl_channelii(ptr addrspace(1) %7, i32 noundef %add5) #4
  call void @llvm.lifetime.end.p0(i64 4, ptr %j) #3
  br label %if.end

if.else6:                                         ; preds = %lor.lhs.false
  call void @llvm.lifetime.start.p0(i64 4, ptr %k) #3
  %9 = load ptr addrspace(1), ptr addrspace(1) getelementptr inbounds ([4 x target("spirv.Channel")], ptr addrspace(1) @ch, i64 0, i64 2), align 4, !tbaa !8
  %call7 = call i32 @_Z18read_channel_intel11ocl_channeli(ptr addrspace(1) %9) #4
  store i32 %call7, ptr %k, align 4, !tbaa !11
  %10 = load ptr addrspace(1), ptr addrspace(1) getelementptr inbounds ([4 x target("spirv.Channel")], ptr addrspace(1) @ch, i64 0, i64 3), align 4, !tbaa !8
  %11 = load i32, ptr %k, align 4, !tbaa !11
  %add8 = add nsw i32 %11, 3
  call void @_Z19write_channel_intel11ocl_channelii(ptr addrspace(1) %10, i32 noundef %add8) #4
  call void @llvm.lifetime.end.p0(i64 4, ptr %k) #3
  br label %if.end

if.end:                                           ; preds = %if.else6, %if.then3
  br label %if.end9

if.end9:                                          ; preds = %if.end, %if.then
  ret void
}

declare i64 @get_compute_id(i32)

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

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
!6 = !{i32 2, i32 2, i32 2}
!7 = !{i32 0}
!8 = !{!9, !9, i64 0}
!9 = !{!"omnipotent char", !10, i64 0}
!10 = !{!"Simple C/C++ TBAA"}
!11 = !{!12, !12, i64 0}
!12 = !{!"int", !9, i64 0}

; get_compute_id is resolved and the corresponding instructions are removed, we ignore the “Missing line xxxx” warning
; DEBUGIFY-NOT: WARNING
; DEBUGIFY: WARNING: Missing line 4
; DEBUGIFY: WARNING: Missing line 17
; DEBUGIFY: WARNING: Missing line 20
; DEBUGIFY-NOT: WARNING
