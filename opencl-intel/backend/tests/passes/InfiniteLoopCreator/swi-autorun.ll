; Compiled from:
; ----------------------------------------------------
; #pragma OPENCL EXTENSION cl_altera_channels: enable
;
; channel int in;
; channel int out;
;
; __attribute__((autorun))
; __attribute__((max_global_work_dim(0)))
; void __kernel test_autorun_1() {
;   int a = 10;
;   write_channel_intel(out, a);
;   a = read_channel_intel(in);
; }
; ----------------------------------------------------
; Clang options: -cc1 -emit-llvm -triple spir64-unknown-unknown-intelfpga -disable-llvm-passes -x cl -cl-std=CL2.0
; ----------------------------------------------------
; Opt passes: -spir-materializer
; ----------------------------------------------------
; RUN: %oclopt -runtimelib=%p/../../vectorizer/Full/runtime.bc -infinite-loop-creator -verify %s -S | FileCheck %s
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown-intelfpga"

%opencl.channel_t = type opaque

@out = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4
@in = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4

; CHECK: define void @test_autorun_1
; CHECK: br label %infinite_loop_entry

; CHECK-NOT: ret
; CHECK-NOT: br label %infinite_loop_entry

; CHECK: br label %infinite_loop_entry
; CHECK-NEXT: }

; Function Attrs: nounwind
define void @test_autorun_1() #0 !kernel_arg_addr_space !6 !kernel_arg_access_qual !6 !kernel_arg_type !6 !kernel_arg_base_type !6 !kernel_arg_type_qual !6 !task !9 !autorun !9 {
  %a = alloca i32, align 4
  %1 = bitcast i32* %a to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %1) #3
  store i32 10, i32* %a, align 4, !tbaa !10
  %2 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @out, align 4, !tbaa !14
  %3 = load i32, i32* %a, align 4, !tbaa !10
  call void @_Z19write_channel_intel11ocl_channelii(%opencl.channel_t addrspace(1)* %2, i32 %3)
  %4 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @in, align 4, !tbaa !14
  %call = call i32 @_Z18read_channel_intel11ocl_channeli(%opencl.channel_t addrspace(1)* %4)
  store i32 %call, i32* %a, align 4, !tbaa !10
  %5 = bitcast i32* %a to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %5) #3
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

declare void @_Z19write_channel_intel11ocl_channelii(%opencl.channel_t addrspace(1)*, i32) #2

declare i32 @_Z18read_channel_intel11ocl_channeli(%opencl.channel_t addrspace(1)*) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }

!opencl.channels = !{!0, !3}
!llvm.module.flags = !{!4}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!5}
!opencl.spir.version = !{!5}
!opencl.used.extensions = !{!6}
!opencl.used.optional.core.features = !{!6}
!opencl.compiler.options = !{!6}
!llvm.ident = !{!7}
!opencl.kernels = !{!8}

!0 = !{%opencl.channel_t addrspace(1)* addrspace(1)* @in, !1, !2}
!1 = !{!"packet_size", i32 4}
!2 = !{!"packet_align", i32 4}
!3 = !{%opencl.channel_t addrspace(1)* addrspace(1)* @out, !1, !2}
!4 = !{i32 1, !"wchar_size", i32 4}
!5 = !{i32 2, i32 0}
!6 = !{}
!7 = !{!"clang version 5.0.0 "}
!8 = !{void ()* @test_autorun_1}
!9 = !{i1 true}
!10 = !{!11, !11, i64 0}
!11 = !{!"int", !12, i64 0}
!12 = !{!"omnipotent char", !13, i64 0}
!13 = !{!"Simple C/C++ TBAA"}
!14 = !{!12, !12, i64 0}
