; Compiled from:
; ----------------------------------------------------
; #pragma OPENCL EXTENSION cl_intel_channels : enable

; channel int a;

; __attribute__((noinline))
; void helper(channel int a, int data) {
;   write_channel_intel(a, data);
; }

; __attribute__((noinline))
; void helper2(channel int a) {
;   helper(a, 10);
; }

; __attribute__((noinline))
; void helper3(channel int a) {
;   helper2(a);
; }

; __attribute__((noinline))
; void helper4(channel int a) {
;  helper(a, 7);
; }

; __kernel void test(__global int *data) {
;   helper(a, *data);
; }
; ----------------------------------------------------
; Compile options: -cc1 -emit-llvm -triple spir -finclude-default-header
; ----------------------------------------------------
; 
; RUN: %oclopt -runtimelib=%p/../../vectorizer/Full/runtime.bc -channel-pipe-transformation -verify %s -S | FileCheck --implicit-check-not helper2,helper3,helper4 %s
target datalayout = "e-p:32:32-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir"

%opencl.channel_t = type opaque

@a = common local_unnamed_addr addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4, !packet_size !0, !packet_align !0

; CHECK: define spir_func void @helper(%opencl.pipe_rw_t addrspace(1)* %0, i32 %1)

; All unused functions with channels should be erased
; check is done using --implicit-check-not

; Function Attrs: convergent noinline nounwind
define spir_func void @helper(%opencl.channel_t addrspace(1)* %a, i32 %data) local_unnamed_addr #0 {
entry:
  tail call void @_Z19write_channel_intel11ocl_channelii(%opencl.channel_t addrspace(1)* %a, i32 %data) #3
  ret void
}

; Function Attrs: convergent
declare void @_Z19write_channel_intel11ocl_channelii(%opencl.channel_t addrspace(1)*, i32) local_unnamed_addr #1

; Function Attrs: convergent noinline nounwind
define spir_func void @helper2(%opencl.channel_t addrspace(1)* %a) local_unnamed_addr #0 {
entry:
  tail call spir_func void @helper(%opencl.channel_t addrspace(1)* %a, i32 10) #4
  ret void
}

; Function Attrs: convergent noinline nounwind
define spir_func void @helper3(%opencl.channel_t addrspace(1)* %a) local_unnamed_addr #0 {
entry:
  tail call spir_func void @helper2(%opencl.channel_t addrspace(1)* %a) #4
  ret void
}

; Function Attrs: convergent noinline nounwind
define spir_func void @helper4(%opencl.channel_t addrspace(1)* %a) local_unnamed_addr #0 {
entry:
  tail call spir_func void @helper(%opencl.channel_t addrspace(1)* %a, i32 7) #4
  ret void
}

; Function Attrs: convergent nounwind
define spir_kernel void @test(i32 addrspace(1)* nocapture readonly %data) local_unnamed_addr #2 !kernel_arg_addr_space !6 !kernel_arg_access_qual !7 !kernel_arg_type !8 !kernel_arg_base_type !8 !kernel_arg_type_qual !9 !kernel_arg_host_accessible !10 !kernel_arg_pipe_depth !11 !kernel_arg_pipe_io !9 !kernel_arg_buffer_location !9 {
entry:
  %0 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @a, align 4, !tbaa !12
  %1 = load i32, i32 addrspace(1)* %data, align 4, !tbaa !15
  tail call spir_func void @helper(%opencl.channel_t addrspace(1)* %0, i32 %1) #4
  ret void
}

attributes #0 = { convergent noinline nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { convergent "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "uniform-work-group-size"="true" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { convergent nounwind }
attributes #4 = { convergent }

!llvm.module.flags = !{!1}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!2}
!opencl.spir.version = !{!3}
!opencl.used.extensions = !{!4}
!opencl.used.optional.core.features = !{!4}
!opencl.compiler.options = !{!4}
!llvm.ident = !{!5}

!0 = !{i32 4}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 1, i32 0}
!3 = !{i32 1, i32 2}
!4 = !{}
!5 = !{!"clang version 7.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 6007a41cf54d413d8e40cbe83c6ff5863e32c924) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 4632cfc3a6485ad54f59794d3f3dad979df36321)"}
!6 = !{i32 1}
!7 = !{!"none"}
!8 = !{!"int*"}
!9 = !{!""}
!10 = !{i1 false}
!11 = !{i32 0}
!12 = !{!13, !13, i64 0}
!13 = !{!"omnipotent char", !14, i64 0}
!14 = !{!"Simple C/C++ TBAA"}
!15 = !{!16, !16, i64 0}
!16 = !{!"int", !13, i64 0}
