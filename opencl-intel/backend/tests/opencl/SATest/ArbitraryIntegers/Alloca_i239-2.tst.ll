; ModuleID = 'main'
source_filename = "1"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "spir64-unknown-unknown-intelfpga"

; Function Attrs: convergent nounwind
define i239 @helper(i239 %x) #0 {
entry:
  %x.addr = alloca i239, align 32
  store i239 %x, ptr %x.addr, align 32, !tbaa !5
  store i239 0, ptr %x.addr, align 32, !tbaa !5
  %0 = load i239, ptr %x.addr, align 4, !tbaa !6
  ret i239 %0
}

; Function Attrs: convergent nounwind
define spir_kernel void @set_zero(i239 %data) #0 !kernel_arg_addr_space !9 !kernel_arg_access_qual !10 !kernel_arg_type !11 !kernel_arg_base_type !11 !kernel_arg_type_qual !12 !kernel_arg_host_accessible !13 !kernel_arg_pipe_depth !9 !kernel_arg_pipe_io !12 !kernel_arg_buffer_location !12 !kernel_arg_name !14 {
entry:
  %data.addr = alloca i239, align 32
  store i239 %data, ptr %data.addr, align 32, !tbaa !5
  %0 = load i239, ptr %data.addr, align 32, !tbaa !5
  call i239 @helper(i239 %0) #1
  ret void
}

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { convergent noinline }

!llvm.linker.options = !{}
!llvm.module.flags = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!1}
!opencl.spir.version = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, i32 2}
!2 = !{}
!3 = !{!"-Idevice/"}
!4 = !{!"clang version 6.0.0"}
!5 = !{!6, !6, i64 0}
!6 = !{!"int", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}
!9 = !{i1 false}
!10 = !{!"none"}
!11 = !{!"i239"}
!12 = !{!""}
!13 = !{i1 false}
!14 = !{!"data"}
