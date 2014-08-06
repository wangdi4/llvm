target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024"
target triple = "spir64-unknown-unknown"

; Function Attrs: nounwind
define spir_func void @test_buitlins_with_gnas(float addrspace(1)* %in) #0 {
entry:
  %in.addr = alloca float addrspace(1)*, align 8
  %a = alloca float, align 4
  %b = alloca float, align 4
  %res = alloca float, align 4
  %c = alloca i32, align 4
  store float addrspace(1)* %in, float addrspace(1)** %in.addr, align 8
  store float 0x4010CCCCC0000000, float* %a, align 4
  %0 = load float* %a, align 4
  %1 = bitcast float* %b to float addrspace(4)*
  %call = call spir_func float @_Z5fractfPU3AS4f(float %0, float addrspace(4)* %1)
  store float %call, float* %res, align 4
  %2 = load float* %a, align 4
  %3 = bitcast i32* %c to i32 addrspace(4)*
  %call1 = call spir_func float @_Z5frexpfPU3AS4i(float %2, i32 addrspace(4)* %3)
  store float %call1, float* %res, align 4
  %4 = load float* %a, align 4
  %5 = bitcast i32* %c to i32 addrspace(4)*
  %call2 = call spir_func float @_Z8lgamma_rfPU3AS4i(float %4, i32 addrspace(4)* %5)
  store float %call2, float* %res, align 4
  %6 = load float* %a, align 4
  %7 = bitcast float* %b to float addrspace(4)*
  %call3 = call spir_func float @_Z4modffPU3AS4f(float %6, float addrspace(4)* %7)
  store float %call3, float* %res, align 4
  %8 = load float* %a, align 4
  %9 = load float* %b, align 4
  %10 = bitcast i32* %c to i32 addrspace(4)*
  %call4 = call spir_func float @_Z6remquoffPU3AS4i(float %8, float %9, i32 addrspace(4)* %10)
  store float %call4, float* %res, align 4
  %11 = load float* %a, align 4
  %12 = bitcast float* %b to float addrspace(4)*
  %call5 = call spir_func float @_Z6sincosfPU3AS4f(float %11, float addrspace(4)* %12)
  store float %call5, float* %res, align 4
  ret void
}

declare spir_func float @_Z5fractfPU3AS4f(float, float addrspace(4)*) #1

declare spir_func float @_Z5frexpfPU3AS4i(float, i32 addrspace(4)*) #1

declare spir_func float @_Z8lgamma_rfPU3AS4i(float, i32 addrspace(4)*) #1

declare spir_func float @_Z4modffPU3AS4f(float, float addrspace(4)*) #1

declare spir_func float @_Z6remquoffPU3AS4i(float, float, i32 addrspace(4)*) #1

declare spir_func float @_Z6sincosfPU3AS4f(float, float addrspace(4)*) #1

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-frame-pointer-elim-non-leaf"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-frame-pointer-elim-non-leaf"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }

!opencl.kernels = !{!0}
!opencl.spir.version = !{!7}
!opencl.ocl.version = !{!8}
!opencl.used.extensions = !{!9}
!opencl.used.optional.core.features = !{!9}
!opencl.compiler.options = !{!10}

!0 = metadata !{void (float addrspace(1)*)* @test_buitlins_with_gnas, metadata !1, metadata !2, metadata !3, metadata !4, metadata !5, metadata !6}
!1 = metadata !{metadata !"kernel_arg_addr_space", i32 1}
!2 = metadata !{metadata !"kernel_arg_access_qual", metadata !"none"}
!3 = metadata !{metadata !"kernel_arg_type", metadata !"float*"}
!4 = metadata !{metadata !"kernel_arg_type_qual", metadata !""}
!5 = metadata !{metadata !"kernel_arg_base_type", metadata !"float*"}
!6 = metadata !{metadata !"kernel_arg_name", metadata !"in"}
!7 = metadata !{i32 1, i32 2}
!8 = metadata !{i32 2, i32 0}
!9 = metadata !{}
!10 = metadata !{metadata !"-cl-std=CL2.0"}