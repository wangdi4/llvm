; RUN: opt -passes=dpcpp-kernel-resolve-var-tid-call -S %s | FileCheck %s
; RUN: opt -passes=dpcpp-kernel-resolve-var-tid-call -enable-debugify -disable-output 2>&1 -S %s | FileCheck %s -check-prefix=DEBUGIFY

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

declare i64 @_Z13get_global_idj(i32)

declare i64 @_Z12get_local_idj(i32)

; Function Attrs: convergent nounwind
define void @test(i64* %a, i64* %b) {
entry:
; CHECK: store i64 0, i64* %a
; CHECK-NEXT: store i64 0, i64* %b
  %call = call i64 @_Z13get_global_idj(i32 4)
  %call2 = call i64 @_Z12get_local_idj(i32 3)
  store i64 %call, i64* %a
  store i64 %call2, i64* %b
  ret void
}

; DEBUGIFY: WARNING: Missing line 1
; DEBUGIFY: WARNING: Missing line 2
; DEBUGIFY-NOT: WARNING
