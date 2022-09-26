; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; This test validates correct handling of various pointer arithmetic idioms
; involving subtraction with a constant integer by the DTrans analysis.

; Pointer arithmetic to produce an integer, and then integer arithmetic.
; This case is supported because the subtraction result is only required
; to feed a division operation when the pointers have a single level of
; dereferencing.
%struct.test01 = type { i32, i32 }
define void @test01(%struct.test01** %p1, %struct.test01** %p2) {
  ; (%p1 - %p2) - 8
  %t1 = ptrtoint %struct.test01** %p1 to i64
  %t2 = ptrtoint %struct.test01** %p2 to i64
  %delta = sub i64 %t1, %t2
  %offset = sub i64 %delta, 8
  ret void
}
; CHECK: LLVMType: %struct.test01 = type { i32, i32 }
; CHECK: Safety data: No issues found

; This test is equivalent to @test01, except the associativity of the
; subtraction has changed.
%struct.test02 = type { i32, i32 }
define void @test02(%struct.test02** %p1, %struct.test02** %p2) {
  ; (%p1 - 8) - %p2
  %t1 = ptrtoint %struct.test02** %p1 to i64
  %t2 = ptrtoint %struct.test02** %p2 to i64
  %tmp = sub i64 %t1, 8
  %offset = sub i64 %tmp, %t2
  ret void
}
; CHECK: LLVMType: %struct.test02 = type { i32, i32 }
; CHECK: Safety data: No issues found

; This case is marked as "Unhandled use" due to not being a pattern
; that is needed currently. It could be supported in the future.
%struct.test03 = type { i32, i32 }
define void @test03(%struct.test03** %p1, %struct.test03** %p2) {
  ; (%p1 - (%p2 - 8)
  %t1 = ptrtoint %struct.test03** %p1 to i64
  %t2 = ptrtoint %struct.test03** %p2 to i64
  %tmp = sub i64 %t2, 8
  %offset = sub i64 %t1, %tmp
  ret void
}
; CHECK: LLVMType: %struct.test03 = type { i32, i32 }
; CHECK: Safety data: Unhandled use

; This case is set as "Unhandled use" because it is not a pointer-to-pointer,
; and the result of the first pointer subtraction does not feed a division
; instruction. It needs to be "Unhandled use" because the transformations do not
; support locating the constant used for the subtract if the size of structure
; were to be changed.
%struct.test04 = type { i32, i32 }
define void @test04(%struct.test04* %p1, %struct.test04* %p2) {
  ; (%p1 - 8) - %p2
  %t1 = ptrtoint %struct.test04* %p1 to i64
  %t2 = ptrtoint %struct.test04* %p2 to i64
  %tmp = sub i64 %t1, 8
  %offset = sub i64 %tmp, %t2
  ret void
}
; CHECK: LLVMType: %struct.test04 = type { i32, i32 }
; CHECK: Safety data: Unhandled use

; This case is set as "Bad pointer manipulation" because the pointers used in
; the subtract instruction are addresses of structure fields.
%struct.test05 = type { i32, i32 }
define void @test05(%struct.test05* %s1, %struct.test05* %s2) {
  ; (%p1 - 4) - %p2
  %p1 = getelementptr %struct.test05, %struct.test05* %s1, i64 0, i32 1
  %p2 = getelementptr %struct.test05, %struct.test05* %s2, i64 0, i32 1
  %t1 = ptrtoint i32* %p1 to i64
  %t2 = ptrtoint i32* %p2 to i64
  %tmp = sub i64 %t1, 4
  %offset = sub i64 %tmp, %t2
  ret void
}
; CHECK: LLVMType: %struct.test05 = type { i32, i32 }
; CHECK: Safety data: Bad pointer manipulation | Unhandled use
