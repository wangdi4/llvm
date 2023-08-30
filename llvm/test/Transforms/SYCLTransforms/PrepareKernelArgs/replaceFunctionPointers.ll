; RUN: opt -passes='sycl-kernel-prepare-args' -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes='sycl-kernel-prepare-args' -S %s | FileCheck %s

%"class.cl::sycl::ext::intel::experimental::task_sequence" = type { i32, i64 }
%"class.cl::sycl::range" = type { %"class.cl::sycl::detail::array" }
%"class.cl::sycl::detail::array" = type { [1 x i64] }

@__pLocalMemBase = linkonce_odr thread_local global ptr addrspace(3) undef, align 8
@__pWorkDim = linkonce_odr thread_local global ptr undef, align 8
@__pWGId = linkonce_odr thread_local global ptr undef, align 8
@__BaseGlbId = linkonce_odr thread_local global [4 x i64] undef, align 16
@__pSpecialBuf = linkonce_odr thread_local global ptr undef, align 8
@__RuntimeHandle = linkonce_odr thread_local global ptr undef, align 8

; Function Attrs: noinline nounwind optnone
define i32 @_Z8user_sotPiS_i(ptr addrspace(4) %data1, ptr addrspace(4) %data2, i32 %N) #0 {
entry:
  ret i32 0
}

; Function Attrs: noinline nounwind optnone
define i32 @_Z11dot_productPiS_i(ptr addrspace(4) %a, ptr addrspace(4) %b, i32 %N) #0 {
entry:
  ret i32 0
}

; Function Attrs: noinline nounwind optnone
define i32 @_Z7lib_sotPiS_i(ptr addrspace(4) %data1, ptr addrspace(4) %data2, i32 %N) #0 {
entry:
  ret i32 0
}

; Function Attrs: noinline nounwind optnone
define void @_ZTSZZ4mainENKUlRN2cl4sycl7handlerEE_clES2_EUlNS0_14kernel_handlerEE_(ptr addrspace(1) align 4 %_arg_res_acc, ptr byval(%"class.cl::sycl::range") align 8 %_arg_res_acc1, ptr byval(%"class.cl::sycl::range") align 8 %_arg_res_acc2, ptr byval(%"class.cl::sycl::range") align 8 %_arg_res_acc3, ptr addrspace(1) align 1 %_arg__specialization_constants_buffer) #0 !kernel_arg_base_type !1 !arg_type_null_val !2 {
entry:
  ret void
}

