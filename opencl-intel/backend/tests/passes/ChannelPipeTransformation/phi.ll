; This code wasn't compiled from OpenCL C source.
; ----------------------------------------------------
; REQUIRES: fpga-emulator
; RUN: %oclopt -runtimelib=%p/../../vectorizer/Full/runtime.bc -channel-pipe-transformation -verify %s -S | FileCheck %s

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown-intelfpga"

%opencl.channel_t = type opaque

@ch1 = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4, !packet_size !0, !packet_align !0
@ch2 = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4, !packet_size !0, !packet_align !0

; CHECK: @[[PIPE1:.*]] = addrspace(1) global %opencl.pipe_rw_t addrspace(1)*
; CHECK: @[[PIPE2:.*]] = addrspace(1) global %opencl.pipe_rw_t addrspace(1)*

; CHECK: %[[PIPE1VAL:.*]] = load %opencl.pipe_rw_t addrspace(1)*{{.*}} @[[PIPE1]]
; CHECK: %[[PIPE2VAL:.*]] = load %opencl.pipe_rw_t addrspace(1)*{{.*}} @[[PIPE2]]

; CHECK: %[[PHI:.*]] = phi %opencl.pipe_rw_t addrspace(1)* [ %[[PIPE1VAL]], %if.then ], [ %[[PIPE2VAL]], %if.else ], [ %[[PHI]], %if.end ]
; CHECK: %[[PHICAST:.*]] = bitcast %opencl.pipe_rw_t addrspace(1)* %[[PHI]] to %opencl.pipe_ro_t
; CALL: call i32 @__read_pipe_2_bl_intel(%opencl.pipe_ro_t addrspace(1)* %[[PHICAST]]

; Function Attrs: nounwind
define spir_kernel void @k(i32 addrspace(1)* %cond, i32 addrspace(1)* %res) #0 !kernel_arg_addr_space !5 !kernel_arg_access_qual !6 !kernel_arg_type !7 !kernel_arg_base_type !7 !kernel_arg_type_qual !8 !kernel_arg_host_accessible !9 {
entry:
  %ch1v = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @ch1, align 4, !tbaa !16
  %ch2v = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @ch2, align 4, !tbaa !16

  %tobool = icmp ne i32 addrspace(1)* %cond, null
  br i1 %tobool, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  br label %if.end

if.else:                                          ; preds = %entry
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %chphi = phi %opencl.channel_t addrspace(1)* [%ch1v, %if.then], [%ch2v, %if.else], [%chphi, %if.end]
  %call = call i32 @_Z18read_channel_intel11ocl_channeli(%opencl.channel_t addrspace(1)* %chphi)
  %again = icmp ne i32 %call, 1
  br i1 %again, label %if.end, label %exit

exit:                                            ; preds = %if.end
  ret void
}

declare i32 @_Z18read_channel_intel11ocl_channeli(%opencl.channel_t addrspace(1)*) #1

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!1}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!2}
!opencl.spir.version = !{!2}
!opencl.used.extensions = !{!3}
!opencl.used.optional.core.features = !{!3}
!opencl.compiler.options = !{!3}
!llvm.ident = !{!4}

!0 = !{i32 4}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 2, i32 0}
!3 = !{}
!4 = !{!"clang version 6.0.0"}
!5 = !{i32 1, i32 1}
!6 = !{!"none", !"none"}
!7 = !{!"int*", !"int*"}
!8 = !{!"", !""}
!9 = !{i1 false, i1 false}
!10 = !{!11, !11, i64 0}
!11 = !{!"any pointer", !12, i64 0}
!12 = !{!"omnipotent char", !13, i64 0}
!13 = !{!"Simple C/C++ TBAA"}
!14 = !{!15, !15, i64 0}
!15 = !{!"int", !12, i64 0}
!16 = !{!12, !12, i64 0}
