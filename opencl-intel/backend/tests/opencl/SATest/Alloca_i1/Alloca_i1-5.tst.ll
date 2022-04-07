; ModuleID = 'main'
source_filename = "1"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "spir64-unknown-unknown-intelfpga"

; Function Attrs: convergent nounwind
define spir_kernel void @set_false(i1 addrspace(1)* %data) #0 !kernel_arg_addr_space !6 !kernel_arg_access_qual !7 !kernel_arg_type !8 !kernel_arg_base_type !8 !kernel_arg_type_qual !9 !kernel_arg_host_accessible !10 !kernel_arg_name !11 {
entry:
  %data.addr = alloca i1 addrspace(1)*, align 8
  store i1 addrspace(1)* %data, i1 addrspace(1)** %data.addr, align 8, !tbaa !12
  %0 = load i1 addrspace(1)*, i1 addrspace(1)** %data.addr, align 8, !tbaa !12
  store i1 0, i1 addrspace(1)* %0, align 1, !tbaa !16
  ret void
}

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.linker.options = !{}
!llvm.module.flags = !{!0}
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
!4 = !{!"clang version 6.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 7fc81fc8922b226ea2e2c069ddae2e44619ea074) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm fcec820f4b80de4ed49d51d31292f09593b932e6)"}
!5 = !{void (i1 addrspace(1)*)* @set_false}
!6 = !{i32 1}
!7 = !{!"none"}
!8 = !{!"bool*"}
!9 = !{!""}
!10 = !{i1 false}
!11 = !{!"data"}
!12 = !{!13, !13, i64 0}
!13 = !{!"any pointer", !14, i64 0}
!14 = !{!"omnipotent char", !15, i64 0}
!15 = !{!"Simple C/C++ TBAA"}
!16 = !{!17, !17, i64 0}
!17 = !{!"bool", !14, i64 0}
