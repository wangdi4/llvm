; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt  %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt  %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; This test validates correct handling of pointer alignment checking idioms
; by the DTrans analysis.

; Check for 8-byte alignment
%struct.test01 = type { i32, i32 }
define void @test01(%struct.test01* %p) {
  %t1 = ptrtoint %struct.test01* %p to i64
  %t2 = and i64 %t1, 7
  %cmp = icmp eq i64 %t2, 0
  ret void
}

; CHECK: LLVMType: %struct.test01 = type { i32, i32 }
; CHECK: Safety data: No issues found

; Check for 8-byte alignment with an extraneous bit set
%struct.test02 = type { i32, i32 }
define void @test02(%struct.test02* %p) {
  %t1 = ptrtoint %struct.test02* %p to i64
  %t2 = or i64 %t1, 8
  %t3 = and i64 %t2, 7
  %cmp = icmp eq i64 %t3, 0
  ret void
}

; CHECK: LLVMType: %struct.test02 = type { i32, i32 }
; CHECK: Safety data: No issues found

; Check for variable bitmask -- this may be OK, but we don't need it so it's
; easier to exclude it and not worry about unintended consequences.
%struct.test03 = type { i32, i32 }
define void @test03(%struct.test03* %p, i64 %mask) {
  %t1 = ptrtoint %struct.test03* %p to i64
  %t2 = or i64 %t1, 8
  %t3 = and i64 %t2, %mask
  %cmp = icmp eq i64 %t3, 0
  ret void
}

; CHECK: LLVMType: %struct.test03 = type { i32, i32 }
; CHECK: Safety data: Unhandled use

; Check for comparison of two masked pointers
%struct.test04 = type { i32, i32 }
define void @test04(%struct.test04* %p1, %struct.test04 *%p2) {
  %t1 = ptrtoint %struct.test04* %p1 to i64
  %t2 = or i64 %t1, 8
  %t3 = and i64 %t2, 7
  %t4 = ptrtoint %struct.test04* %p2 to i64
  %t5 = or i64 %t4, 8
  %t6 = and i64 %t5, 7
  %cmp = icmp eq i64 %t3, %t6
  ret void
}

; CHECK: LLVMType: %struct.test04 = type { i32, i32 }
; CHECK: Safety data: Unhandled use
