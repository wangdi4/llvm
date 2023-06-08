; Check comdat is added to new function and dropped from old
; _before.AddImplicitArgs function declaration.

; RUN: opt -passes=sycl-kernel-add-implicit-args %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-add-implicit-args %s -S | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

$_ZTS13dummy_functorIN14program_info__12program_infoEE = comdat any

@llvm.used = appending global [1 x ptr] [ptr @_ZTS13dummy_functorIN14program_info__12program_infoEE], section "llvm.metadata"

; CHECK: declare void @___ZTS13dummy_functorIN14program_info__12program_infoEE_before.AddImplicitArgs() #0
; CHECK-NOT: comdat
; CHECK: define weak_odr void @_ZTS13dummy_functorIN14program_info__12program_infoEE(
; CHECK-SAME: comdat

define weak_odr void @_ZTS13dummy_functorIN14program_info__12program_infoEE() #0 comdat !kernel_arg_addr_space !1 !kernel_arg_access_qual !1 !kernel_arg_type !1 !kernel_arg_base_type !1 !kernel_arg_type_qual !1 !kernel_arg_buffer_location !1 {
entry:
  ret void
}

attributes #0 = { mustprogress norecurse }

!sycl.kernels = !{!0}

!0 = !{ptr @_ZTS13dummy_functorIN14program_info__12program_infoEE}
!1 = !{}

; DEBUGIFY-NOT: WARNING
