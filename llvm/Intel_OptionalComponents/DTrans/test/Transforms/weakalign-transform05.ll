; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -S -whole-program-assume -dtrans-weakalign 2>&1 | FileCheck %s
; RUN: opt < %s -S -whole-program-assume -passes=dtrans-weakalign 2>&1 | FileCheck %s

; Test that the transformation occurs when there is a safe use of the
; @llvm.assume intrinsic call for checking that a value is non-null.

; The following pattern should be considered safe:
;  %cmp = icmp eq i32* %x, null
;  %xor = xor i1 %cmp, true
;  call void @llvm.assume(i1 %xor)

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%__SOADT_class.F = type { %__SOADT_AR_struct.Arr*, i64 }
%__SOADT_AR_struct.Arr = type { i32, [4 x i8], %__SOADT_EL_class.F*, i32, [4 x i8] }
%__SOADT_EL_class.F = type { i32*, float* }

@gVar = internal global i32 zeroinitializer

define internal i32* @getAddr() {
  ret i32* @gVar
}

define internal void @test01() !dtrans-soatoaos !0 {
  %x = call i32* @getAddr()
  %cmp = icmp eq i32* %x, null
  %xor =   xor i1 %cmp, true
  call void @llvm.assume(i1 %xor)

  ; Test with reversed operands
  %cmp2 = icmp eq i32* null, %x
  %xor2 =   xor i1 true, %cmp2
  call void @llvm.assume(i1 %xor2)
  ret void
}

define i32 @main() {
  call void @test01()
  ret i32 0
}

declare void @llvm.assume(i1)

; CHECK-LABEL: define i32 @main()
; CHECK-NEXT: call i32 @mallopt(i32 3225, i32 0)
; CHECK-NEXT: call void @test01()

!0 = !{%__SOADT_EL_class.F zeroinitializer, i32 1}
