; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -disable-output -whole-program-assume -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -dtrans-aostosoa -debug-only=dtrans-aostosoa 2>&1 | FileCheck %s
; RUN: opt < %s -disable-output -whole-program-assume -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -passes=dtrans-aostosoa -debug-only=dtrans-aostosoa 2>&1 | FileCheck %s

; This test verifies that a type allocated in a function called from
; within a loop is disqualified from being transformed
%struct.test01 = type { i32, i64, i32 }
define void @test01() {
  %p1 = call i8* @calloc(i64 4, i64 16)
  %p1_test = bitcast i8* %p1 to %struct.test01*
  ret void
}

define void @test01wrapper() {
  call void @test01()
  ret void
}

define i32 @main(i32 %argc, i8** %argv) {
entry:
  br i1 undef, label %loop_top, label %loop_exit

loop_top:
  call void @test01wrapper()
   br i1 undef, label %loop_top, label %loop_exit

loop_exit:
  ret i32 0
}

; CHECK: DTRANS-AOSTOSOA: Rejecting -- Allocation in loop: struct.test01


declare i8* @calloc(i64, i64)

