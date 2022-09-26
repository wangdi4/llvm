; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -disable-output -whole-program-assume -dtrans-weakalign -dtrans-weakalign-heur-override=false -debug-only=dtrans-weakalign 2>&1 | FileCheck %s
; RUN: opt < %s -disable-output -whole-program-assume -passes=dtrans-weakalign -dtrans-weakalign-heur-override=false -debug-only=dtrans-weakalign 2>&1 | FileCheck %s

; Test that the weak alignment transform is enabled when the heuristic
; passes, and there are no safety issues found.

; CHECK: DTRANS Weak Align: enabled

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%__SOADT_class.F = type { %__SOADT_AR_struct.Arr*, i64 }
%__SOADT_AR_struct.Arr = type { i32, [4 x i8], %__SOADT_EL_class.F*, i32, [4 x i8] }
%__SOADT_EL_class.F = type { i32*, float* }

define internal void @test01() !dtrans-soatoaos !0 {
  ret void
}

define i32 @main() {
  call void @test01()
  ret i32 0
}

!0 = !{%__SOADT_EL_class.F zeroinitializer, i32 1}
