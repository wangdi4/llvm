; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -disable-output -whole-program-assume -dtrans-weakalign -debug-only=dtrans-weakalign 2>&1 | FileCheck %s
; RUN: opt < %s -disable-output -whole-program-assume -passes=dtrans-weakalign -debug-only=dtrans-weakalign 2>&1 | FileCheck %s

; Test that weak align gets inhibited when @llvm.assume is used on a case
; involving pointer types.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%__SOADT_EL_class.F = type { i32*, float* }

@glob = internal global i32 zeroinitializer

define void @test01() !dtrans-soatoaos !0 {
  ret void
}

define void @test02() {
  %pti = ptrtoint i32* @glob to i64
  %low = trunc i64 %pti to i8
  %and = and i8 %low, 252
  %icmp = icmp eq i8 %low, %and
  call void @llvm.assume(i1 %icmp)
  ret void
}

define i32 @main() {
  call void @test01()
  call void @test02()
  ret i32 0
}
; CHECK: DTRANS Weak Align: inhibited -- Contains unsupported intrinsic

declare void @llvm.assume(i1)


!0 = !{%__SOADT_EL_class.F zeroinitializer, i32 1}
