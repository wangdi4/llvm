; RUN: opt -passes=sycl-kernel-prepare-args -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-prepare-args -S %s | FileCheck %s --check-prefixes CHECK

; Check that PrepareKernelArgsPass can handle kernel argument of
; pointer-to-pointer type.
; IR is extracted from test llvm_test_suite_sycl/atomicref_assignment_atomic64

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

%"class.sycl::_V1::id" = type { %"class.sycl::_V1::detail::array" }
%"class.sycl::_V1::detail::array" = type { [1 x i64] }

; CHECK: define void @_ZTS17assignment_kernelIN4sycl3_V110atomic_refELNS1_6access13address_spaceE1EPcE(ptr noalias %UniformArgs, ptr noalias %pWGId, ptr noalias %RuntimeHandle)

define void @_ZTS17assignment_kernelIN4sycl3_V110atomic_refELNS1_6access13address_spaceE1EPcE(ptr addrspace(1) noalias align 8 %_arg_st, ptr noalias nocapture readonly byval(%"class.sycl::_V1::id") align 8 %_arg_st3, ptr addrspace(3) noalias %pLocalMemBase, ptr noalias %pWorkDim, ptr noalias %pWGId, [4 x i64] %BaseGlbId, ptr noalias %pSpecialBuf, ptr noalias %RuntimeHandle) !kernel_arg_base_type !1 {
entry:
  ret void
}

!sycl.kernels = !{!0}

!0 = !{ptr @_ZTS17assignment_kernelIN4sycl3_V110atomic_refELNS1_6access13address_spaceE1EPcE}
!1 = !{!"char**", !"class.sycl::_V1::id"}

; DEBUGIFY-COUNT-32: WARNING: Instruction with empty DebugLoc in function _ZTS17assignment_kernelIN4sycl3_V110atomic_refELNS1_6access13address_spaceE1EPcE
; DEBUGIFY-NOT: WARNING
