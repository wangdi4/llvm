; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; This test verifies that the DTrans analysis marks the structure
; %struct.test01 as System Object since it is used as parameter in a
; dllexport function. The structure %struct.test01.123 must be marked
; as System Object too since it is the return type of a dllexport function.

%struct.test01 = type { i32, i64, i32 }
%struct.test01.123 = type { i32, i64, i32 }
%struct.test01.456 = type { i32, i64, i32 }

; Check that %struct.test01 is marked as System Object since it is used
; in a dllexport function.

; CHECK-LABEL:  LLVMType: %struct.test01 = type { i32, i64, i32 }
; CHECK:  Safety data: Bad casting | Address taken | System object

; CHECK-LABEL: LLVMType: %struct.test01.123 = type { i32, i64, i32 }
; CHECK: Safety data: Bad casting | System object

; CHECK-LABEL: LLVMType: %struct.test01.456 = type { i32, i64, i32 }
; CHECK: Safety data: No issues found

define dllexport %struct.test01.123* @test01_a(%struct.test01* %p) {
  %retval = bitcast %struct.test01* %p to %struct.test01.123*
  ret %struct.test01.123* %retval
}

define void @test01_b(%struct.test01.123* %p) {
  ret void
}

define void @test01_c(%struct.test01.456* %p) {
  ret void
}

define void @test01() {
  %buf = call i8* @malloc(i32 16)
  %p = bitcast i8* %buf to %struct.test01*
  %p2 = call %struct.test01.123* @test01_a(%struct.test01* %p)
  call void @test01_b(%struct.test01.123* %p2)
  call void bitcast (void (%struct.test01.456*)* @test01_c
              to void (%struct.test01*)*) (%struct.test01* %p)
  call void @free(i8* %buf)
  ret void
}

define i32 @main(i32 %argc, i8** %argv) {
  call void @test01()
  ret i32 0
}

declare i8* @malloc(i32)
declare void @free(i8*)
