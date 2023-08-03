; This test verifies that GlobalOpt is not triggered since types are
; not same.

; RUN: opt -S -passes=globalopt %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-win32-msvc-elf"

%"class.kernel" = type { [2 x i64] }
%"class.anon" = type { [2 x double] }

@ArgShadow = internal addrspace(3) global %"class.kernel" undef, align 16
@WGCopy.1 = internal addrspace(3) global %class.anon undef, align 8
@WGCopy = internal addrspace(3) global ptr addrspace(4) undef, align 8

; CHECK: @WGCopy.1
define void @foo() {
entry:
  store i64 ptrtoint (ptr addrspace(4) addrspacecast (ptr addrspace(3) @ArgShadow to ptr addrspace(4)) to i64), ptr addrspace(3) bitcast (ptr addrspace(3) @WGCopy.1 to ptr addrspace(3)), align 8
  %agg.tmp.sroa.0.0.copyload1 = load i32, ptr addrspace(3) bitcast (ptr addrspace(3) @WGCopy.1 to ptr addrspace(3)), align 8
  ret void
}
