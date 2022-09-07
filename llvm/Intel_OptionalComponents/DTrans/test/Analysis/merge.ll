; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; Select safely merges pointers.
%struct.test01 = type { i32, i32 }
define void @test01(%struct.test01* %p1, %struct.test01* %p2) {
  %p3 = select i1 undef, %struct.test01* %p1, %struct.test01* %p2
  ret void
}

; CHECK: LLVMType: %struct.test01 = type { i32, i32 }
; CHECK: Safety data: No issues found

; Select safely merges pointers after bitcast.
%struct.test02 = type { i32, i32 }
define void @test02(%struct.test02* %p1, %struct.test02* %p2) {
  %p1b = bitcast %struct.test02* %p1 to i8*
  %p2b = bitcast %struct.test02* %p2 to i8*
  %p3 = select i1 undef, i8* %p1b, i8* %p2b
  ret void
}

; CHECK: LLVMType: %struct.test02 = type { i32, i32 }
; CHECK: Safety data: No issues found

; PHI node safely merges pointers.
%struct.test03 = type { i32, i32 }
define void @test03(%struct.test03* %p1, %struct.test03* %p2) {
entry:
  %p1b = bitcast %struct.test03* %p1 to i8*
  br i1 undef, label %other, label %merge

other:
  %p2b = bitcast %struct.test03* %p2 to i8*
  br label %merge

merge:
  %p3 = phi i8* [%p1b, %entry], [%p2b, %other]
  ret void
}

; CHECK: LLVMType: %struct.test03 = type { i32, i32 }
; CHECK: Safety data: No issues found

; Select creates an ambiguous pointer.
%struct.test04.a = type { i32, i32 }
%struct.test04.b = type { i64, i64 }
define void @test04(%struct.test04.a** %p1, %struct.test04.b** %p2) {
  %p1n = bitcast %struct.test04.a** %p1 to i64*
  %p2n = bitcast %struct.test04.b** %p2 to i64*
  %p3n = select i1 undef, i64* %p1n, i64* %p2n
  ret void
}

; CHECK: LLVMType: %struct.test04.a = type { i32, i32 }
; CHECK: Safety data: Unsafe pointer merge
; CHECK: LLVMType: %struct.test04.b = type { i64, i64 }
; CHECK: Safety data: Unsafe pointer merge

; PHI node creates an ambiguous pointer.
%struct.test05.a = type { i32, i32 }
%struct.test05.b = type { i64, i64 }
define void @test05(%struct.test05.a** %p1, %struct.test05.b** %p2) {
entry:
  %p1n = bitcast %struct.test05.a** %p1 to i64*
  br i1 undef, label %other, label %merge

other:
  %p2n = bitcast %struct.test05.b** %p2 to i64*
  br label %merge

merge:
  %p3n = phi i64* [%p1n, %entry], [%p2n, %other]
  ret void
}

; CHECK: LLVMType: %struct.test05.a = type { i32, i32 }
; CHECK: Safety data: Unsafe pointer merge
; CHECK: LLVMType: %struct.test05.b = type { i64, i64 }
; CHECK: Safety data: Unsafe pointer merge

; Select merges an unknown pointer and an aliased pointer.
%struct.test06 = type { i32, i32 }
define void @test06(%struct.test06* %p1, i8* %p2) {
  %p1b = bitcast %struct.test06* %p1 to i8*
  %p3 = select i1 undef, i8* %p1b, i8* %p2
  ret void
}

; CHECK: LLVMType: %struct.test06 = type { i32, i32 }
; CHECK: Safety data: Unsafe pointer merge

; PHI node merges an unknown pointer and an aliased pointer.
%struct.test07 = type { i32, i32 }
define void @test07(%struct.test07* %p1, i8* %p2) {
entry:
  %p1b = bitcast %struct.test07* %p1 to i8*
  br i1 undef, label %other, label %merge

other:
  br label %merge

merge:
  %p3 = phi i8* [%p1b, %entry], [%p2, %other]
  ret void
}

; CHECK: LLVMType: %struct.test07 = type { i32, i32 }
; CHECK: Safety data: Unsafe pointer merge

; Select merges pointers of the same type but different levels of indirection.
%struct.test08 = type { i32, i32 }
define void @test08(%struct.test08* %p1, %struct.test08** %p2) {
  %p1b = bitcast %struct.test08* %p1 to i8*
  %p2b = bitcast %struct.test08** %p2 to i8*
  %p3 = select i1 undef, i8* %p1b, i8* %p2b
  ret void
}

; CHECK: LLVMType: %struct.test08 = type { i32, i32 }
; CHECK: Safety data: Unsafe pointer merge

; PHI merges pointers of the same type but different levels of indirection.
%struct.test09 = type { i32, i32 }
define void @test09(%struct.test09* %p1, %struct.test09** %p2) {
entry:
  %p1b = bitcast %struct.test09* %p1 to i8*
  br i1 undef, label %other, label %merge

other:
  %p2b = bitcast %struct.test09** %p2 to i8*
  br label %merge

merge:
  %p3 = phi i8* [%p1b, %entry], [%p2b, %other]
  ret void
}

; CHECK: LLVMType: %struct.test09 = type { i32, i32 }
; CHECK: Safety data: Unsafe pointer merge

; Select merges pointers to different elements.
%struct.test10 = type { i32, i32 }
define void @test10(%struct.test10* %p) {
  %p.a = getelementptr %struct.test10, %struct.test10* %p, i64 0, i32 0
  %p.b = getelementptr %struct.test10, %struct.test10* %p, i64 0, i32 1
  %p3 = select i1 undef, i32* %p.a, i32* %p.b
  ret void
}

; CHECK: LLVMType: %struct.test10 = type { i32, i32 }
; CHECK: Safety data: No issues found

; PHI merges pointers to different elements.
%struct.test11 = type { i32, i32 }
define void @test11(%struct.test11* %p) {
entry:
  %p.a = getelementptr %struct.test11, %struct.test11* %p, i64 0, i32 0
  br i1 undef, label %other, label %merge

other:
  %p.b = getelementptr %struct.test11, %struct.test11* %p, i64 0, i32 1
  br label %merge

merge:
  %p3 = phi i32* [%p.a, %entry], [%p.b, %other]
  ret void
}

; CHECK: LLVMType: %struct.test11 = type { i32, i32 }
; CHECK: Safety data: No issues found
