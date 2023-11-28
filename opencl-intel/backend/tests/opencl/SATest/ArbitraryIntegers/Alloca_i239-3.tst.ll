; ModuleID = 'main'
source_filename = "1"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "spir64-unknown-unknown-intelfpga"

@.str = private unnamed_addr addrspace(2) constant [3 x i8] c"%d\00", align 1

; Function Attrs: convergent nounwind
define spir_kernel void @print_zero(i239 %data) #0 !kernel_arg_addr_space !6 !kernel_arg_access_qual !7 !kernel_arg_type !8 !kernel_arg_base_type !8 !kernel_arg_type_qual !9 !kernel_arg_host_accessible !10 !kernel_arg_pipe_depth !6 !kernel_arg_pipe_io !9 !kernel_arg_buffer_location !9 !kernel_arg_name !11 {
entry:
  %data.addr = alloca i239, align 32
  store i239 %data, ptr %data.addr, align 32, !tbaa !12
  store i239 0, ptr %data.addr, align 32, !tbaa !12
  %0 = load i239, ptr %data.addr, align 32, !tbaa !12
  %call = call i32 (ptr addrspace(2), ...) @printf(ptr addrspace(2) getelementptr inbounds ([3 x i8], ptr addrspace(2) @.str, i32 0, i32 0), i239 %0) #2
  ret void
}

; Function Attrs: convergent
declare i32 @printf(ptr addrspace(2), ...) #1

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { convergent "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zer    os-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { convergent }

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
!5 = !{ptr @print_zero}
!6 = !{i32 0}
!7 = !{!"none"}
!8 = !{!"i239"}
!9 = !{!""}
!10 = !{i1 false}
!11 = !{!"data"}
!12 = !{!13, !13, i64 0}
!13 = !{!"i239", !14, i64 0}
!14 = !{!"omnipotent char", !15, i64 0}
!15 = !{!"Simple C/C++ TBAA"}
