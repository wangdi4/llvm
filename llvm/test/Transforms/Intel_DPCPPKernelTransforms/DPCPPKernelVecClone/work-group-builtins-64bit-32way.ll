; RUN: opt -dpcpp-kernel-vec-clone -dpcpp-vector-variant-isa-encoding-override=AVX512Core -dpcpp-vect-info=%p/../Inputs/VectInfo64.gen %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=dpcpp-kernel-vec-clone -dpcpp-vector-variant-isa-encoding-override=AVX512Core -dpcpp-vect-info=%p/../Inputs/VectInfo64.gen %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -dpcpp-kernel-vec-clone -vplan-vec -dpcpp-vector-variant-isa-encoding-override=AVX512Core -dpcpp-vect-info=%p/../Inputs/VectInfo64.gen %s -S -o - | FileCheck %s
; RUN: opt -passes=dpcpp-kernel-vec-clone,vplan-vec -dpcpp-vector-variant-isa-encoding-override=AVX512Core -dpcpp-vect-info=%p/../Inputs/VectInfo64.gen %s -S -o - | FileCheck %s

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: convergent nounwind
define spir_kernel void @a(i32 addrspace(1)* nocapture readonly %a, i32 addrspace(1)* nocapture %b) local_unnamed_addr #0 !kernel_arg_addr_space !4 !kernel_arg_access_qual !5 !kernel_arg_type !6 !kernel_arg_base_type !6 !kernel_arg_type_qual !7 !kernel_arg_host_accessible !8 !kernel_arg_pipe_depth !9 !kernel_arg_pipe_io !7 !kernel_arg_buffer_location !7 !recommended_vector_length !15 {
entry:
  %call = tail call spir_func i64 @_Z13get_global_idj(i32 0) #3
  %arrayidx = getelementptr inbounds i32, i32 addrspace(1)* %a, i64 %call
  %0 = load i32, i32 addrspace(1)* %arrayidx, align 4, !tbaa !10
  %1 = zext i32 %0 to i64
; CHECK: [[WIDE_LOAD_i64:%.*]] = zext <32 x i32> %wide.load to <32 x i64>

  %call1 = tail call spir_func i32 @_Z14work_group_alli(i32 %0) #4
; CHECK: [[VECTOR_ALL:%.*]] = call {{(spir_func )?}}<32 x i32> @_Z14work_group_allDv32_i(<32 x i32> %wide.load)
  %call2 = tail call spir_func i32 @_Z14work_group_anyi(i32 %0) #4
; CHECK: [[VECTOR_ANY:%.*]] = call {{(spir_func )?}}<32 x i32> @_Z14work_group_anyDv32_i(<32 x i32> %wide.load)

  %call3 = tail call spir_func i32 @_Z20work_group_broadcastim(i32 %0, i64 0) #4
; CHECK: = call {{(spir_func )?}}<32 x i32> @_Z20work_group_broadcastDv32_im(<32 x i32> %wide.load, i64 0)
  %call4 = tail call spir_func i32 @_Z20work_group_broadcastimm(i32 %0, i64 0, i64 0) #4
; CHECK: = call {{(spir_func )?}}<32 x i32> @_Z20work_group_broadcastDv32_imm(<32 x i32> %wide.load, i64 0, i64 0)
  %call5 = tail call spir_func i32 @_Z20work_group_broadcastimmm(i32 %0, i64 0, i64 0, i64 0) #4
; CHECK: = call {{(spir_func )?}}<32 x i32> @_Z20work_group_broadcastDv32_immm(<32 x i32> %wide.load, i64 0, i64 0, i64 0)

  %call6 = tail call spir_func i32 @_Z21work_group_reduce_addi(i32 %0) #4
  %call7 = tail call spir_func i32 @_Z21work_group_reduce_minj(i32 %0) #4
  %call8 = tail call spir_func i64 @_Z21work_group_reduce_maxl(i64 %1) #4
; CHECK: = call {{(spir_func )?}}<32 x i32> @_Z21work_group_reduce_addDv32_i(<32 x i32> %wide.load)
; CHECK: = call {{(spir_func )?}}<32 x i32> @_Z21work_group_reduce_minDv32_j(<32 x i32> %wide.load)
; CHECK: = call {{(spir_func )?}}<32 x i64> @_Z21work_group_reduce_maxDv32_l(<32 x i64> [[WIDE_LOAD_i64]])

  %call9  = tail call spir_func i32 @_Z29work_group_scan_exclusive_addi(i32 %0) #4
  %call10 = tail call spir_func i32 @_Z29work_group_scan_exclusive_minj(i32 %0) #4
  %call11 = tail call spir_func i64 @_Z29work_group_scan_exclusive_maxl(i64 %1) #4
