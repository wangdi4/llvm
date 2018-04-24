; RUN: opt < %s -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; This test validates correct handling of various pointer arithmetic idioms
; by the DTrans analysis.

; Subtracting two pointers of like types yields a safe offset.
%struct.test01 = type { i32, i32 }
define void @test01(%struct.test01* %p1, %struct.test01* %p2) {
  %t1 = ptrtoint %struct.test01* %p1 to i64
  %t2 = ptrtoint %struct.test01* %p2 to i64
  %offset = sub i64 %t1, %t2
  ret void
}

; CHECK: LLVMType: %struct.test01 = type { i32, i32 }
; CHECK: Safety data: No issues found

; Subtracting two pointers of unlike types is an unhandled use.
%struct.test02.a = type { i32, i32 }
%struct.test02.b = type { i32, i32, i32 }
define void @test02(%struct.test02.a* %p1, %struct.test02.b* %p2) {
  %t1 = ptrtoint %struct.test02.a* %p1 to i64
  %t2 = ptrtoint %struct.test02.b* %p2 to i64
  %offset = sub i64 %t1, %t2
  ret void
}

; CHECK: LLVMType: %struct.test02.a = type { i32, i32 }
; CHECK: Safety data: Unhandled use
; CHECK: LLVMType: %struct.test02.b = type { i32, i32, i32 }
; CHECK: Safety data: Unhandled use

; Subtracting a scalar from a pointer is an unhandled use.
%struct.test03 = type { i32, i32 }
define void @test03(%struct.test03* %p, i64 %n) {
  %t = ptrtoint %struct.test03* %p to i64
  %offset = sub i64 %t, %n
  ret void
}

; CHECK: LLVMType: %struct.test03 = type { i32, i32 }
; CHECK: Safety data: Unhandled use

; Subtracting a pointer from a scalar is an unhandled use.
%struct.test04 = type { i32, i32 }
define void @test04(%struct.test04* %p, i64 %n) {
  %t = ptrtoint %struct.test04* %p to i64
  %offset = sub i64 %n, %t
  ret void
}

; CHECK: LLVMType: %struct.test04 = type { i32, i32 }
; CHECK: Safety data: Unhandled use

; Subtracting a pointers to elements is bad pointer manipulation.
%struct.test05 = type { i32, i32, i32, i32 }
define void @test05(%struct.test05* %p) {
  %p1 = getelementptr %struct.test05, %struct.test05* %p, i64 0, i32 1
  %p2 = getelementptr %struct.test05, %struct.test05* %p, i64 0, i32 3
  %t1 = ptrtoint i32* %p1 to i64
  %t2 = ptrtoint i32* %p2 to i64
  %offset = sub i64 %t2, %t1
  ret void
}

; CHECK: LLVMType: %struct.test05 = type { i32, i32, i32, i32 }
; CHECK: Safety data: Bad pointer manipulation
