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
; Opt passes: -spir-materializer
; ----------------------------------------------------
; RUN: %oclopt -runtimelib=%p/../../vectorizer/Full/runtime.bc -autorun-replicator -verify %s -S | FileCheck %s
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown-intelfpga"

%opencl.channel_t = type opaque

@ch = common addrspace(1) global [5 x %opencl.channel_t addrspace(1)*] zeroinitializer, align 4

; CHECK: define void @plus() #0 [[ATTRIBUTES:.*]] {
; CHECK: define void @plus.[[R1:[0-9]+]]() {{.*}} [[ATTRIBUTES]] {
; CHECK: define void @plus.[[R2:[0-9]+]]() {{.*}} [[ATTRIBUTES]] {
; CHECK: define void @plus.[[R3:[0-9]+]]() {{.*}} [[ATTRIBUTES]] {
; CHECK: !opencl.kernels = !{![[K:[0-9]+]]}
; CHECK: ![[K]] = !{{{.*}}@plus, {{.*}}@plus.[[R1]], {{.*}}@plus.[[R2]], {{.*}}@plus.[[R3]]}

; Function Attrs: nounwind
define void @plus() #0 !kernel_arg_addr_space !5 !kernel_arg_access_qual !5 !kernel_arg_type !5 !kernel_arg_base_type !5 !kernel_arg_type_qual !5 !task !8 !autorun !8 !num_compute_units !9 {
entry:
  %a = alloca i32, align 4
  %0 = bitcast i32* %a to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #3
  %1 = call i64 @get_compute_id(i32 0)
  %arrayidx = getelementptr inbounds [5 x %opencl.channel_t addrspace(1)*], [5 x %opencl.channel_t addrspace(1)*] addrspace(1)* @ch, i64 0, i64 %1
  %2 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* %arrayidx, align 4, !tbaa !10
  %call = call i32 @_Z18read_channel_intel11ocl_channeli(%opencl.channel_t addrspace(1)* %2)
  store i32 %call, i32* %a, align 4, !tbaa !13
  %3 = call i64 @get_compute_id(i32 0)
  %add = add i64 %3, 1
  %arrayidx1 = getelementptr inbounds [5 x %opencl.channel_t addrspace(1)*], [5 x %opencl.channel_t addrspace(1)*] addrspace(1)* @ch, i64 0, i64 %add
  %4 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* %arrayidx1, align 4, !tbaa !10
  %5 = load i32, i32* %a, align 4, !tbaa !13
  %add2 = add nsw i32 %5, 1
  call void @_Z19write_channel_intel11ocl_channelii(%opencl.channel_t addrspace(1)* %4, i32 %add2)
  %6 = bitcast i32* %a to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %6) #3
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

declare i32 @_Z18read_channel_intel11ocl_channeli(%opencl.channel_t addrspace(1)*) #2

declare i64 @get_compute_id(i32)

declare void @_Z19write_channel_intel11ocl_channelii(%opencl.channel_t addrspace(1)*, i32) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }

!opencl.channels = !{!0}
!llvm.module.flags = !{!3}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!4}
!opencl.spir.version = !{!4}
!opencl.used.extensions = !{!5}
!opencl.used.optional.core.features = !{!5}
!opencl.compiler.options = !{!5}
!llvm.ident = !{!6}
!opencl.kernels = !{!7}

!0 = !{[5 x %opencl.channel_t addrspace(1)*] addrspace(1)* @ch, !1, !2}
!1 = !{!"packet_size", i32 4}
!2 = !{!"packet_align", i32 4}
!3 = !{i32 1, !"wchar_size", i32 4}
!4 = !{i32 2, i32 0}
!5 = !{}
!6 = !{!"clang version 5.0.0 "}
!7 = !{void ()* @plus}
!8 = !{i1 true}
!9 = !{i32 4, i32 1, i32 1}
!10 = !{!11, !11, i64 0}
!11 = !{!"omnipotent char", !12, i64 0}
!12 = !{!"Simple C/C++ TBAA"}
!13 = !{!14, !14, i64 0}
!14 = !{!"int", !11, i64 0}
