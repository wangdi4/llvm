; This code was compiled from llvm/test/Transforms/SYCLTransforms/ChannelPipeTransformation/phi.ll
; by the follow command:
; opt -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl.bc -sycl-kernel-channel-pipe-transformation -S
; ----------------------------------------------------
; RUN: llvm-as %p/../Inputs/fpga-pipes.rtl -o %t.rtl.bc
; RUN: opt -sycl-kernel-builtin-lib=%t.rtl.bc -passes=sycl-kernel-pipe-support %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -sycl-kernel-builtin-lib=%t.rtl.bc -passes=sycl-kernel-pipe-support %s -S | FileCheck %s

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown-intelfpga"

@ch1 = internal addrspace(1) global target("spirv.Channel") zeroinitializer, align 4, !packet_size !0, !packet_align !0
@ch2 = internal addrspace(1) global target("spirv.Channel") zeroinitializer, align 4, !packet_size !0, !packet_align !0
@llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @__pipe_global_ctor, ptr null }]
@ch1.pipe = addrspace(1) global ptr addrspace(1) null, align 8, !packet_size !0, !packet_align !0
@ch1.pipe.bs = addrspace(1) global [328 x i8] zeroinitializer, align 4
@ch2.pipe = addrspace(1) global ptr addrspace(1) null, align 8, !packet_size !0, !packet_align !0
@ch2.pipe.bs = addrspace(1) global [328 x i8] zeroinitializer, align 4

; Function Attrs: nounwind
define spir_kernel void @k(ptr addrspace(1) %cond, ptr addrspace(1) %res) #0 !kernel_arg_addr_space !5 !kernel_arg_access_qual !6 !kernel_arg_type !7 !kernel_arg_base_type !7 !kernel_arg_type_qual !8 !kernel_arg_host_accessible !9 !arg_type_null_val !10 {
entry:
  %read.dst = alloca i32, align 4
  %ch1v4 = load ptr addrspace(1), ptr addrspace(1) @ch1.pipe, align 8, !tbaa !11
  %ch1v = load ptr addrspace(1), ptr addrspace(1) @ch1, align 4, !tbaa !11
  %ch2v1 = load ptr addrspace(1), ptr addrspace(1) @ch2.pipe, align 8, !tbaa !11
  %ch2v = load ptr addrspace(1), ptr addrspace(1) @ch2, align 4, !tbaa !11
  %tobool = icmp ne ptr addrspace(1) %cond, null
  br i1 %tobool, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  br label %if.end

if.else:                                          ; preds = %entry
  br label %if.end

if.end:                                           ; preds = %if.end, %if.else, %if.then
  %chphi2 = phi ptr addrspace(1) [ %ch1v4, %if.then ], [ %ch2v1, %if.else ], [ %chphi2, %if.end ]
  %chphi = phi ptr addrspace(1) [ %ch1v, %if.then ], [ %ch2v, %if.else ], [ %chphi, %if.end ]
  %0 = addrspacecast ptr %read.dst to ptr addrspace(4)
  %call3 = call i32 @__read_pipe_2_bl_fpga(ptr addrspace(1) %chphi2, ptr addrspace(4) %0, i32 4, i32 4)
  %1 = load i32, ptr %read.dst, align 4
  %again = icmp ne i32 %1, 1
  br i1 %again, label %if.end, label %exit

; CHECK:      %[[PIPERW:.*]] = phi ptr
; CHECK:      call void @__store_read_pipe_use({{.*}} ptr addrspace(1) %[[PIPERW]]
; CHECK:      %[[CALL:.+]] = call i32 @__read_pipe_2_fpga(ptr addrspace(1) %[[PIPERW]]
; CHECK-NEXT: %[[ICMP:.+]] = icmp ne i32 %[[CALL]], 0
; CHECK-NEXT: br i1 %[[ICMP]], label %[[FLUSHBB:[0-9]+]]
; CHECK:      [[FLUSHBB]]:
; CHECK-NEXT: call void @__flush_pipe_read_array
; CHECK-NEXT: call void @__flush_pipe_write_array

exit:                                             ; preds = %if.end
  ret void
}

define void @__pipe_global_ctor() {
entry:
  call void @__pipe_init_fpga(ptr addrspace(1) @ch1.pipe.bs, i32 4, i32 0, i32 0)
  store ptr addrspace(1) @ch1.pipe.bs, ptr addrspace(1) @ch1.pipe, align 8
  call void @__pipe_init_fpga(ptr addrspace(1) @ch2.pipe.bs, i32 4, i32 0, i32 0)
  store ptr addrspace(1) @ch2.pipe.bs, ptr addrspace(1) @ch2.pipe, align 8
  ret void
}

; Function Attrs: nounwind memory(none)
declare void @__pipe_init_fpga(ptr addrspace(1), i32, i32, i32) #1

; Function Attrs: nounwind memory(none)
declare i32 @__read_pipe_2_fpga(ptr addrspace(1), ptr addrspace(4) nocapture, i32, i32) #1

declare i32 @__read_pipe_2_bl_fpga(ptr addrspace(1), ptr addrspace(4), i32, i32)

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind memory(none) }

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
!10 = !{ptr addrspace(1) null, ptr addrspace(1) null}
!11 = !{!12, !12, i64 0}
!12 = !{!"omnipotent char", !13, i64 0}
!13 = !{!"Simple C/C++ TBAA"}

; DEBUGIFY-NOT: WARNING
