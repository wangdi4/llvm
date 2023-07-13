; RUN: opt < %s -S -whole-program-assume -intel-libirc-allowed -passes=dtrans-weakalign 2>&1 | FileCheck %s

; Test that the transformation occurs when there is a safe use of the
; @llvm.assume intrinsic call. This case is considered safe because the
; condition that is used for the @llvm.assume does involve only
; pointers that are returned from memory allocation calls.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%__SOADT_EL_class.F = type { ptr, ptr }

; Dummy function with DTrans SOA-to-AOS attribute to allow weak align
; transform to run.
define void @test01() !dtrans-soatoaos !0 {
  ret void
}

define ptr @alloc1(i64 %s) {
  %a1 = call noalias ptr @_Znwm(i64 noundef %s)
  ret ptr %a1
}

define ptr @alloc2(i64 %s) {
  unreachable
}

; Function to use llvm.assume intrinsic in a way that should not inhibit the
; weak align transformation.
define void @test02() {
entry:
  br i1 undef, label %bb2, label %dummy

bb2:
  %p1 = call ptr @alloc1(i64 8)
  br label %bb3

dummy: 
  %p2 = call ptr @alloc2(i64 8)
  br label %bb3

bb3:
  %phi = phi ptr [ %p1, %bb2 ], [ %p2, %dummy ]
  %ic = icmp ne ptr %phi, null
  call void @llvm.assume(i1 %ic)
  br label %done

done:
  ret void
}

declare ptr @_Znwm(i64)

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
