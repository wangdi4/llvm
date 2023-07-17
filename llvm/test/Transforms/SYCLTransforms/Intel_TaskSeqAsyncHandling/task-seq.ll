; RUN: opt -S -passes=sycl-kernel-handle-taskseq-async %s | FileCheck %s
; RUN: opt -S -passes=sycl-kernel-handle-taskseq-async %s -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY

%class.task_sequence = type { i32, i64 }
%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range" = type { %"class._ZTSN2cl4sycl6detail5arrayILi1EEE.cl::sycl::detail::array" }
%"class._ZTSN2cl4sycl6detail5arrayILi1EEE.cl::sycl::detail::array" = type { [1 x i64] }

declare i64 @_Z31__spirv_TaskSequenceCreateINTELIN13task_sequenceIL_Z8block_fniELj1EEEiJiEEmPT_PFT0_DpT1_Ej(ptr addrspace(4), ptr)
; CHECK-LABEL: define internal i64 @_Z31__spirv_TaskSequenceCreateINTELIN13task_sequenceIL_Z8block_fniELj1EEEiJiEEmPT_PFT0_DpT1_Ej
; CHECK-NEXT:   [[ID:%.*]] = call ptr @__create_task_sequence(i64 4)
; CHECK-NEXT:   [[BC:%.*]] = ptrtoint ptr [[ID]] to i64
; CHECK-NEXT:   ret i64 [[BC]]
; CHECK-NEXT: }

declare void @_Z30__spirv_TaskSequenceAsyncINTELIN13task_sequenceIL_Z8block_fniELj1EEEiJiEEvPT_PFT0_DpT1_EjjSB_(ptr addrspace(4), ptr, i64, i32, i32)
; CHECK-LABEL: define internal void @_Z30__spirv_TaskSequenceAsyncINTELIN13task_sequenceIL_Z8block_fniELj1EEEiJiEEvPT_PFT0_DpT1_EjjSB_
; CHECK-SAME:     (ptr addrspace(4) [[TASKSEQ:%.*]], ptr [[ASYNCFUNC:%.*]], i64 {{%.*}}, i32 [[CAPACITY:%.*]], i32 [[ARG:%.*]])
; CHECK-NEXT:   [[CAST:%.*]] = addrspacecast ptr [[ASYNCFUNC]] to ptr addrspace(4)
; CHECK-NEXT:   %block.invoke = call ptr addrspace(4) @_Z30__spirv_TaskSequenceAsyncINTELIN13task_sequenceIL_Z8block_fniELj1EEEiJiEEvPT_PFT0_DpT1_EjjSB_.block_invoke_mapper(ptr addrspace(4) [[CAST]])
; CHECK-NEXT:   %literal = alloca { i32, i32, ptr, i32, ptr }, align 8
; CHECK-NEXT:   %literal.size = getelementptr inbounds { i32, i32, ptr, i32, ptr }, ptr %literal, i32 0, i32 0
; CHECK-NEXT:   store i32 32, ptr %literal.size, align 4
; CHECK-NEXT:   %literal.align = getelementptr inbounds { i32, i32, ptr, i32, ptr }, ptr %literal, i32 0, i32 1
; CHECK-NEXT:   store i32 8, ptr %literal.align, align 4
; CHECK-NEXT:   %literal.invoke = getelementptr inbounds { i32, i32, ptr, i32, ptr }, ptr %literal, i32 0, i32 2
; CHECK-NEXT:   [[CAST1:%.*]] = addrspacecast ptr addrspace(4) %block.invoke to ptr
; CHECK-NEXT:   store ptr [[CAST1]], ptr %literal.invoke, align 8
; CHECK-NEXT:   [[ARG_ADDR:%.*]] = getelementptr { i32, i32, ptr, i32, ptr }, ptr %literal, i32 0, i32 3
; CHECK-NEXT:   store i32 [[ARG]], ptr [[ARG_ADDR]], align 4
; CHECK-NEXT:   [[CAST2:%.*]] = addrspacecast ptr %literal to ptr addrspace(4)
; CHECK-NEXT:   call void @__async(ptr addrspace(4) [[TASKSEQ]], i32 [[CAPACITY]], ptr addrspace(4) %block.invoke, ptr addrspace(4) [[CAST2]])
; CHECK-NEXT:   ret void
; CHECK-NEXT: }


declare i32 @_Z28__spirv_TaskSequenceGetINTELIN13task_sequenceIL_Z8block_fniELj1EEEiJiEET0_PT_PFS7_DpT1_Ej(ptr addrspace(4), ptr, i64, i32)
; CHECK-LABEL: define internal i32 @_Z28__spirv_TaskSequenceGetINTELIN13task_sequenceIL_Z8block_fniELj1EEEiJiEET0_PT_PFS7_DpT1_Ej
; CHECK-SAME:     (ptr addrspace(4) [[TASKSEQ:%.*]], ptr {{.*}}, i64 {{.*}}, i32 {{.*}})
; CHECK-NEXT:   [[CALL:%.*]] = call ptr addrspace(4) @__get(ptr addrspace(4) [[TASKSEQ]], i32 %3)
; CHECK-NEXT:   [[CAST:%.*]] = addrspacecast ptr addrspace(4) [[CALL]] to ptr
; CHECK-NEXT:   [[LOAD:%.*]] = load i32, ptr [[CAST]], align 4
; CHECK-NEXT:   ret i32 [[LOAD]]
; CHECK-NEXT: }

