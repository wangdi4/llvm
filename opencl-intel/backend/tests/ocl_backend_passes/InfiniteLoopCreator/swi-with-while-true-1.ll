; Compiled from:
; ----------------------------------------------------
; #pragma OPENCL EXTENSION cl_altera_channels: enable
;
; channel int out;
;
; __attribute__((autorun))
; __attribute__((max_global_work_dim(0)))
; void __kernel test_autorun_1() {
;   int a = 10;
;   while (true) {
;     write_channel_intel(out, a++);
;   }
; }
; ----------------------------------------------------
; Clang options: -cc1 -emit-llvm -triple spir64-unknown-unknown-intelfpga -disable-llvm-passes -x cl -cl-std=CL2.0
; ----------------------------------------------------
; Opt passes: -spir-materializer
; ----------------------------------------------------
; without -cl-loop-creator there are no ret instructions (or all of them are
; unreachable), check that pass doesn't change the ir
; RUN: %oclopt -runtimelib=%p/../../vectorizer/Full/runtime.bc -kernel-analysis -infinite-loop-creator -verify %s -S > %t1.ll
; RUN: %oclopt -runtimelib=%p/../../vectorizer/Full/runtime.bc -kernel-analysis -verify %s -S > %t2.ll
; diff %t1.ll %t2.ll
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown-intelfpga"

%opencl.channel_t = type opaque

@out = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4

; Function Attrs: nounwind
define void @test_autorun_1() #0 !kernel_arg_addr_space !5 !kernel_arg_access_qual !5 !kernel_arg_type !5 !kernel_arg_base_type !5 !kernel_arg_type_qual !5 !task !8 !autorun !8 {
  %a = alloca i32, align 4
  %1 = bitcast i32* %a to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %1) #3
  store i32 10, i32* %a, align 4, !tbaa !9
  br label %2

; <label>:2:                                      ; preds = %3, %0
  br label %3

; <label>:3:                                      ; preds = %2
  %4 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @out, align 4, !tbaa !13
  %5 = load i32, i32* %a, align 4, !tbaa !9
  %inc = add nsw i32 %5, 1
  store i32 %inc, i32* %a, align 4, !tbaa !9
  call void @_Z19write_channel_intel11ocl_channelii(%opencl.channel_t addrspace(1)* %4, i32 %5)
  br label %2
                                                  ; No predecessors!
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

declare void @_Z19write_channel_intel11ocl_channelii(%opencl.channel_t addrspace(1)*, i32) #2

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

!0 = !{%opencl.channel_t addrspace(1)* addrspace(1)* @out, !1, !2}
!1 = !{!"packet_size", i32 4}
!2 = !{!"packet_align", i32 4}
!3 = !{i32 1, !"wchar_size", i32 4}
!4 = !{i32 2, i32 0}
!5 = !{}
!6 = !{!"clang version 5.0.0 "}
!7 = !{void ()* @test_autorun_1}
!8 = !{i1 true}
!9 = !{!10, !10, i64 0}
!10 = !{!"int", !11, i64 0}
!11 = !{!"omnipotent char", !12, i64 0}
!12 = !{!"Simple C/C++ TBAA"}
!13 = !{!11, !11, i64 0}
