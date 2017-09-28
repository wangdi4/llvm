;The test checks that in case of type of channel element is struct
;channel transformation pass passes pointer to struct directly.
;
;IR was generated from:
;
;#pragma OPENCL EXTENSION cl_altera_channels : enable
;
;struct ST
;{
;    int z;
;};
;
;channel struct ST chan;
;
;__kernel void foo()
;{
;    struct ST st;
;    write_channel_altera(chan, st);
;}

; RUN: %oclopt -runtimelib=%p/../../vectorizer/Full/runtime.bc -channel-pipe-transformation -verify %s -S | FileCheck %s

; ModuleID = '/tmp/test.cl'
target datalayout = "e-p:32:32-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir-unknown-unknown-intelfpga"

%opencl.channel_t = type opaque
%struct.ST = type { i32 }

@chan = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4

; Function Attrs: nounwind
define spir_kernel void @foo() #0 {
; CHECK-NOT: %[[ADDITIONAL_ALLOCA:.*]] = alloca %struct.ST*
  %st = alloca %struct.ST, align 4
  %1 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @chan, align 4
; CHECK-NOT: store %struct.ST* %st, %struct.ST** %[[ADDITIONAL_ALLOCA]]
  call void @_Z20write_channel_altera11ocl_channel2STS_(%opencl.channel_t addrspace(1)* %1, %struct.ST* byval align 4 %st)
  ret void
}

declare void @_Z20write_channel_altera11ocl_channel2STS_(%opencl.channel_t addrspace(1)*, %struct.ST* byval align 4) #1

attributes #0 = { nounwind "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!opencl.kernels = !{!0}
!opencl.channels = !{!6}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!9}
!opencl.spir.version = !{!9}
!opencl.used.extensions = !{!10}
!opencl.used.optional.core.features = !{!10}
!opencl.compiler.options = !{!10}
!llvm.ident = !{!11}

!0 = !{void ()* @foo, !1, !2, !3, !4, !5}
!1 = !{!"kernel_arg_addr_space"}
!2 = !{!"kernel_arg_access_qual"}
!3 = !{!"kernel_arg_type"}
!4 = !{!"kernel_arg_base_type"}
!5 = !{!"kernel_arg_type_qual"}
!6 = !{%opencl.channel_t addrspace(1)* addrspace(1)* @chan, !7, !8}
!7 = !{!"packet_size", i32 4}
!8 = !{!"packet_align", i32 4}
!9 = !{i32 2, i32 0}
!10 = !{}
!11 = !{!"clang version 3.8.1 (ssh://nnopencl-git-01.inn.intel.com/home/git/repo/opencl_qa-clang 0bafc07483cc63a342ec92f71891b92814225fe7) (ssh://nnopencl-git-01.inn.intel.com/home/git/repo/opencl_qa-llvm 0b49c14901e27f89de0b13395e27c893769ecf71)"}
