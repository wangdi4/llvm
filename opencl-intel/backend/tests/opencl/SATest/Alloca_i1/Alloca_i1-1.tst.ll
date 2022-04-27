; ModuleID = 'main'
source_filename = "1"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "spir64-unknown-unknown-intelfpga"

; Function Attrs: convergent nounwind
define void @helper(i1 addrspace(4)* %x) #0 {
entry:
  %x.addr = alloca i1 addrspace(4)*, align 8
  store i1 addrspace(4)* %x, i1 addrspace(4)** %x.addr, align 8, !tbaa !6
  %0 = load i1 addrspace(4)*, i1 addrspace(4)** %x.addr, align 8, !tbaa !6
  store i1 0, i1 addrspace(4)* %0, align 1, !tbaa !10
  ret void
}

; Function Attrs: convergent nounwind
define spir_kernel void @set_false(i1 addrspace(1)* %data) #0 !kernel_arg_addr_space !12 !kernel_arg_access_qual !13 !kernel_arg_type !14 !kernel_arg_base_type !14 !kernel_arg_type_qual !15 !kernel_arg_host_accessible !16 !kernel_arg_pipe_depth !17 !kernel_arg_pipe_io !15 !kernel_arg_buffer_location !15 !kernel_arg_name !18 {
entry:
  %data.addr = alloca i1 addrspace(1)*, align 8
  store i1 addrspace(1)* %data, i1 addrspace(1)** %data.addr, align 8, !tbaa !6
  %0 = load i1 addrspace(1)*, i1 addrspace(1)** %data.addr, align 8, !tbaa !6
  %1 = addrspacecast i1 addrspace(1)* %0 to i1 addrspace(4)*
  call void @helper(i1 addrspace(4)* %1) #1
  ret void
}

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { convergent }

!llvm.linker.options = !{}
!llvm.module.flags = !{!0}
!opencl.ocl.version = !{!1}
!opencl.spir.version = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 2, i32 0}
!2 = !{}
!3 = !{!"-Idevice/", !"-cl-std=CL2.0"}
!4 = !{!"clang version 6.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 79a66fe08a5ec5ca52ccfe57b470a07663130d9d) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 855da9909de4908777194cc1995d96eaf3e79869)"}
!5 = !{void (i1 addrspace(1)*)* @set_false}
!6 = !{!7, !7, i64 0}
!7 = !{!"any pointer", !8, i64 0}
!8 = !{!"omnipotent char", !9, i64 0}
!9 = !{!"Simple C/C++ TBAA"}
!10 = !{!11, !11, i64 0}
!11 = !{!"bool", !8, i64 0}
!12 = !{i32 1}
!13 = !{!"none"}
!14 = !{!"bool*"}
!15 = !{!""}
!16 = !{i1 false}
!17 = !{i32 0}
!18 = !{!"data"}
