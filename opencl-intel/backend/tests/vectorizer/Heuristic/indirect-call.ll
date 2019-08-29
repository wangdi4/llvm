; RUN: llvm-as %s -o %t.bc
; check that WightedInstCounter doesn't crash on indirect calls
; RUN: %oclopt -winstcounter -verify %t.bc -S -o %t.2.bc

; ModuleID = 'Program'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"
target triple = "i686-pc-win32"

define void @foo(i32 %val, i32 addrspace(1)* %ret) {
  %ptr = inttoptr i32 %val to i32 (i32)*
  %call = call i32 %ptr(i32 %val)
  store i32 %call, i32 addrspace(1)* %ret
  ret void
}

!opencl.kernels = !{!0}

!0 = !{void (i32, i32 addrspace(1)*)* @foo}

