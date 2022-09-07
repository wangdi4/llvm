; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -whole-program-assume  -dtransanalysis -dtrans-print-allocations -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume  -passes='require<dtransanalysis>' -dtrans-print-allocations -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; This test verifies correct analysis of return instructions.

; Return a properly typed pointer to a structure.
%struct.test01 = type { i32, i32 }
define %struct.test01* @test1(%struct.test01* %p) {
  ret %struct.test01* %p
}

; CHECK: LLVMType: %struct.test01 = type { i32, i32 }
; CHECK: Safety data: No issues found

; Return an instance of a structure.
%struct.test02 = type { i32, i32 }
define %struct.test02 @test2(%struct.test02 %s) {
  ret %struct.test02 %s
}

; CHECK: LLVMType: %struct.test02 = type { i32, i32 }
; CHECK: Safety data: Whole structure reference

; Return an i8* typed pointer to a structure.
%struct.test03 = type { i32, i32 }
define i8* @test3(%struct.test03* %p) {
  %p8 = bitcast %struct.test03* %p to i8*
  ret i8* %p8
}

; CHECK: LLVMType: %struct.test03 = type { i32, i32 }
; CHECK: Safety data: Address taken

; Return an i64 typed pointer to a structure.
%struct.test04 = type { i32, i32 }
define i64 @test4(%struct.test04* %p) {
  %n = ptrtoint %struct.test04* %p to i64
  ret i64 %n
}

; CHECK: LLVMType: %struct.test04 = type { i32, i32 }
; CHECK: Safety data: Address taken

; Return the address of a structure field.
%struct.test05 = type { i32, i32 }
define i32* @test5(%struct.test05* %p) {
  %p1 = getelementptr %struct.test05, %struct.test05* %p, i64 0, i32 1
  ret i32* %p1
}

; CHECK: LLVMType: %struct.test05 = type { i32, i32 }
; CHECK: Safety data: Field address taken return

; Return the address of element zero of a structure.
%struct.test06 = type { i32, i32 }
define i32* @test6(%struct.test06* %p) {
  %p0 = bitcast %struct.test06* %p to i32*
  ret i32* %p0
}

; CHECK: LLVMType: %struct.test06 = type { i32, i32 }
; CHECK: Safety data: Field address taken return
