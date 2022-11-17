; Check that kernel function pointer in global value is replaced with wrapper
; kernel function pointer and comdat is set to wrapper kernel.

; RUN: opt -dpcpp-kernel-prepare-args -S < %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -dpcpp-kernel-prepare-args -S < %s | FileCheck %s --check-prefixes CHECK
; RUN: opt -passes=dpcpp-kernel-prepare-args -S < %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=dpcpp-kernel-prepare-args -S < %s | FileCheck %s --check-prefixes CHECK

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

$_ZTS13dummy_functorIN14program_info__12program_infoEE = comdat any

@llvm.used = appending global [1 x i8*] [i8* bitcast (void (i8 addrspace(3)*, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }*, i64*, [4 x i64], i8*, {}*)* @_ZTS13dummy_functorIN14program_info__12program_infoEE to i8*)], section "llvm.metadata"

; CHECK: @llvm.used = appending global [1 x i8*] [i8* bitcast (void (i8*, i64*, {}*)* @_ZTS13dummy_functorIN14program_info__12program_infoEE to i8*)], section "llvm.metadata"

; CHECK: define weak_odr void @___ZTS13dummy_functorIN14program_info__12program_infoEE_separated_args(
; CHECK-NOT: comdat

; CHECK: define weak_odr void @_ZTS13dummy_functorIN14program_info__12program_infoEE(
; CHECK-SAME: comdat

; Function Attrs: mustprogress norecurse
declare void @___ZTS13dummy_functorIN14program_info__12program_infoEE_before.AddImplicitArgs() #0

; Function Attrs: mustprogress norecurse
declare void @___ZGVeN16__ZTS13dummy_functorIN14program_info__12program_infoEE_before.AddImplicitArgs() #0

; Function Attrs: mustprogress norecurse
define weak_odr void @_ZTS13dummy_functorIN14program_info__12program_infoEE(i8 addrspace(3)* noalias %pLocalMemBase, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* noalias %pWorkDim, i64* noalias %pWGId, [4 x i64] %BaseGlbId, i8* noalias %pSpecialBuf, {}* noalias %RuntimeHandle) #0 comdat !kernel_arg_addr_space !1 !kernel_arg_access_qual !1 !kernel_arg_type !1 !kernel_arg_base_type !1 !kernel_arg_type_qual !1 !kernel_arg_buffer_location !1 {
entry:
  ret void
}

attributes #0 = { mustprogress norecurse }

!sycl.kernels = !{!0}

!0 = !{void (i8 addrspace(3)*, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }*, i64*, [4 x i64], i8*, {}*)* @_ZTS13dummy_functorIN14program_info__12program_infoEE}
!1 = !{}

; DEBUGIFY-NOT: WARNING
