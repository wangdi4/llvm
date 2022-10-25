; RUN: opt < %s -opaque-pointers -S -whole-program-assume -intel-libirc-allowed -passes=dtrans-weakalign -dtrans-weakalign-heur-override=true 2>&1 | FileCheck %s

; Test that the mallopt call gets inserted at the start of main when
; the safety checks pass, and heuristics are being ignored.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define internal void @test01(ptr "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !2 {
  ret void
}

define i32 @main() {
  %a = alloca i32
  call void @test01(ptr %a)
  ret i32 0
}

; CHECK-LABEL: define i32 @main()
; CHECK-NEXT: call i32 @mallopt(i32 3225, i32 0)
; CHECK-NEXT: %a = alloca i32
; CHECK-NEXT: call void @test01(ptr %a)

!1 = !{i32 0, i32 1}  ; i32*
!2 = distinct !{!1}

!intel.dtrans.types = !{}

