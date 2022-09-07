; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; The types all get eliminated without some use of them. That's why the
; function declarations are here.

; Check for the identification of system structure types.
%struct._IO_FILE = type { i32, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, %struct._IO_marker*, %struct._IO_FILE*, i32, i32, i64, i16, i8, [1 x i8], i8*, i64, i8*, i8*, i8*, i8*, i64, i32, [20 x i8] }
%struct._IO_marker = type { %struct._IO_marker*, %struct._IO_FILE*, i32 }

declare void @test01(%struct._IO_FILE*)

; CHECK: LLVMType: %struct._IO_FILE = type { i32, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, %struct._IO_marker*, %struct._IO_FILE*, i32, i32, i64, i16, i8, [1 x i8], i8*, i64, i8*, i8*, i8*, i8*, i64, i32, [20 x i8] }
; CHECK: Safety data: System object
; CHECK: LLVMType: %struct._IO_marker = type { %struct._IO_marker*, %struct._IO_FILE*, i32 }
; CHECK: Safety data: System object

; Check that a pointer to a structure isn't misidentified as nesting.
%struct.test02.a = type { i32, i32 }
%struct.test02.b = type { i32, %struct.test02.a* }
declare void @test02(%struct.test02.b*)

; CHECK: LLVMType: %struct.test02.a = type { i32, i32 }
; CHECK: Safety data: No issues found
; CHECK: LLVMType: %struct.test02.b = type { i32, %struct.test02.a* }
; CHECK: Safety data: No issues found

; Check that a non-pointer structure field is identified as nesting.
%struct.test03.a = type { i32, i32 }
%struct.test03.b = type { i32, %struct.test03.a }
declare void @test03(%struct.test03.b*)

; CHECK: LLVMType: %struct.test03.a = type { i32, i32 }
; CHECK: Safety data: Nested structure
; CHECK: LLVMType: %struct.test03.b = type { i32, %struct.test03.a }
; CHECK: Safety data: Contains nested structure

; Check that empty structures are identified.
%struct.test04 = type {}
declare void @test04(%struct.test04*)

; CHECK: LLVMType: %struct.test04 = type {}
; CHECK: Safety data: No fields in structure

; Check that an array of structures are identified as nesting.
%struct.test05.a = type { i32, i32 }
%struct.test05.b = type { i32, [10 x %struct.test05.a] }
declare void @test06(%struct.test05.b*)

; CHECK: LLVMType: %struct.test05.a = type { i32, i32 }
; CHECK: Safety data: Nested structure
; CHECK: LLVMType: %struct.test05.b = type { i32, [10 x %struct.test05.a] }
; CHECK: Safety data: Contains nested structure

; Check that %struct.test06.eh.CatchableType and
; %struct.test06.eh.CatchableTypeArray.1 have 'Bad casting'
; due to the bitcast in the global variable @_TI1H of type
; %struct.test06.eh.ThrowInfo.
; NOTE: See DTransAnalysis.cpp: We may want to replace this with "Unsafe
; pointer store" after the Feb 2020 release.

$_TI1H = comdat any
$_CTA1H = comdat any
%struct.test06.eh.ThrowInfo = type { i32, i8*, i8*, i8* }
%struct.test06.eh.CatchableType = type { i32, i8*, i32, i32, i32, i32, i8* }
%struct.test06.eh.CatchableTypeArray.1 = type { i32, [1 x %struct.test06.eh.CatchableType*] }
@_TI1H = internal unnamed_addr constant %struct.test06.eh.ThrowInfo { i32 0, i8* null, i8* null, i8* bitcast (%struct.test06.eh.CatchableTypeArray.1* @_CTA1H to i8*) }, section ".xdata", comdat
@_CTA1H = internal unnamed_addr constant %struct.test06.eh.CatchableTypeArray.1 { i32 1, [1 x %struct.test06.eh.CatchableType*] [%struct.test06.eh.CatchableType* null ] }, section ".xdata", comdat

declare void @test07(%struct.test06.eh.ThrowInfo*)

; CHECK: LLVMType: %struct.test06.eh.CatchableType = type { i32, i8*, i32, i32, i32, i32, i8* }
; CHECK: Safety data: Bad casting
; CHECK: LLVMType: %struct.test06.eh.CatchableTypeArray.1 = type { i32, [1 x %struct.test06.eh.CatchableType*] }
; CHECK: Safety data: Bad casting | Global instance | Has initializer list
