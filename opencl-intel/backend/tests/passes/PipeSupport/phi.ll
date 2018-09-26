; This code was compiled from backend/tests/passes/ChannelPipeTransformation/phi.ll
; by the follow command:
; %oclopt -runtimelib=%p/../../vectorizer/Full/runtime.bc -channel-pipe-transformation -verify -S
; ----------------------------------------------------
; REQUIRES: fpga-emulator
; RUN: %oclopt -runtimelib=%p/../../vectorizer/Full/runtime.bc -pipe-support -verify %s -S | FileCheck %s

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown-intelfpga"

%opencl.channel_t = type opaque
%opencl.pipe_rw_t = type opaque
%opencl.pipe_ro_t = type opaque
%struct.__pipe_t = type { i32, i32, i32, i32, i32, i32, [0 x i8] }

@ch1 = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4, !packet_size !0, !packet_align !0
@ch2 = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4, !packet_size !0, !packet_align !0
@llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 65535, void ()* @__pipe_global_ctor, i8* null }]
@ch1.pipe = addrspace(1) global %opencl.pipe_rw_t addrspace(1)* null, align 8, !packet_size !0, !packet_align !0
@ch1.pipe.bs = addrspace(1) global [328 x i8] zeroinitializer, align 4
@ch2.pipe = addrspace(1) global %opencl.pipe_rw_t addrspace(1)* null, align 8, !packet_size !0, !packet_align !0
@ch2.pipe.bs = addrspace(1) global [328 x i8] zeroinitializer, align 4

; Function Attrs: nounwind
define spir_kernel void @k(i32 addrspace(1)* %cond, i32 addrspace(1)* %res) #0 !kernel_arg_addr_space !5 !kernel_arg_access_qual !6 !kernel_arg_type !7 !kernel_arg_base_type !7 !kernel_arg_type_qual !8 !kernel_arg_host_accessible !9 {
entry:
  %read.dst = alloca i32
  %ch1v4 = load %opencl.pipe_rw_t addrspace(1)*, %opencl.pipe_rw_t addrspace(1)* addrspace(1)* @ch1.pipe
  %ch1v = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @ch1, align 4, !tbaa !10
  %ch2v1 = load %opencl.pipe_rw_t addrspace(1)*, %opencl.pipe_rw_t addrspace(1)* addrspace(1)* @ch2.pipe
  %ch2v = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @ch2, align 4, !tbaa !10
  %tobool = icmp ne i32 addrspace(1)* %cond, null
  br i1 %tobool, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  br label %if.end

if.else:                                          ; preds = %entry
  br label %if.end

if.end:                                           ; preds = %if.end, %if.else, %if.then
  %chphi2 = phi %opencl.pipe_rw_t addrspace(1)* [ %ch1v4, %if.then ], [ %ch2v1, %if.else ], [ %chphi2, %if.end ]
  %chphi = phi %opencl.channel_t addrspace(1)* [ %ch1v, %if.then ], [ %ch2v, %if.else ], [ %chphi, %if.end ]
  %0 = bitcast %opencl.pipe_rw_t addrspace(1)* %chphi2 to %opencl.pipe_ro_t addrspace(1)*
  %1 = addrspacecast i32* %read.dst to i8 addrspace(4)*
  %call3 = call i32 @__read_pipe_2_bl_intel(%opencl.pipe_ro_t addrspace(1)* %0, i8 addrspace(4)* %1, i32 4, i32 4)
  %2 = load i32, i32* %read.dst
  %again = icmp ne i32 %2, 1
  br i1 %again, label %if.end, label %exit

; CHECK:      %[[PIPERW:.*]] = phi %opencl.pipe_rw_t
; CHECK:      %[[PIPERO:[0-9]+]] = bitcast %opencl.pipe_rw_t {{.*}} %[[PIPERW]] to %opencl.pipe_ro_t
; CHECK:      call void @__store_read_pipe_use({{.*}} %opencl.pipe_ro_t addrspace(1)* %[[PIPERO:[0-9]+]]
; CHECK:      %[[CALL:.+]] = call i32 @__read_pipe_2_intel(%opencl.pipe_ro_t addrspace(1)* %[[PIPERO]]
; CHECK-NEXT: %[[ICMP:.+]] = icmp ne i32 %[[CALL]], 0
; CHECK-NEXT: br i1 %[[ICMP]], label %[[FLUSHBB:[0-9]+]]
; CHECK:      ; <label>:[[FLUSHBB]]
; CHECK-NEXT: call void @__flush_pipe_read_array
; CHECK-NEXT: call void @__flush_pipe_write_array

exit:                                             ; preds = %if.end
  ret void
}

define void @__pipe_global_ctor() {
entry:
  call void @__pipe_init_intel(%struct.__pipe_t addrspace(1)* bitcast ([328 x i8] addrspace(1)* @ch1.pipe.bs to %struct.__pipe_t addrspace(1)*), i32 4, i32 0, i32 0)
  store %opencl.pipe_rw_t addrspace(1)* bitcast ([328 x i8] addrspace(1)* @ch1.pipe.bs to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* @ch1.pipe
  call void @__pipe_init_intel(%struct.__pipe_t addrspace(1)* bitcast ([328 x i8] addrspace(1)* @ch2.pipe.bs to %struct.__pipe_t addrspace(1)*), i32 4, i32 0, i32 0)
  store %opencl.pipe_rw_t addrspace(1)* bitcast ([328 x i8] addrspace(1)* @ch2.pipe.bs to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* @ch2.pipe
  ret void
}

; Function Attrs: nounwind readnone
declare void @__pipe_init_intel(%struct.__pipe_t addrspace(1)*, i32, i32, i32) #1

; Function Attrs: nounwind readnone
declare i32 @__read_pipe_2_intel(%opencl.pipe_ro_t addrspace(1)*, i8 addrspace(4)* nocapture, i32, i32) #1

declare i32 @__read_pipe_2_bl_intel(%opencl.pipe_ro_t addrspace(1)*, i8 addrspace(4)*, i32, i32)

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }

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
!11 = !{!"omnipotent char", !12, i64 0}
!12 = !{!"Simple C/C++ TBAA"}
