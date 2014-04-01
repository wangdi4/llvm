target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024"
target triple = "spir64-unknown-unknown"

; Function Attrs: nounwind
define spir_func void @test_get_local_size(<4 x i32> addrspace(1)* %a) #0 {
entry:
  %a.addr = alloca <4 x i32> addrspace(1)*, align 8
  %linear_gid = alloca i64, align 8
  store <4 x i32> addrspace(1)* %a, <4 x i32> addrspace(1)** %a.addr, align 8
  %call = call spir_func i64 @_Z20get_global_linear_idv() #1
  store i64 %call, i64* %linear_gid, align 8
  %call1 = call spir_func i64 @_Z14get_local_sizej(i32 0) #1
  %conv = trunc i64 %call1 to i32
  %0 = load i64* %linear_gid, align 8
  %1 = load <4 x i32> addrspace(1)** %a.addr, align 8
  %arrayidx = getelementptr inbounds <4 x i32> addrspace(1)* %1, i64 %0
  %2 = load <4 x i32> addrspace(1)* %arrayidx, align 16
  %3 = insertelement <4 x i32> %2, i32 %conv, i32 0
  store <4 x i32> %3, <4 x i32> addrspace(1)* %arrayidx, align 16
  %call2 = call spir_func i64 @_Z14get_local_sizej(i32 1) #1
  %conv3 = trunc i64 %call2 to i32
  %4 = load i64* %linear_gid, align 8
  %5 = load <4 x i32> addrspace(1)** %a.addr, align 8
  %arrayidx4 = getelementptr inbounds <4 x i32> addrspace(1)* %5, i64 %4
  %6 = load <4 x i32> addrspace(1)* %arrayidx4, align 16
  %7 = insertelement <4 x i32> %6, i32 %conv3, i32 1
  store <4 x i32> %7, <4 x i32> addrspace(1)* %arrayidx4, align 16
  %call5 = call spir_func i64 @_Z14get_local_sizej(i32 2) #1
  %conv6 = trunc i64 %call5 to i32
  %8 = load i64* %linear_gid, align 8
  %9 = load <4 x i32> addrspace(1)** %a.addr, align 8
  %arrayidx7 = getelementptr inbounds <4 x i32> addrspace(1)* %9, i64 %8
  %10 = load <4 x i32> addrspace(1)* %arrayidx7, align 16
  %11 = insertelement <4 x i32> %10, i32 %conv6, i32 2
  store <4 x i32> %11, <4 x i32> addrspace(1)* %arrayidx7, align 16
  ret void
}

; Function Attrs: nounwind readnone
declare spir_func i64 @_Z20get_global_linear_idv() #1

; Function Attrs: nounwind readnone
declare spir_func i64 @_Z14get_local_sizej(i32) #1

attributes #0 = { nounwind }
attributes #1 = { nounwind readnone }

!opencl.kernels = !{!0}
!opencl.spir.version = !{!7}
!opencl.ocl.version = !{!8}
!opencl.used.extensions = !{!9}
!opencl.used.optional.core.features = !{!9}
!opencl.compiler.options = !{!10}

!0 = metadata !{void (<4 x i32> addrspace(1)*)* @test_get_local_size, metadata !1, metadata !2, metadata !3, metadata !4, metadata !5, metadata !6}
!1 = metadata !{metadata !"kernel_arg_addr_space", i32 1}
!2 = metadata !{metadata !"kernel_arg_access_qual", metadata !"none"}
!3 = metadata !{metadata !"kernel_arg_type", metadata !"int4*"}
!4 = metadata !{metadata !"kernel_arg_type_qual", metadata !""}
!5 = metadata !{metadata !"kernel_arg_base_type", metadata !"int4*"}
!6 = metadata !{metadata !"kernel_arg_name", metadata !"a"}
!7 = metadata !{i32 1, i32 2}
!8 = metadata !{i32 2, i32 0}
!9 = metadata !{}
!10 = metadata !{metadata !"-cl-std=CL2.0"}
