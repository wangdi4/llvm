; RUN: opt < %s -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; This test verifies correct analysis of values passed as arguments to
; function calls. If a value is passed using the type of a structure, it
; can be analyzed, but if it is passed in a generic form (such as i8*) then
; the address has escaped. Also, pointers to elements within an aggregate
; must be treated as the address of the field having been taken regardless of
; the type used.

; Call a function with an i8* aliased pointer.
%struct.test01 = type { i32, i32 }
declare void @f1(i8*);
define void @test1(%struct.test01* %s) {
  %p = bitcast %struct.test01* %s to i8*
  call void @f1(i8* %p)
  ret void
}

; CHECK: LLVMType: %struct.test01 = type { i32, i32 }
; CHECK: Safety data: Address taken

; Call a function with an i64 aliased pointer.
%struct.test02 = type { i32, i32 }
declare void @f2(i64);
define void @test2(%struct.test02* %s) {
  %n = ptrtoint %struct.test02* %s to i64
  call void @f2(i64 %n)
  ret void
}

; CHECK: LLVMType: %struct.test02 = type { i32, i32 }
; CHECK: Safety data: Address taken

; Call a function with an i8** aliased pointer.
%struct.test03 = type { i32, i32 }
declare void @f3(i8**);
define void @test3(%struct.test03** %ps) {
  %p = bitcast %struct.test03** %ps to i8**
  call void @f3(i8** %p)
  ret void
}

; CHECK: LLVMType: %struct.test03 = type { i32, i32 }
; CHECK: Safety data: Address taken

; Call a function with an i64* aliased pointer.
%struct.test04 = type { i32, i32 }
declare void @f4(i64*);
define void @test4(%struct.test04** %ps) {
  %p = bitcast %struct.test04** %ps to i64*
  call void @f4(i64* %p)
  ret void
}

; CHECK: LLVMType: %struct.test04 = type { i32, i32 }
; CHECK: Safety data: Address taken

; Call a function with a pointer to an element.
%struct.test05 = type { i32, i32 }
declare void @f5(i32*);
define void @test5(%struct.test05* %s) {
  %p = getelementptr %struct.test05, %struct.test05* %s, i64 0, i32 1
  call void @f5(i32* %p)
  ret void
}

; CHECK: LLVMType: %struct.test05 = type { i32, i32 }
; CHECK: Safety data: Field address taken

; Call a function with a pointer to an aggregate element.
%struct.test06.a = type { i32, i32 }
%struct.test06.b = type { i32, %struct.test06.a, i32 }
define void @f6(%struct.test06.a*) { ret void }
define void @test6(%struct.test06.b* %s) {
  %p = getelementptr %struct.test06.b, %struct.test06.b* %s, i64 0, i32 1
  call void @f6(%struct.test06.a* %p)
  ret void
}

; Note: test06.a gets the "Field address taken" condition because safety
;       conditions are propagated to nested structures.
; CHECK: LLVMType: %struct.test06.a = type { i32, i32 }
; CHECK: Safety data: Field address taken | Nested structure
; CHECK: LLVMType: %struct.test06.b = type { i32, %struct.test06.a, i32 }
; CHECK: Safety data: Field address taken | Contains nested structure

; Call an external function with a properly typed pointer to the structure.
; This is unsafe because we don't have the function's definition.
%struct.test07 = type { i32, i32 }
declare void @f7(%struct.test07*)
define void @test7(%struct.test07* %s) {
  call void @f7(%struct.test07* %s)
  ret void
}

; CHECK: LLVMType: %struct.test07 = type { i32, i32 }
; CHECK: Safety data: Address taken

; Call a local function with a properly typed pointer to the structure.
; This is safe because we do have the function's definition.
%struct.test08 = type { i32, i32 }

define void @f8(%struct.test08*) {
  ret void
}

define void @test8(%struct.test08* %s) {
  call void @f8(%struct.test08* %s)
  ret void
}

; CHECK: LLVMType: %struct.test08 = type { i32, i32 }
; CHECK: Safety data: No issues found

; Call a function with an overloaded pointer.
%struct.test09.a = type { i32, i32 }
%struct.test09.b = type { i32, i32, i32 }
declare void @f9(i8*)
define void @test9(%struct.test09.a* %pa, %struct.test09.b* %pb) {
  %p1 = bitcast %struct.test09.a* %pa to i8*
  %p2 = bitcast %struct.test09.b* %pb to i8*
  %p3 = select i1 undef, i8* %p1, i8* %p2
  call void @f9(i8* %p3)
  ret void
}

; CHECK: LLVMType: %struct.test09.a = type { i32, i32 }
; CHECK: Safety data: Unsafe pointer merge | Address taken
; CHECK: LLVMType: %struct.test09.b = type { i32, i32, i32 }
; CHECK: Safety data: Unsafe pointer merge | Address taken

; Call free with a pointer to a structure.
declare void @free(i8*)
%struct.test10 = type { i32, i32 }
define void @test10(%struct.test10* %p) {
  %tmp = bitcast %struct.test10* %p to i8*
  call void @free(i8* %tmp)
  ret void
}

; CHECK: LLVMType: %struct.test10 = type { i32, i32 }
; CHECK: Safety data: No issues found
