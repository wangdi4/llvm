; RUN: opt -opaque-pointers=0 -S -passes=sycl-kernel-handle-taskseq-async %s | FileCheck %s
; RUN: opt -opaque-pointers=0 -S -passes=sycl-kernel-handle-taskseq-async %s -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY

%class.task_sequence = type { i32, i64 }
%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range" = type { %"class._ZTSN2cl4sycl6detail5arrayILi1EEE.cl::sycl::detail::array" }
%"class._ZTSN2cl4sycl6detail5arrayILi1EEE.cl::sycl::detail::array" = type { [1 x i64] }

declare i64 @_Z31__spirv_TaskSequenceCreateINTELIN13task_sequenceIL_Z8block_fniELj1EEEiJiEEmPT_PFT0_DpT1_Ej(%class.task_sequence addrspace(4)*, i32 (i32)*)
; CHECK-LABEL: define internal i64 @_Z31__spirv_TaskSequenceCreateINTELIN13task_sequenceIL_Z8block_fniELj1EEEiJiEEmPT_PFT0_DpT1_Ej
; CHECK-NEXT:   [[ID:%.*]] = call i8* @__create_task_sequence(i64 4)
; CHECK-NEXT:   [[BC:%.*]] = ptrtoint i8* [[ID]] to i64
; CHECK-NEXT:   ret i64 [[BC]]
; CHECK-NEXT: }

declare void @_Z30__spirv_TaskSequenceAsyncINTELIN13task_sequenceIL_Z8block_fniELj1EEEiJiEEvPT_PFT0_DpT1_EjjSB_(%class.task_sequence addrspace(4)*, i32 (i32)*, i64, i32, i32)
; CHECK-LABEL: define internal void @_Z30__spirv_TaskSequenceAsyncINTELIN13task_sequenceIL_Z8block_fniELj1EEEiJiEEvPT_PFT0_DpT1_EjjSB_
; CHECK-SAME:     (%class.task_sequence addrspace(4)* [[TASKSEQ:%.*]], i32 (i32)* [[ASYNCFUNC:%.*]], i64 {{%.*}}, i32 [[CAPACITY:%.*]], i32 [[ARG:%.*]])
; CHECK-NEXT:   [[CAST:%.*]] = addrspacecast i32 (i32)* [[ASYNCFUNC]] to i8 addrspace(4)*
; CHECK-NEXT:   %block.invoke = call i8 addrspace(4)* @_Z30__spirv_TaskSequenceAsyncINTELIN13task_sequenceIL_Z8block_fniELj1EEEiJiEEvPT_PFT0_DpT1_EjjSB_.block_invoke_mapper(i8 addrspace(4)* [[CAST]])
; CHECK-NEXT:   %literal = alloca { i32, i32, i8*, i32, i8* }, align 8
; CHECK-NEXT:   %literal.size = getelementptr inbounds { i32, i32, i8*, i32, i8* }, { i32, i32, i8*, i32, i8* }* %literal, i32 0, i32 0
; CHECK-NEXT:   store i32 32, i32* %literal.size, align 4
; CHECK-NEXT:   %literal.align = getelementptr inbounds { i32, i32, i8*, i32, i8* }, { i32, i32, i8*, i32, i8* }* %literal, i32 0, i32 1
; CHECK-NEXT:   store i32 8, i32* %literal.align, align 4
; CHECK-NEXT:   %literal.invoke = getelementptr inbounds { i32, i32, i8*, i32, i8* }, { i32, i32, i8*, i32, i8* }* %literal, i32 0, i32 2
; CHECK-NEXT:   [[CAST1:%.*]] = addrspacecast i8 addrspace(4)* %block.invoke to i8*
; CHECK-NEXT:   store i8* [[CAST1]], i8** %literal.invoke, align 8
; CHECK-NEXT:   [[ARG_ADDR:%.*]] = getelementptr { i32, i32, i8*, i32, i8* }, { i32, i32, i8*, i32, i8* }* %literal, i32 0, i32 3
; CHECK-NEXT:   store i32 [[ARG]], i32* [[ARG_ADDR]], align 4
; CHECK-NEXT:   [[BC:%.*]] = bitcast %class.task_sequence addrspace(4)* [[TASKSEQ]] to i8 addrspace(4)*
; CHECK-NEXT:   [[CAST2:%.*]] = addrspacecast { i32, i32, i8*, i32, i8* }* %literal to i8 addrspace(4)*
; CHECK-NEXT:   call void @__async(i8 addrspace(4)* [[BC]], i32 [[CAPACITY]], i8 addrspace(4)* %block.invoke, i8 addrspace(4)* [[CAST2]])
; CHECK-NEXT:   ret void
; CHECK-NEXT: }


