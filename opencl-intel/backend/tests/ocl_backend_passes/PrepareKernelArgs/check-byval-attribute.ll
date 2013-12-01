; RUN: opt -add-implicit-args -local-buffers -prepare-kernel-args -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-f80:32:32-n8:16:32-S32"

%struct.struct1 = type <{ <4 x i32> , i8}>
%struct.struct2 = type <{ i32 ,i32 ,i32 }>

; CHECK: @t1
define void @t1(%struct.struct1* %arg1, %struct.struct2* %arg2, <4 x i32>* byval %arg3) {
entry:
  ret void
}

;; struct1 my_struct1 arg1 - 17 bytes - expected alignment: 0
; CHECK: [[ARG0_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8* %pBuffer, i32 0
; CHECK-NEXT: [[ARG0_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG0_BUFF_INDEX]] to %struct.struct1*
;; struct2 my_struct2 arg2 - 12 bytes - expected alignment: 0
; CHECK-NEXT: [[ARG1_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8* %pBuffer, i32 17
; CHECK-NEXT: [[ARG1_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG1_BUFF_INDEX]] to %struct.struct2*
;; int4 byvalue , size is the actual size not the pointer size - expected alignment:16 (17+12 = 29 so it's aligned to 32)
; CHECK-NEXT: [[ARG2_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8* %pBuffer, i32 32
; CHECK-NEXT: [[ARG2_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG2_BUFF_INDEX]] to <4 x i32>*
;;implicit args
;; call original func
; CHECK: call void @__t1_separated_args(%struct.struct1* [[ARG0_TYPECAST]], %struct.struct2* [[ARG1_TYPECAST]], <4 x i32>* [[ARG2_TYPECAST]], [[IMPLICIT_ARGS:[a-zA-Z0-9]+]]
; CHECK-NEXT: ret void

!opencl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.spir.version = !{!6}
!opencl.ocl.version = !{!7}
!opencl.used.extensions = !{!8}
!opencl.used.optional.core.features = !{!8}
!opencl.compiler.options = !{!8}

!0 = metadata !{void (%struct.struct1*,%struct.struct2*, <4 x i32>*)* @t1, metadata !1, metadata !2, metadata !3, metadata !4, metadata !5}
!1 = metadata !{metadata !"kernel_arg_addr_space", i32 0, i32 0, i32 0}
!2 = metadata !{metadata !"kernel_arg_access_qual", metadata !"none", metadata !"none", metadata !"none"}
!3 = metadata !{metadata !"kernel_arg_type", metadata !"my_struct1*", metadata !"my_struct2*", metadata !"int4ByVal"}
!4 = metadata !{metadata !"kernel_arg_type_qual", metadata !"", metadata !"", metadata !""}
!5 = metadata !{metadata !"kernel_arg_name", metadata !"arg1", metadata !"arg2", metadata !"arg3"}
!6 = metadata !{i32 1, i32 0}
!7 = metadata !{i32 0, i32 0}
!8 = metadata !{}
