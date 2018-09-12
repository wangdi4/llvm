; The test checks that in case of type of channel element is struct
; channel transformation pass passes pointer to struct directly.
; ----------------------------------------------------
; Compiled from:
; ----------------------------------------------------
; #pragma OPENCL EXTENSION cl_intel_channels : enable
;
; struct ST
; {
;     int z;
; };
;
; channel struct ST chan;
;
; __kernel void foo()
; {
;     struct ST st;
;     write_channel_intel(chan, st);
; }
; ----------------------------------------------------
; Compile options: -cc1 -emit-llvm -triple spir64-unknown-unknown-intelfpga -disable-llvm-passes -x cl -cl-std=CL2.0
; ----------------------------------------------------
; REQUIRES: fpga-emulator
; RUN: %oclopt -runtimelib=%p/../../vectorizer/Full/runtime.bc -channel-pipe-transformation -verify %s -S | FileCheck %s
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown-intelfpga"

%opencl.channel_t = type opaque
%struct.ST = type { i32 }

@chan = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4, !packet_size !0, !packet_align !0

; Function Attrs: nounwind
define spir_kernel void @foo() #0 !kernel_arg_addr_space !5 !kernel_arg_access_qual !5 !kernel_arg_type !5 !kernel_arg_base_type !5 !kernel_arg_type_qual !5 {
; CHECK-NOT: %[[ADDITIONAL_ALLOCA:.*]] = alloca %struct.ST*
entry:
  %st = alloca %struct.ST, align 4
  %0 = bitcast %struct.ST* %st to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #3
  %1 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @chan, align 4, !tbaa !7
; CHECK-NOT: store %struct.ST* %st, %struct.ST** %[[ADDITIONAL_ALLOCA]]
  call void @_Z19write_channel_intel11ocl_channel2STS_(%opencl.channel_t addrspace(1)* %1, %struct.ST* byval align 4 %st)
  %2 = bitcast %struct.ST* %st to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %2) #3
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

declare void @_Z19write_channel_intel11ocl_channel2STS_(%opencl.channel_t addrspace(1)*, %struct.ST* byval align 4) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }

!llvm.module.flags = !{!3}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!4}
!opencl.spir.version = !{!4}
!opencl.used.extensions = !{!5}
!opencl.used.optional.core.features = !{!5}
!opencl.compiler.options = !{!5}
!llvm.ident = !{!6}

!0 = !{i32 4}
!3 = !{i32 1, !"wchar_size", i32 4}
!4 = !{i32 2, i32 0}
!5 = !{}
!6 = !{!"clang version 5.0.0 "}
!7 = !{!8, !8, i64 0}
!8 = !{!"omnipotent char", !9, i64 0}
!9 = !{!"Simple C/C++ TBAA"}