declare i32 @_Z28__spirv_TaskSequenceGetINTELIN13task_sequenceIL_Z8block_fniELj1EEEiJiEET0_PT_PFS7_DpT1_Ej(%class.task_sequence addrspace(4)*, i32 (i32)*, i64, i32)
; CHECK-LABEL: define internal i32 @_Z28__spirv_TaskSequenceGetINTELIN13task_sequenceIL_Z8block_fniELj1EEEiJiEET0_PT_PFS7_DpT1_Ej
; CHECK-SAME:     (%class.task_sequence addrspace(4)* [[TASKSEQ:%.*]], i32 (i32)* {{.*}}, i64 {{.*}}, i32 {{.*}})
; CHECK-NEXT:   [[BC:%.*]] = bitcast %class.task_sequence addrspace(4)* [[TASKSEQ]] to i8 addrspace(4)*
; CHECK-NEXT:   [[CALL:%.*]] = call i8 addrspace(4)* @__get(i8 addrspace(4)* [[BC]], i32 %3)
; CHECK-NEXT:   [[CAST:%.*]] = addrspacecast i8 addrspace(4)* [[CALL]] to i32*
; CHECK-NEXT:   [[LOAD:%.*]] = load i32, i32* [[CAST]], align 4
; CHECK-NEXT:   ret i32 [[LOAD]]
; CHECK-NEXT: }

declare void @_Z32__spirv_TaskSequenceReleaseINTELIN13task_sequenceIL_Z8block_fniELj1EEEEvPT_(%class.task_sequence addrspace(4)*)
; CHECK-LABEL: define internal void @_Z32__spirv_TaskSequenceReleaseINTELIN13task_sequenceIL_Z8block_fniELj1EEEEvPT_
; CHECK-SAME:     (%class.task_sequence addrspace(4)* [[TASKSEQ:%.*]])
; CHECK-NEXT:   [[BC:%.*]] = bitcast %class.task_sequence addrspace(4)* [[TASKSEQ]] to i8 addrspace(4)*
; CHECK-NEXT:   call void @__release_task_sequence(i8 addrspace(4)* [[BC]])
; CHECK-NEXT:   ret void
; CHECK-NEXT: }

define void @_ZTSZZ4mainENKUlRN2cl4sycl7handlerEE_clES2_EUlNS0_14kernel_handlerEE_(i32 addrspace(1)* nocapture %_arg_, %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* nocapture readnone byval(%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range") align 8 %_arg_1, %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* nocapture readnone byval(%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range") align 8 %_arg_2, %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* nocapture readonly byval(%"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range") align 8 %_arg_3)  {
entry:
  %ts = alloca %class.task_sequence, align 8
  %0 = getelementptr inbounds %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range", %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"* %_arg_3, i64 0, i32 0, i32 0, i64 0
  %1 = load i64, i64* %0, align 8
  %add.ptr = getelementptr inbounds i32, i32 addrspace(1)* %_arg_, i64 %1
  %2 = addrspacecast %class.task_sequence* %ts to %class.task_sequence addrspace(4)*
  %outstanding = getelementptr inbounds %class.task_sequence, %class.task_sequence* %ts, i64 0, i32 0
  store i32 0, i32* %outstanding, align 8
  %call = call i64 @_Z31__spirv_TaskSequenceCreateINTELIN13task_sequenceIL_Z8block_fniELj1EEEiJiEEmPT_PFT0_DpT1_Ej(%class.task_sequence addrspace(4)* %2, i32 (i32)* nonnull @_Z8block_fni)
  %id = getelementptr inbounds %class.task_sequence, %class.task_sequence* %ts, i64 0, i32 1
  store i64 %call, i64* %id, align 8
  %3 = load i32, i32* %outstanding, align 8
  %inc = add i32 %3, 1
  store i32 %inc, i32* %outstanding, align 8
  call void @_Z30__spirv_TaskSequenceAsyncINTELIN13task_sequenceIL_Z8block_fniELj1EEEiJiEEvPT_PFT0_DpT1_EjjSB_(%class.task_sequence addrspace(4)* %2, i32 (i32)* nonnull @_Z8block_fni, i64 %call, i32 10, i32 4)
  %4 = load i32, i32* %outstanding, align 8
  %dec = add i32 %4, -1
  store i32 %dec, i32* %outstanding, align 8
  %call.i4 = call i32 @_Z28__spirv_TaskSequenceGetINTELIN13task_sequenceIL_Z8block_fniELj1EEEiJiEET0_PT_PFS7_DpT1_Ej(%class.task_sequence addrspace(4)* %2, i32 (i32)* nonnull @_Z8block_fni, i64 %call, i32 10)
  store i32 %call.i4, i32 addrspace(1)* %add.ptr, align 4
  call void @_Z32__spirv_TaskSequenceReleaseINTELIN13task_sequenceIL_Z8block_fniELj1EEEEvPT_(%class.task_sequence addrspace(4)* %2)
  ret void
}

