; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

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
; CHECK: Safety data: Field address taken call

; Call a function with a pointer to an aggregate element.
%struct.test06.a = type { i32, i32 }
%struct.test06.b = type { i32, %struct.test06.a, i32 }
define void @f6(%struct.test06.a*) { ret void }
define void @test6(%struct.test06.b* %s) {
  %p = getelementptr %struct.test06.b, %struct.test06.b* %s, i64 0, i32 1
  call void @f6(%struct.test06.a* %p)
  ret void
}

; Note: test06.a gets the "Field address taken call" condition because safety
;       conditions are propagated to nested structures.
; CHECK: LLVMType: %struct.test06.a = type { i32, i32 }
; CHECK: Safety data: Nested structure | Field address taken call
; CHECK: LLVMType: %struct.test06.b = type { i32, %struct.test06.a, i32 }
; CHECK: Safety data: Contains nested structure | Field address taken call

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

; Call an external function with a properly typed pointer to a structure that
; contain a pointer to another type.
; This is unsafe for both the first type and the type pointed to by the first
; type because we don't have the function's definition.
%struct.test11.a = type { i32, i32, %struct.test11.b* }
%struct.test11.b = type { i32, i32 }
declare void @f11(%struct.test11.a*)
define void @test11(%struct.test11.a* %s) {
  call void @f11(%struct.test11.a* %s)
  ret void
}

; CHECK: LLVMType: %struct.test11.a = type { i32, i32, %struct.test11.b* }
; CHECK: Safety data: Address taken
; CHECK: LLVMType: %struct.test11.b = type { i32, i32 }
; CHECK: Safety data: Address taken

; Call an external function with a properly typed pointer to a structure that
; contains a pointer to another type that also has a pointer to the original
; type.
; This is unsafe for both the first type and the type pointed to by the first
; type because we don't have the function's definition.
%struct.test12.a = type { i32, i32, %struct.test12.b* }
%struct.test12.b = type { i32, i32, %struct.test12.a* }
declare void @f12(%struct.test12.a*)
define void @test12(%struct.test12.a* %s) {
  call void @f12(%struct.test12.a* %s)
  ret void
}

; CHECK: LLVMType: %struct.test12.a = type { i32, i32, %struct.test12.b* }
; CHECK: Safety data: Address taken
; CHECK: LLVMType: %struct.test12.b = type { i32, i32, %struct.test12.a* }
; CHECK: Safety data: Address taken

; Call strcpy with an i8* pointer to an i8 array field.
%struct.test13 = type { [100 x i8], i32 }
define void @test13(%struct.test13* %s) {
  %buf = alloca [100 x i8]
  %pbuf = bitcast [100 x i8]* %buf to i8*
  %psbuf = getelementptr inbounds %struct.test13, %struct.test13* %s,
                                  i64 0, i32 0, i32 0
  %tmp = call i8* @strcpy(i8* %pbuf, i8* %psbuf)
  ret void
}

; The important thing here is that we shouldn't report address taken.
; CHECK: LLVMType: %struct.test13 = type { [100 x i8], i32 }
; CHECK: Safety data: Field address taken

; Repeat the previous test with a bitcast to get the i8* pointer.
%struct.test14 = type { [100 x i8], i32 }
define void @test14(%struct.test14* %s) {
  %buf = alloca [100 x i8]
  %pbuf = bitcast [100 x i8]* %buf to i8*
  %sbuf = getelementptr inbounds %struct.test14, %struct.test14* %s,
                                 i64 0, i32 0
  %psbuf = bitcast [100 x i8]* %sbuf to i8*
  %tmp = call i8* @strcpy(i8* %pbuf, i8* %psbuf)
  ret void
}

; The important thing here is that we shouldn't report address taken.
; FIXME: Should this case also report address taken on the outer struct?
; CHECK: LLVMType: %struct.test14 = type { [100 x i8], i32 }
; CHECK: Safety data: No issues found

declare i8* @strcpy(i8*, i8*)
