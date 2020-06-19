; REQUIRES: asserts
; RUN: opt  -whole-program-assume < %s -disable-output -dtrans-aostosoa -debug-only=dtrans-aostosoa 2>&1 | FileCheck %s

; This test verifies that the dtrans aostosoa pass does not try to transform
; structures that do not meet the DTrans analysis safety conditions on the
; structure.
;
; The safety check is straightforward, so it is not necessary to check every
; safety condition individually.
%struct.test01 = type { i32, i64, i32 }
%struct.other01 = type { i32, i32, i32, i32 }
define %struct.other01* @test01() {
  %p = call i8* @malloc(i64 16)
  %p_test = bitcast i8* %p to %struct.test01*

  ; Introduce a bad bitcast.
  %p_test_A = getelementptr %struct.test01, %struct.test01* %p_test, i64 0, i32 0
  %p_other = bitcast i32* %p_test_A to %struct.other01*
  ret %struct.other01* %p_other
}

define i32 @main(i32 %argc, i8** %argv) {
  %res = call %struct.other01* @test01()
  ret i32 0
}

; CHECK: DTRANS-AOSTOSOA: Rejecting -- Unsupported safety data: struct.test01

declare i8* @malloc(i64)