define internal i32 @_Z8block_fni(i32 %mul) {
entry:
  %mul1 = mul nsw i32 %mul, 7
  %sub = add nsw i32 %mul1, -21
  ret i32 %sub
}

; CHECK: define void @_Z8block_fni._block_invoke_kernel(i8* %0) !block_literal_size !1 {
; CHECK-NEXT:   %literal = bitcast i8* %0 to { i32, i32, i8*, i32, i8* }*
; CHECK-NEXT:   [[ARG_ADDR:%.*]] = getelementptr { i32, i32, i8*, i32, i8* }, { i32, i32, i8*, i32, i8* }* %literal, i32 0, i32 3
; CHECK-NEXT:   [[ARG:%.*]] = load i32, i32* [[ARG_ADDR]], align 4
; CHECK-NEXT:   [[RET:%.*]] = call i32 @_Z8block_fni(i32 [[ARG]])
; CHECK-NEXT:   [[RET_ADDR_ADDR:%.*]] = getelementptr { i32, i32, i8*, i32, i8* }, { i32, i32, i8*, i32, i8* }* %literal, i32 0, i32 4
; CHECK-NEXT:   [[RET_ADDR:%.*]] = load i8*, i8** [[RET_ADDR_ADDR]], align 8
; CHECK-NEXT:   [[BC:%.*]] = bitcast i8* [[RET_ADDR]] to i32*
; CHECK-NEXT:   store i32 [[RET]], i32* [[BC]], align 4
; CHECK-NEXT:   ret void
; CHECK-NEXT: }

; CHECK-LABEL: define internal i8 addrspace(4)* @_Z30__spirv_TaskSequenceAsyncINTELIN13task_sequenceIL_Z8block_fniELj1EEEiJiEEvPT_PFT0_DpT1_EjjSB_.block_invoke_mapper
; CHECK: ret i8 addrspace(4)* addrspacecast (i8* bitcast (void (i8*)* @_Z8block_fni._block_invoke_kernel to i8*) to i8 addrspace(4)*)

; CHECK: declare i8* @__create_task_sequence(i64)
; CHECK: declare void @__release_task_sequence(i8 addrspace(4)*)
; CHECK: declare i8 addrspace(4)* @__get(i8 addrspace(4)*, i32)
; CHECK: declare void @__async(i8 addrspace(4)*, i32, i8 addrspace(4)*, i8 addrspace(4)*)

!sycl.kernels = !{!0}

!0 = !{void (i32 addrspace(1)*, %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"*, %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"*, %"class._ZTSN2cl4sycl5rangeILi1EEE.cl::sycl::range"*)* @_ZTSZZ4mainENKUlRN2cl4sycl7handlerEE_clES2_EUlNS0_14kernel_handlerEE_}

; CHECK: !sycl.kernels = !{[[KERNELS:![0-9]+]]}
; CHECK: [[KERNELS]] =
; CHECK-SAME: void (i8*)* @_Z8block_fni._block_invoke_kernel

; Generated functions won't contain any debug info, so we only check debug info
; in the original kernel wasn't discarded.
; DEBUGIFY-NOT: WARNING: {{.*}} _ZTSZZ4mainENKUlRN2cl4sycl7handlerEE_clES2_EUlNS0_14kernel_handlerEE_
