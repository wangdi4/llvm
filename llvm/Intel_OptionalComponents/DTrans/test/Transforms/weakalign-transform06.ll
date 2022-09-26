; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -S -whole-program-assume -dtrans-weakalign 2>&1 | FileCheck %s
; RUN: opt < %s -S -whole-program-assume -passes=dtrans-weakalign 2>&1 | FileCheck %s

; Test that the transformation occurs when there is a safe use of the
; @llvm.assume intrinsic call. This case is considered safe because the
; condition that is used for the @llvm.assume does not involve any
; pointers.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%__SOADT_EL_class.F = type { i32*, float* }

@glob = internal global i32 zeroinitializer

; Dummy function with DTrans SOA-to-AOS attribute to allow weak align
; transform to run.
define void @test01() !dtrans-soatoaos !0 {
  ret void
}

; Function to use llvm.assume intrinsic in a way that should not inhibit the
; weak align transformation.
define void @test02() {
entry:
  %load = load i32, i32* @glob
  br label %top
top:
  %phi = phi i64 [ 0, %entry ], [ %add2, %top ]
  %trunc = trunc i64 %phi to i32
  %add1 = add nuw i32 %trunc, 1
  %icmp = icmp ult i32 %add1, %load
  %xor = xor i1 %icmp, true
  call void @llvm.assume(i1 %xor)
  %add2 = add nuw nsw i64 %phi, 1
  br i1 undef, label %top, label %done

done:
  ret void
}

define i32 @main() {
  call void @test01()
  call void @test02()
  ret i32 0
}
; CHECK-LABEL: define i32 @main()
; CHECK-NEXT: call i32 @mallopt(i32 3225, i32 0)
; CHECK-NEXT: call void @test01()

declare void @llvm.assume(i1)

!0 = !{%__SOADT_EL_class.F zeroinitializer, i32 1}
