; REQUIRES: asserts
; RUN: opt  -whole-program-assume < %s -disable-output -dtrans-aostosoa -debug-only=dtrans-aostosoa 2>&1 | FileCheck %s
; RUN: opt  -whole-program-assume < %s -disable-output -passes=dtrans-aostosoa -debug-only=dtrans-aostosoa 2>&1 | FileCheck %s

; This test verifies that a type allocated within a loop is
; disqualified from being transformed
%struct.test01 = type { i32, i64, i32 }
define void @test01() {
entry:
  br i1 undef, label %loop_top, label %loop_exit

loop_top:
  %p1 = call i8* @calloc(i64 4, i64 16)
  %p1_test = bitcast i8* %p1 to %struct.test01*
   br i1 undef, label %loop_top, label %loop_exit
loop_exit:
  ret void
}

define i32 @main(i32 %argc, i8** %argv) {
  call void @test01()
  ret i32 0
}

; CHECK: DTRANS-AOSTOSOA: Rejecting -- Allocation in loop: struct.test01


declare i8* @calloc(i64, i64)

