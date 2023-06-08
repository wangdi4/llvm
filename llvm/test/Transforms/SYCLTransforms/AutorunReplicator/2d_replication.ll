; Compiled from:
; ----------------------------------------------------
; #pragma OPENCL EXTENSION cl_intel_channels: enable
;
; channel int ch[4][2];
;
; __attribute__((autorun))
; __attribute__((max_global_work_dim(0)))
; __attribute__((num_compute_units(2,2,1)))
; __kernel void plus() {
;   unsigned int x = get_compute_id(0);
;   unsigned int y = get_compute_id(1);
;   unsigned int chan_index = 2 * x + y;
;   int a = read_channel_intel(ch[chan_index][0]);
;   write_channel_intel(ch[chan_index][1], a + 1);
; }
; ----------------------------------------------------
; Clang options: -cc1 -emit-llvm -triple spir64-unknown-unknown-intelfpga -disable-llvm-passes -x cl -cl-std=CL2.0
; ----------------------------------------------------
; RUN: opt -passes=sycl-kernel-autorun-replicator %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-autorun-replicator %s -S | FileCheck %s --implicit-check-not get_compute_id
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64-unknown-unknown-intelfpga"

@ch = addrspace(1) global [4 x [2 x target("spirv.Channel")]] zeroinitializer, align 4, !packet_size !0, !packet_align !0

; CHECK: define {{.*}}@plus
; CHECK: %conv = trunc i64 0 to i32
; CHECK-NEXT: store i32 %conv, ptr %x
; CHECK: %conv1 = trunc i64 0 to i32
; CHECK-NEXT: store i32 %conv1, ptr %y

; CHECK: define {{.*}}@plus.1
; CHECK: %conv = trunc i64 0 to i32
; CHECK-NEXT: store i32 %conv, ptr %x
; CHECK: %conv1 = trunc i64 1 to i32
; CHECK-NEXT: store i32 %conv1, ptr %y

; CHECK: define {{.*}}@plus.2
; CHECK: %conv = trunc i64 1 to i32
; CHECK-NEXT: store i32 %conv, ptr %x
; CHECK: %conv1 = trunc i64 0 to i32
; CHECK-NEXT: store i32 %conv1, ptr %y

; CHECK: define {{.*}}@plus.3
; CHECK: %conv = trunc i64 1 to i32
; CHECK-NEXT: store i32 %conv, ptr %x
; CHECK: %conv1 = trunc i64 1 to i32
; CHECK-NEXT: store i32 %conv1, ptr %y

; CHECK-NOT: define {{.*}}@plus

; Function Attrs: convergent norecurse nounwind
define dso_local void @plus() #0 !kernel_arg_addr_space !2 !kernel_arg_access_qual !2 !kernel_arg_type !2 !kernel_arg_base_type !2 !kernel_arg_type_qual !2 !kernel_arg_host_accessible !2 !kernel_arg_pipe_depth !2 !kernel_arg_pipe_io !2 !kernel_arg_buffer_location !2 !autorun !5 !num_compute_units !6 !max_global_work_dim !7 !arg_type_null_val !2 {
entry:
  %x = alloca i32, align 4
  %y = alloca i32, align 4
  %chan_index = alloca i32, align 4
  %a = alloca i32, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr %x) #3
  %0 = call i64 @get_compute_id(i32 0)
  %conv = trunc i64 %0 to i32
  store i32 %conv, ptr %x, align 4, !tbaa !8
  call void @llvm.lifetime.start.p0(i64 4, ptr %y) #3
  %1 = call i64 @get_compute_id(i32 1)
  %conv1 = trunc i64 %1 to i32
  store i32 %conv1, ptr %y, align 4, !tbaa !8
  call void @llvm.lifetime.start.p0(i64 4, ptr %chan_index) #3
  %2 = load i32, ptr %x, align 4, !tbaa !8
  %mul = mul i32 2, %2
  %3 = load i32, ptr %y, align 4, !tbaa !8
  %add = add i32 %mul, %3
  store i32 %add, ptr %chan_index, align 4, !tbaa !8
  call void @llvm.lifetime.start.p0(i64 4, ptr %a) #3
  %4 = load i32, ptr %chan_index, align 4, !tbaa !8
  %idxprom = zext i32 %4 to i64
  %arrayidx = getelementptr inbounds [4 x [2 x target("spirv.Channel")]], ptr addrspace(1) @ch, i64 0, i64 %idxprom
  %arrayidx2 = getelementptr inbounds [2 x target("spirv.Channel")], ptr addrspace(1) %arrayidx, i64 0, i64 0
  %5 = load ptr addrspace(1), ptr addrspace(1) %arrayidx2, align 4, !tbaa !12
  %call = call i32 @_Z18read_channel_intel11ocl_channeli(ptr addrspace(1) %5) #4
  store i32 %call, ptr %a, align 4, !tbaa !8
  %6 = load i32, ptr %chan_index, align 4, !tbaa !8
  %idxprom3 = zext i32 %6 to i64
  %arrayidx4 = getelementptr inbounds [4 x [2 x target("spirv.Channel")]], ptr addrspace(1) @ch, i64 0, i64 %idxprom3
  %arrayidx5 = getelementptr inbounds [2 x target("spirv.Channel")], ptr addrspace(1) %arrayidx4, i64 0, i64 1
  %7 = load ptr addrspace(1), ptr addrspace(1) %arrayidx5, align 4, !tbaa !12
  %8 = load i32, ptr %a, align 4, !tbaa !8
  %add6 = add nsw i32 %8, 1
  call void @_Z19write_channel_intel11ocl_channelii(ptr addrspace(1) %7, i32 noundef %add6) #4
  call void @llvm.lifetime.end.p0(i64 4, ptr %a) #3
  call void @llvm.lifetime.end.p0(i64 4, ptr %chan_index) #3
  call void @llvm.lifetime.end.p0(i64 4, ptr %y) #3
  call void @llvm.lifetime.end.p0(i64 4, ptr %x) #3
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
!6 = !{i32 2, i32 2, i32 1}
!7 = !{i32 0}
!8 = !{!9, !9, i64 0}
!9 = !{!"int", !10, i64 0}
!10 = !{!"omnipotent char", !11, i64 0}
!11 = !{!"Simple C/C++ TBAA"}
!12 = !{!10, !10, i64 0}

; get_compute_id is resolved and the corresponding instructions are removed, we ignore the “Missing line xxxx” warning
; DEBUGIFY-NOT: WARNING
; DEBUGIFY: WARNING: Missing line 6
; DEBUGIFY: WARNING: Missing line 10
; DEBUGIFY-NOT: WARNING
