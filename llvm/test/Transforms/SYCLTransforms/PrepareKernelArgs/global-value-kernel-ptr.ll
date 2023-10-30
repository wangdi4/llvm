; Check that kernel function pointer in global value is replaced with wrapper
; kernel function pointer and comdat is set to wrapper kernel.

; RUN: opt -passes=sycl-kernel-prepare-args -S < %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-prepare-args -S < %s | FileCheck %s --check-prefixes CHECK

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

$foo = comdat any

@llvm.used = appending global [1 x ptr] [ptr @foo], section "llvm.metadata"
@a = global ptr @foo

; CHECK: @llvm.used = appending global [1 x ptr] [ptr @foo], section "llvm.metadata"
; CHECK: @a = global ptr @foo

; CHECK: define weak_odr void @__foo_separated_args(
; CHECK-NOT: comdat

; CHECK: define weak_odr void @foo(
; CHECK-SAME: comdat

; Function Attrs: mustprogress norecurse
declare void @__foo_before.AddImplicitArgs() #0

; Function Attrs: mustprogress norecurse
declare void @___ZGVeN16_foo_before.AddImplicitArgs() #0

; Function Attrs: mustprogress norecurse
define weak_odr void @foo(ptr addrspace(3) noalias %pLocalMemBase, ptr noalias %pWorkDim, ptr noalias %pWGId, [4 x i64] %BaseGlbId, ptr noalias %pSpecialBuf, ptr noalias %RuntimeHandle) #0 comdat !kernel_arg_addr_space !1 !kernel_arg_access_qual !1 !kernel_arg_type !1 !kernel_arg_base_type !1 !kernel_arg_type_qual !1 !kernel_arg_buffer_location !1 {
entry:
  ret void
}

attributes #0 = { mustprogress norecurse }

!sycl.kernels = !{!0}

!0 = !{ptr @foo}
!1 = !{}

; DEBUGIFY-NOT: WARNING
