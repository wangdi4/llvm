; RUN: opt -module-cleanup -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll

; CHECK-NOT:    @_NSConcreteGlobalBlock
; CHECK-NOT:    external

@_NSConcreteGlobalBlock = external global i8*
@.str = private unnamed_addr constant [9 x i8] c"i12@?0i8\00", align 1
@__block_descriptor_tmp = internal constant { i64, i64, i8*, i8* } { i64 0, i64 32, i8* getelementptr inbounds ([9 x i8]* @.str, i32 0, i32 0), i8* null }

define spir_kernel void @block_typedef_reassign(i32 addrspace(1)* %res) nounwind {
  ret void
}

!opencl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.spir.version = !{!6}
!opencl.ocl.version = !{!7}
!opencl.used.extensions = !{!8}
!opencl.used.optional.core.features = !{!8}
!opencl.compiler.options = !{!9}

!0 = !{void (i32 addrspace(1)*)* @block_typedef_reassign, !1, !2, !3, !4, !5}
!1 = !{!"kernel_arg_addr_space", i32 1}
!2 = !{!"kernel_arg_access_qual", !"none"}
!3 = !{!"kernel_arg_type", !"int*"}
!4 = !{!"kernel_arg_type_qual", !""}
!5 = !{!"kernel_arg_name", !"res"}
!6 = !{i32 1, i32 0}
!7 = !{i32 2, i32 0}
!8 = !{}
!9 = !{!"-cl-std=CL2.0"}
