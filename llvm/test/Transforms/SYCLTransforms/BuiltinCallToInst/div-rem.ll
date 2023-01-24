; RUN: opt -passes=sycl-kernel-builtin-call-to-inst -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-builtin-call-to-inst -S %s | FileCheck %s

; This test checks that scalar integer div/rem builtin calls are replaced with
; sdiv/srem/udiv/urem instruction.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define void @test(i32 %a) {
; CHECK: = sdiv i32
; CHECK: = srem i32
; CHECK: = udiv i32
; CHECK: = urem i32
  %_Z4idivii = call i32 @_Z4idivii(i32 0, i32 %a)
  %_Z4iremii = call i32 @_Z4iremii(i32 0, i32 %a)
  %_Z4udivjj = call i32 @_Z4udivjj(i32 0, i32 %a)
  %_Z4uremjj = call i32 @_Z4uremjj(i32 0, i32 %a)
  ret void
}

declare i32 @_Z4idivii(i32, i32)

declare i32 @_Z4iremii(i32, i32)

declare i32 @_Z4udivjj(i32, i32)

declare i32 @_Z4uremjj(i32, i32)

; DEBUGIFY-NOT: WARNING
