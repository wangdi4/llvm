; RUN: opt -passes=sycl-kernel-vec-clone -sycl-vector-variant-isa-encoding-override=AVX512Core -sycl-vect-info=%p/../Inputs/VectInfo64.gen %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-vec-clone -sycl-vector-variant-isa-encoding-override=AVX512Core -sycl-vect-info=%p/../Inputs/VectInfo64.gen %s -S -o - | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; CHECK-LABEL: @_ZGVeN16u_test
; Function Attrs: convergent norecurse nounwind
define void @test(ptr addrspace(1) noalias %img) local_unnamed_addr #0 !kernel_arg_addr_space !6 !kernel_arg_access_qual !7 !kernel_arg_type !8 !kernel_arg_base_type !8 !kernel_arg_type_qual !9 !kernel_arg_host_accessible !10 !kernel_arg_pipe_depth !11 !kernel_arg_pipe_io !9 !kernel_arg_buffer_location !9 !kernel_arg_name !12 !no_barrier_path !10 !kernel_has_sub_groups !13 !recommended_vector_length !14 !arg_type_null_val !15 {
entry:
  %call = tail call signext i16 @_Z23intel_sub_group_shufflesj(i16 signext 1, i32 0) #3
; CHECK: call signext i16 @_Z23intel_sub_group_shufflesj(i16 signext 1, i32 0) #[[SHUFFLE_ATTR:[0-9]+]]
  %call1 = tail call zeroext i16 @_Z29intel_sub_group_block_read_us14ocl_image2d_roDv2_i(ptr addrspace(1) %img, <2 x i32> zeroinitializer) #3
; CHECK: call zeroext i16 @_Z29intel_sub_group_block_read_us14ocl_image2d_roDv2_i(ptr addrspace(1) {{%.*}}, <2 x i32> zeroinitializer) #[[BLOCK_READ_ATTR:[0-9]+]]
  %call2 = tail call double @_Z21work_group_reduce_addd(double 1.000000e+00) #4;
; CHECK: call double @_Z21work_group_reduce_addd(double 1.000000e+00) #[[WG_REDUCE_ATTR:[0-9]+]]
  ret void
}

; Function Attrs: convergent
declare signext i16 @_Z23intel_sub_group_shufflesj(i16 signext, i32) local_unnamed_addr #1

; Function Attrs: convergent
declare zeroext i16 @_Z29intel_sub_group_block_read_us14ocl_image2d_roDv2_i(ptr addrspace(1), <2 x i32>) local_unnamed_addr #1

; Function Attrs: convergent
declare double @_Z21work_group_reduce_addd(double) local_unnamed_addr #2

attributes #0 = { convergent norecurse nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "kernel-call-once" "kernel-convergent-call" "less-precise-fpmad"="false" "min-legal-vector-width"="64" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { convergent "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { convergent "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "kernel-call-once" "kernel-convergent-call" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { convergent nounwind }
attributes #4 = { convergent nounwind "kernel-call-once" "kernel-convergent-call" }

; CHECK: attributes #[[SHUFFLE_ATTR]] = { {{.*}} "has-vplan-mask" {{.*}} }
; CHECK: attributes #[[BLOCK_READ_ATTR]] = { {{.*}} "has-vplan-mask" {{.*}} }
; CHECK: attributes #[[WG_REDUCE_ATTR]] = { {{.*}} "call-params-num"="1" {{.*}} }

!llvm.linker.options = !{}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!0}
!opencl.spir.version = !{!0}
!opencl.used.extensions = !{!1}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!3}
!llvm.ident = !{!4}
!sycl.kernels = !{!5}

!0 = !{i32 2, i32 0}
!1 = !{}
!2 = !{!"cl_doubles", !"cl_images"}
!3 = !{!"-cl-std=CL2.0"}
!4 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!5 = !{ptr @test}
!6 = !{i32 1}
!7 = !{!"read_only"}
!8 = !{!"image2d_t"}
!9 = !{!""}
!10 = !{i1 false}
!11 = !{i32 0}
!12 = !{!"img"}
!13 = !{i1 true}
!14 = !{i32 16}
!15 = !{target("spirv.Image", void, 1, 0, 0, 0, 0, 0, 0) zeroinitializer}

; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZGVeN16u_test {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16u_test {{.*}} call
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16u_test {{.*}} add
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16u_test {{.*}} icmp
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16u_test {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16u_test {{.*}} call
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN16u_test {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16u_test {{.*}} alloca
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16u_test {{.*}} store
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16u_test {{.*}} alloca
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16u_test {{.*}} store
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16u_test {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16u_test {{.*}} call
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16u_test {{.*}} getelementptr
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16u_test {{.*}} load
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16u_test {{.*}} icmp
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16u_test {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16u_test {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16u_test {{.*}} add
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16u_test {{.*}} icmp
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16u_test {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16u_test {{.*}} call
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeM16u_test {{.*}} br
; DEBUGIFY-NOT: WARNING
