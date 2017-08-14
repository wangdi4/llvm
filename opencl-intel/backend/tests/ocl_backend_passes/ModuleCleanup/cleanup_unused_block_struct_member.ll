; RUN: opt -module-cleanup -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll

; CHECK-NOT:    @_NSConcreteGlobalBlock
; CHECK-NOT:    external

@_NSConcreteGlobalBlock = external global i8*
@.str = private unnamed_addr constant [9 x i8] c"i12@?0i8\00", align 1
@__block_descriptor_tmp = internal constant { i64, i64, i8*, i8* } { i64 0, i64 32, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str, i32 0, i32 0), i8* null }

define spir_kernel void @block_typedef_reassign(i32 addrspace(1)* %res) nounwind !kernel_arg_addr_space !1 !kernel_arg_access_qual !2 !kernel_arg_type !3 !kernel_arg_type_qual !5 !kernel_arg_name !6 {
  ret void
}

!1 = !{i32 1}
!2 = !{!"none"}
!3 = !{!"int*"}
!5 = !{!""}
!6 = !{!"res"}
