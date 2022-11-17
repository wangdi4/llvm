; RUN: opt -dpcpp-kernel-enable-tls-globals -dpcpp-kernel-prepare-args -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -dpcpp-kernel-enable-tls-globals -dpcpp-kernel-prepare-args -S %s | FileCheck %s
; RUN: opt -dpcpp-kernel-enable-tls-globals -passes='dpcpp-kernel-prepare-args' -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -dpcpp-kernel-enable-tls-globals -passes='dpcpp-kernel-prepare-args' -S %s | FileCheck %s

%"class.cl::sycl::ext::intel::experimental::task_sequence" = type { i32, i64 }
%"class.cl::sycl::range" = type { %"class.cl::sycl::detail::array" }
%"class.cl::sycl::detail::array" = type { [1 x i64] }

@pLocalMemBase = linkonce_odr thread_local global i8 addrspace(3)* undef, align 8
@pWorkDim = linkonce_odr thread_local global { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* undef, align 8
@pWGId = linkonce_odr thread_local global i64* undef, align 8
@BaseGlbId = linkonce_odr thread_local global [4 x i64] undef, align 16
@pSpecialBuf = linkonce_odr thread_local global i8* undef, align 8
@RuntimeHandle = linkonce_odr thread_local global {}* undef, align 8

; Function Attrs: noinline nounwind optnone
define i32 @_Z8user_sotPiS_i(i32 addrspace(4)* %data1, i32 addrspace(4)* %data2, i32 %N) #0 {
entry:
  ret i32 0
}

; Function Attrs: noinline nounwind optnone
define i32 @_Z11dot_productPiS_i(i32 addrspace(4)* %a, i32 addrspace(4)* %b, i32 %N) #0 {
entry:
  ret i32 0
}

; Function Attrs: noinline nounwind optnone
define i32 @_Z7lib_sotPiS_i(i32 addrspace(4)* %data1, i32 addrspace(4)* %data2, i32 %N) #0 {
entry:
  ret i32 0
}

; Function Attrs: noinline nounwind optnone
define void @_ZTSZZ4mainENKUlRN2cl4sycl7handlerEE_clES2_EUlNS0_14kernel_handlerEE_(i32 addrspace(1)* align 4 %_arg_res_acc, %"class.cl::sycl::range"* byval(%"class.cl::sycl::range") align 8 %_arg_res_acc1, %"class.cl::sycl::range"* byval(%"class.cl::sycl::range") align 8 %_arg_res_acc2, %"class.cl::sycl::range"* byval(%"class.cl::sycl::range") align 8 %_arg_res_acc3, i8 addrspace(1)* align 1 %_arg__specialization_constants_buffer) #0 {
entry:
  ret void
}