; Function Attrs: nounwind
define internal void @"_Z30__spirv_TaskSequenceAsyncINTELPU3AS455class.cl::sycl::ext::intel::experimental::task_sequenceU13block_pointerFvvEliPU3AS4iS3_i"(ptr addrspace(4) %0, ptr %1, i64 %2, i32 %3, ptr addrspace(4) %4, ptr addrspace(4) %5, i32 %6) #2 {
  %8 = load ptr, ptr @__pWorkDim, align 8
  %9 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], ptr, ptr, [3 x i64], [2 x [3 x i64]], [3 x i64] }, ptr %8, i32 0, i32 5
  %RuntimeInterface = load ptr, ptr %9, align 1
  %10 = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], ptr, ptr, [3 x i64], [2 x [3 x i64]], [3 x i64] }, ptr %8, i32 0, i32 6
  %Block2KernelMapper = load ptr, ptr %10, align 1
  %11 = load ptr, ptr @__RuntimeHandle, align 8
  %LocalMemBase = load ptr addrspace(3), ptr @__pLocalMemBase, align 8
  %12 = load ptr, ptr @__pWorkDim, align 8
  %13 = load ptr, ptr @__pWGId, align 8
  %14 = load [4 x i64], ptr @__BaseGlbId, align 8
  %15 = load ptr, ptr @__pSpecialBuf, align 8
  %16 = load ptr, ptr @__RuntimeHandle, align 8
  %17 = addrspacecast ptr %1 to ptr addrspace(4)
  %18 = load ptr addrspace(3), ptr @__pLocalMemBase, align 8
  store ptr addrspace(3) %18, ptr @__pLocalMemBase, align 8
  %LocalMemBase.i = load ptr addrspace(3), ptr @__pLocalMemBase, align 8
  %19 = load ptr, ptr @__pWorkDim, align 8
  %20 = load ptr, ptr @__pWGId, align 8
  %21 = load [4 x i64], ptr @__BaseGlbId, align 8
  %22 = load ptr, ptr @__pSpecialBuf, align 8
  %23 = load ptr, ptr @__RuntimeHandle, align 8
  %24 = ptrtoint ptr addrspace(4) %17 to i64
  %25 = icmp eq i64 %24, ptrtoint (ptr @_Z7lib_sotPiS_i to i64)
  ; CHECK: select i1 %25, ptr @_Z7lib_sotPiS_i._block_invoke_kernel, ptr @_Z8user_sotPiS_i._block_invoke_kernel
  %26 = select i1 %25, ptr @_Z7lib_sotPiS_i._block_invoke_kernel, ptr @_Z8user_sotPiS_i._block_invoke_kernel
  %27 = icmp eq i64 %24, ptrtoint (ptr @_Z11dot_productPiS_i to i64)
  ; CHECK: select i1 %27, ptr @_Z11dot_productPiS_i._block_invoke_kernel, ptr %26
  %28 = select i1 %27, ptr @_Z11dot_productPiS_i._block_invoke_kernel, ptr %26
  %29 = addrspacecast ptr %28 to ptr addrspace(4)
  store ptr addrspace(3) %18, ptr @__pLocalMemBase, align 8
  %literal = alloca { i32, i32, ptr, ptr addrspace(4), ptr addrspace(4), i32, ptr }, align 8
  %literal.size = getelementptr inbounds { i32, i32, ptr, ptr addrspace(4), ptr addrspace(4), i32, ptr }, ptr %literal, i32 0, i32 0
  store i32 48, ptr %literal.size, align 4
  %literal.align = getelementptr inbounds { i32, i32, ptr, ptr addrspace(4), ptr addrspace(4), i32, ptr }, ptr %literal, i32 0, i32 1
  store i32 8, ptr %literal.align, align 4
  %literal.invoke = getelementptr inbounds { i32, i32, ptr, ptr addrspace(4), ptr addrspace(4), i32, ptr }, ptr %literal, i32 0, i32 2
  %30 = addrspacecast ptr addrspace(4) %29 to ptr
  store ptr %30, ptr %literal.invoke, align 8
  %31 = getelementptr { i32, i32, ptr, ptr addrspace(4), ptr addrspace(4), i32, ptr }, ptr %literal, i32 0, i32 3
  store ptr addrspace(4) %4, ptr %31, align 8
  %32 = getelementptr { i32, i32, ptr, ptr addrspace(4), ptr addrspace(4), i32, ptr }, ptr %literal, i32 0, i32 4
  store ptr addrspace(4) %5, ptr %32, align 8
  %33 = getelementptr { i32, i32, ptr, ptr addrspace(4), ptr addrspace(4), i32, ptr }, ptr %literal, i32 0, i32 5
  store i32 %6, ptr %33, align 4
  %34 = addrspacecast ptr %literal to ptr addrspace(4)
  %35 = addrspacecast ptr %RuntimeInterface to ptr addrspace(4)
  %36 = addrspacecast ptr %Block2KernelMapper to ptr addrspace(4)
  %37 = addrspacecast ptr %11 to ptr addrspace(4)
  ret void
}

define void @_Z8user_sotPiS_i._block_invoke_kernel(ptr %0) !kernel_arg_base_type !3 !arg_type_null_val !4 {
  ret void
}

define void @_Z7lib_sotPiS_i._block_invoke_kernel(ptr %0) !kernel_arg_base_type !3 !arg_type_null_val !4 {
  ret void
}

define void @_Z11dot_productPiS_i._block_invoke_kernel(ptr %0) !kernel_arg_base_type !3 !arg_type_null_val !4 {
  ret void
}

attributes #0 = { noinline nounwind optnone "prefer-vector-width"="512" }
attributes #2 = { nounwind "prefer-vector-width"="512" }

!sycl.kernels = !{!0}
!0 = !{ptr @_ZTSZZ4mainENKUlRN2cl4sycl7handlerEE_clES2_EUlNS0_14kernel_handlerEE_, ptr @_Z8user_sotPiS_i._block_invoke_kernel, ptr @_Z7lib_sotPiS_i._block_invoke_kernel, ptr @_Z11dot_productPiS_i._block_invoke_kernel}
!1 = !{!"int*", !"class.cl::sycl::range", !"class.cl::sycl::range", !"class.cl::sycl::range", !"char*"}
!2 = !{ptr addrspace(1) null, ptr null, ptr null, ptr null, ptr addrspace(1) null}
!3 = !{!"char*"}
!4 = !{ptr null}

; DEBUGIFY-NOT: WARNING
; DEBUGIFY-COUNT-144: WARNING: Instruction with empty DebugLoc in function {{.*}}
; DEBUGIFY-NOT: WARNING
