; RUN: opt -passes=sycl-kernel-resolve-var-tid-call -S %s | FileCheck %s
; RUN: opt -passes=sycl-kernel-resolve-var-tid-call -enable-debugify -disable-output 2>&1 -S %s | FileCheck %s -check-prefix=DEBUGIFY

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

declare i64 @_Z13get_global_idj(i32)

declare i64 @_Z12get_local_idj(i32)

; Function Attrs: convergent nounwind
define void @test(ptr %a, ptr %b) !kernel_arg_base_type !0 !arg_type_null_val !1 {
entry:
; CHECK: store i64 0, ptr %a
; CHECK-NEXT: store i64 0, ptr %b
  %call = call i64 @_Z13get_global_idj(i32 4)
  %call2 = call i64 @_Z12get_local_idj(i32 3)
  store i64 %call, ptr %a
  store i64 %call2, ptr %b
  ret void
}

!0 = !{!"long*", !"long*"}
!1 = !{ptr null, ptr null}

; DEBUGIFY: WARNING: Missing line 1
; DEBUGIFY: WARNING: Missing line 2
; DEBUGIFY-NOT: WARNING