; Function Attrs: nounwind
define internal void @"_Z30__spirv_TaskSequenceAsyncINTELPU3AS455class.cl::sycl::ext::intel::experimental::task_sequenceU13block_pointerFvvEliPU3AS4iS3_i"(%"class.cl::sycl::ext::intel::experimental::task_sequence" addrspace(4)* %0, i32 (i32 addrspace(4)*, i32 addrspace(4)*, i32)* %1, i64 %2, i32 %3, i32 addrspace(4)* %4, i32 addrspace(4)* %5, i32 %6) #2 {
  %8 = load { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }*, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }** @pWorkDim, align 8
  %9 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* %8, i32 0, i32 5
  %RuntimeInterface = load {}*, {}** %9, align 1
  %10 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* %8, i32 0, i32 6
  %Block2KernelMapper = load {}*, {}** %10, align 1
  %11 = load {}*, {}** @RuntimeHandle, align 8
  %LocalMemBase = load i8 addrspace(3)*, i8 addrspace(3)** @pLocalMemBase, align 8
  %12 = load { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }*, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }** @pWorkDim, align 8
  %13 = load i64*, i64** @pWGId, align 8
  %14 = load [4 x i64], [4 x i64]* @BaseGlbId, align 8
  %15 = load i8*, i8** @pSpecialBuf, align 8
  %16 = load {}*, {}** @RuntimeHandle, align 8
  %17 = addrspacecast i32 (i32 addrspace(4)*, i32 addrspace(4)*, i32)* %1 to i8 addrspace(4)*
  %18 = load i8 addrspace(3)*, i8 addrspace(3)** @pLocalMemBase, align 8
  %"pLocalMem__Z30__spirv_TaskSequenceAsyncINTELPU3AS455class.cl::sycl::ext::intel::experimental::task_sequenceU13block_pointerFvvEliPU3AS4iS3_i.block_invoke_mapper" = getelementptr i8, i8 addrspace(3)* %18, i32 0
  store i8 addrspace(3)* %"pLocalMem__Z30__spirv_TaskSequenceAsyncINTELPU3AS455class.cl::sycl::ext::intel::experimental::task_sequenceU13block_pointerFvvEliPU3AS4iS3_i.block_invoke_mapper", i8 addrspace(3)** @pLocalMemBase, align 8
  %LocalMemBase.i = load i8 addrspace(3)*, i8 addrspace(3)** @pLocalMemBase, align 8
  %19 = load { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }*, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }** @pWorkDim, align 8
  %20 = load i64*, i64** @pWGId, align 8
  %21 = load [4 x i64], [4 x i64]* @BaseGlbId, align 8
  %22 = load i8*, i8** @pSpecialBuf, align 8
  %23 = load {}*, {}** @RuntimeHandle, align 8
  %24 = ptrtoint i8 addrspace(4)* %17 to i64
  %25 = icmp eq i64 %24, ptrtoint (i32 (i32 addrspace(4)*, i32 addrspace(4)*, i32)* @_Z7lib_sotPiS_i to i64)
  ; CHECK: select i1 %25, void (i8*)* bitcast (void (i8*, i64*, {}*)* @_Z7lib_sotPiS_i._block_invoke_kernel to void (i8*)*), void (i8*)* bitcast (void (i8*, i64*, {}*)* @_Z8user_sotPiS_i._block_invoke_kernel to void (i8*)*)
  %26 = select i1 %25, void (i8*)* @_Z7lib_sotPiS_i._block_invoke_kernel, void (i8*)* @_Z8user_sotPiS_i._block_invoke_kernel
  %27 = icmp eq i64 %24, ptrtoint (i32 (i32 addrspace(4)*, i32 addrspace(4)*, i32)* @_Z11dot_productPiS_i to i64)
  ; CHECK: select i1 %27, void (i8*)* bitcast (void (i8*, i64*, {}*)* @_Z11dot_productPiS_i._block_invoke_kernel to void (i8*)*), void (i8*)* %26
  %28 = select i1 %27, void (i8*)* @_Z11dot_productPiS_i._block_invoke_kernel, void (i8*)* %26
  %29 = addrspacecast void (i8*)* %28 to i8 addrspace(4)*
  store i8 addrspace(3)* %18, i8 addrspace(3)** @pLocalMemBase, align 8
  %literal = alloca { i32, i32, i8*, i32 addrspace(4)*, i32 addrspace(4)*, i32, i8* }, align 8
  %literal.size = getelementptr inbounds { i32, i32, i8*, i32 addrspace(4)*, i32 addrspace(4)*, i32, i8* }, { i32, i32, i8*, i32 addrspace(4)*, i32 addrspace(4)*, i32, i8* }* %literal, i32 0, i32 0
  store i32 48, i32* %literal.size, align 4
  %literal.align = getelementptr inbounds { i32, i32, i8*, i32 addrspace(4)*, i32 addrspace(4)*, i32, i8* }, { i32, i32, i8*, i32 addrspace(4)*, i32 addrspace(4)*, i32, i8* }* %literal, i32 0, i32 1
  store i32 8, i32* %literal.align, align 4
  %literal.invoke = getelementptr inbounds { i32, i32, i8*, i32 addrspace(4)*, i32 addrspace(4)*, i32, i8* }, { i32, i32, i8*, i32 addrspace(4)*, i32 addrspace(4)*, i32, i8* }* %literal, i32 0, i32 2
  %30 = addrspacecast i8 addrspace(4)* %29 to i8*
  store i8* %30, i8** %literal.invoke, align 8
  %31 = getelementptr { i32, i32, i8*, i32 addrspace(4)*, i32 addrspace(4)*, i32, i8* }, { i32, i32, i8*, i32 addrspace(4)*, i32 addrspace(4)*, i32, i8* }* %literal, i32 0, i32 3
  store i32 addrspace(4)* %4, i32 addrspace(4)** %31, align 8
  %32 = getelementptr { i32, i32, i8*, i32 addrspace(4)*, i32 addrspace(4)*, i32, i8* }, { i32, i32, i8*, i32 addrspace(4)*, i32 addrspace(4)*, i32, i8* }* %literal, i32 0, i32 4
  store i32 addrspace(4)* %5, i32 addrspace(4)** %32, align 8
  %33 = getelementptr { i32, i32, i8*, i32 addrspace(4)*, i32 addrspace(4)*, i32, i8* }, { i32, i32, i8*, i32 addrspace(4)*, i32 addrspace(4)*, i32, i8* }* %literal, i32 0, i32 5
  store i32 %6, i32* %33, align 4
  %34 = bitcast %"class.cl::sycl::ext::intel::experimental::task_sequence" addrspace(4)* %0 to i8 addrspace(4)*
  %35 = addrspacecast { i32, i32, i8*, i32 addrspace(4)*, i32 addrspace(4)*, i32, i8* }* %literal to i8 addrspace(4)*
  %36 = addrspacecast {}* %RuntimeInterface to i8 addrspace(4)*
  %37 = addrspacecast {}* %Block2KernelMapper to i8 addrspace(4)*
  %38 = addrspacecast {}* %11 to i8 addrspace(4)*
  ret void
}

define void @_Z8user_sotPiS_i._block_invoke_kernel(i8* %0) {
  ret void
}

define void @_Z7lib_sotPiS_i._block_invoke_kernel(i8* %0) {
  ret void
}

define void @_Z11dot_productPiS_i._block_invoke_kernel(i8* %0) {
  ret void
}

attributes #0 = { noinline nounwind optnone "prefer-vector-width"="512" }
attributes #2 = { nounwind "prefer-vector-width"="512" }

!sycl.kernels = !{!0}
!0 = !{void (i32 addrspace(1)*, %"class.cl::sycl::range"*, %"class.cl::sycl::range"*, %"class.cl::sycl::range"*, i8 addrspace(1)*)* @_ZTSZZ4mainENKUlRN2cl4sycl7handlerEE_clES2_EUlNS0_14kernel_handlerEE_, void (i8*)* @_Z8user_sotPiS_i._block_invoke_kernel, void (i8*)* @_Z7lib_sotPiS_i._block_invoke_kernel, void (i8*)* @_Z11dot_productPiS_i._block_invoke_kernel}

; DEBUGIFY-NOT: WARNING
; DEBUGIFY-COUNT-180: WARNING: Instruction with empty DebugLoc in function {{.*}}
; DEBUGIFY-NOT: WARNING