; CHECK: = call {{(spir_func )?}}<32 x i32> @_Z29work_group_scan_exclusive_addDv32_i(<32 x i32> %wide.load)
; CHECK: = call {{(spir_func )?}}<32 x i32> @_Z29work_group_scan_exclusive_minDv32_j(<32 x i32> %wide.load)
; CHECK: = call {{(spir_func )?}}<32 x i64> @_Z29work_group_scan_exclusive_maxDv32_l(<32 x i64> [[WIDE_LOAD_i64]])

  %call12 = tail call spir_func i32 @_Z29work_group_scan_inclusive_addi(i32 %0) #4
  %call13 = tail call spir_func i32 @_Z29work_group_scan_inclusive_minj(i32 %0) #4
  %call14 = tail call spir_func i64 @_Z29work_group_scan_inclusive_maxl(i64 %1) #4
; CHECK: = call {{(spir_func )?}}<32 x i32> @_Z29work_group_scan_inclusive_addDv32_i(<32 x i32> %wide.load)
; CHECK: = call {{(spir_func )?}}<32 x i32> @_Z29work_group_scan_inclusive_minDv32_j(<32 x i32> %wide.load)
; CHECK: = call {{(spir_func )?}}<32 x i64> @_Z29work_group_scan_inclusive_maxDv32_l(<32 x i64> [[WIDE_LOAD_i64]])

  ret void
}

; Function Attrs: convergent
declare spir_func i32 @_Z14work_group_alli(i32) local_unnamed_addr #1
declare spir_func i32 @_Z14work_group_anyi(i32) local_unnamed_addr #1
declare spir_func i32 @_Z20work_group_broadcastim(i32, i64) local_unnamed_addr #1
declare spir_func i32 @_Z20work_group_broadcastimm(i32, i64, i64) local_unnamed_addr #1
declare spir_func i32 @_Z20work_group_broadcastimmm(i32, i64, i64, i64) local_unnamed_addr #1
declare spir_func i32 @_Z21work_group_reduce_addi(i32) #1
declare spir_func i32 @_Z21work_group_reduce_minj(i32) #1
declare spir_func i64 @_Z21work_group_reduce_maxl(i64) #1
declare spir_func i32 @_Z29work_group_scan_exclusive_addi(i32) #1
declare spir_func i32 @_Z29work_group_scan_exclusive_minj(i32) #1
declare spir_func i64 @_Z29work_group_scan_exclusive_maxl(i64) #1
declare spir_func i32 @_Z29work_group_scan_inclusive_addi(i32) #1
declare spir_func i32 @_Z29work_group_scan_inclusive_minj(i32) #1
declare spir_func i64 @_Z29work_group_scan_inclusive_maxl(i64) #1

; Function Attrs: convergent nounwind readnone
declare spir_func i64 @_Z13get_global_idj(i32) local_unnamed_addr #2

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "uniform-work-group-size"="false" "unsafe-fp-math"="false" "use-soft-float"="false" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87"}
attributes #1 = { convergent "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false"  "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87"}
attributes #2 = { convergent nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false"  "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87"}
attributes #3 = { convergent nounwind readnone }
attributes #4 = { convergent nounwind }

!llvm.module.flags = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!1}
!opencl.spir.version = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}

!sycl.kernels = !{!14}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 2, i32 0}
!2 = !{}
!3 = !{!"icx (ICX) dev.8.x.0"}
!4 = !{i32 1, i32 1}
!5 = !{!"none", !"none"}
!6 = !{!"int*", !"int*"}
!7 = !{!"", !""}
!8 = !{i1 false, i1 false}
!9 = !{i32 0, i32 0}
!10 = !{!11, !11, i64 0}
!11 = !{!"int", !12, i64 0}
!12 = !{!"omnipotent char", !13, i64 0}
!13 = !{!"Simple C/C++ TBAA"}
!14 = !{void (i32 addrspace(1)*, i32 addrspace(1)*)* @a}
!15 = !{i32 32}

; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZGVeN32uu_a {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN32uu_a {{.*}} call
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN32uu_a {{.*}} add
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN32uu_a {{.*}} add
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN32uu_a {{.*}} icmp
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN32uu_a {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN32uu_a {{.*}} call
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN32uu_a {{.*}} br
; DEBUGIFY-NOT: WARNING