declare void @_Z32__spirv_TaskSequenceReleaseINTELIN13task_sequenceIL_Z8block_fniELj1EEEEvPT_(ptr addrspace(4))
; CHECK-LABEL: define internal void @_Z32__spirv_TaskSequenceReleaseINTELIN13task_sequenceIL_Z8block_fniELj1EEEEvPT_
; CHECK-SAME:     (ptr addrspace(4) [[TASKSEQ:%.*]])
; CHECK-NEXT:   call void @__release_task_sequence(ptr addrspace(4) [[TASKSEQ]])
; CHECK-NEXT:   ret void
; CHECK-NEXT: }

define void @_ZTSZZ4mainENKUlRN2cl4sycl7handlerEE_clES2_EUlNS0_14kernel_handlerEE_(ptr addrspace(1) nocapture %_arg_, ptr nocapture readnone byval(%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range") align 8 %_arg_1, ptr nocapture readnone byval(%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range") align 8 %_arg_2, ptr nocapture readonly byval(%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range") align 8 %_arg_3) !kernel_arg_base_type !1 !arg_type_null_val !2 {
entry:
  %ts = alloca %class.task_sequence, align 8
  %0 = load i64, ptr %_arg_3, align 8
  %add.ptr = getelementptr inbounds i32, ptr addrspace(1) %_arg_, i64 %0
  %1 = addrspacecast ptr %ts to ptr addrspace(4)
  store i32 0, ptr %ts, align 8
  %call = call i64 @_Z31__spirv_TaskSequenceCreateINTELIN13task_sequenceIL_Z8block_fniELj1EEEiJiEEmPT_PFT0_DpT1_Ej(ptr addrspace(4) %1, ptr nonnull @_Z8block_fni)
  %id = getelementptr inbounds %class.task_sequence, ptr %ts, i64 0, i32 1
  store i64 %call, ptr %id, align 8
  %2 = load i32, ptr %ts, align 8
  %inc = add i32 %2, 1
  store i32 %inc, ptr %ts, align 8
  call void @_Z30__spirv_TaskSequenceAsyncINTELIN13task_sequenceIL_Z8block_fniELj1EEEiJiEEvPT_PFT0_DpT1_EjjSB_(ptr addrspace(4) %1, ptr nonnull @_Z8block_fni, i64 %call, i32 10, i32 4)
  %3 = load i32, ptr %ts, align 8
  %dec = add i32 %3, -1
  store i32 %dec, ptr %ts, align 8
  %call.i4 = call i32 @_Z28__spirv_TaskSequenceGetINTELIN13task_sequenceIL_Z8block_fniELj1EEEiJiEET0_PT_PFS7_DpT1_Ej(ptr addrspace(4) %1, ptr nonnull @_Z8block_fni, i64 %call, i32 10)
  store i32 %call.i4, ptr addrspace(1) %add.ptr, align 4
  call void @_Z32__spirv_TaskSequenceReleaseINTELIN13task_sequenceIL_Z8block_fniELj1EEEEvPT_(ptr addrspace(4) %1)
  ret void
}

define internal i32 @_Z8block_fni(i32 %mul) {
entry:
  %mul1 = mul nsw i32 %mul, 7
  %sub = add nsw i32 %mul1, -21
  ret i32 %sub
}

; CHECK: define void @_Z8block_fni._block_invoke_kernel(ptr %0) !block_literal_size
; CHECK-NEXT:   [[ARG_ADDR:%.*]] = getelementptr { i32, i32, ptr, i32, ptr }, ptr %0, i32 0, i32 3
; CHECK-NEXT:   [[ARG:%.*]] = load i32, ptr [[ARG_ADDR]], align 4
; CHECK-NEXT:   [[RET:%.*]] = call i32 @_Z8block_fni(i32 [[ARG]])
; CHECK-NEXT:   [[RET_ADDR_ADDR:%.*]] = getelementptr { i32, i32, ptr, i32, ptr }, ptr %0, i32 0, i32 4
; CHECK-NEXT:   [[RET_ADDR:%.*]] = load ptr, ptr [[RET_ADDR_ADDR]], align 8
; CHECK-NEXT:   store i32 [[RET]], ptr [[RET_ADDR]], align 4
; CHECK-NEXT:   ret void
; CHECK-NEXT: }

; CHECK-LABEL: define internal ptr addrspace(4) @_Z30__spirv_TaskSequenceAsyncINTELIN13task_sequenceIL_Z8block_fniELj1EEEiJiEEvPT_PFT0_DpT1_EjjSB_.block_invoke_mapper
; CHECK: ret ptr addrspace(4) addrspacecast (ptr @_Z8block_fni._block_invoke_kernel to ptr addrspace(4))

; CHECK: declare ptr @__create_task_sequence(i64)
; CHECK: declare void @__release_task_sequence(ptr addrspace(4))
; CHECK: declare ptr addrspace(4) @__get(ptr addrspace(4), i32)
; CHECK: declare void @__async(ptr addrspace(4), i32, ptr addrspace(4), ptr addrspace(4))

!sycl.kernels = !{!0}

!0 = !{ptr @_ZTSZZ4mainENKUlRN2cl4sycl7handlerEE_clES2_EUlNS0_14kernel_handlerEE_}
!1 = !{!"int*", !"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range", !"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range", !"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"}
!2 = !{ptr addrspace(1) null, ptr null, ptr null, ptr null}

; CHECK: !sycl.kernels = !{[[KERNELS:![0-9]+]]}
; CHECK: [[KERNELS]] =
; CHECK-SAME: ptr @_Z8block_fni._block_invoke_kernel

; Generated functions won't contain any debug info, so we only check debug info
; in the original kernel wasn't discarded.
; DEBUGIFY-NOT: WARNING: {{.*}} _ZTSZZ4mainENKUlRN2cl4sycl7handlerEE_clES2_EUlNS0_14kernel_handlerEE_
