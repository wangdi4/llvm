; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -whole-program-assume -dtransanalysis -debug-only=dtrans-lpa-results -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -passes='require<dtransanalysis>' -debug-only=dtrans-lpa-results -disable-output 2>&1 | FileCheck %s

; Test various cases of byte-flattened GEP accesses. In particular, verify
; cases where special element-zero handling gets applied to the structure.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"


; Byte-flattend GEP without special handling of first element of structure.
%struct.test01 = type { i64, i32 }
define void @test01(%struct.test01* %p) {
  %p2 = bitcast %struct.test01* %p to i8*
  %py8 = getelementptr i8, i8* %p2, i64 8
  %py = bitcast i8* %py8 to i32*
  %y = load i32, i32* %py
  ret void
}
; CHECK-LABEL: void @test01(
; CHECK: %py = bitcast i8* %py8 to i32*
; CHECK-NEXT:    LocalPointerInfo:
; CHECK-NEXT:      Aliased types:
; CHECK-NEXT:        i32*
; CHECK-NEXT:        i8*
; CHECK-NEXT:      Element pointees:
; CHECK-NEXT:        %struct.test01 @ 1

; Byte-flattened GEP when first element of structure is ptr-to-ptr
%struct.test02 = type { i64**, i32 }
define void @test02(%struct.test02* %p) {
  %p2 = bitcast %struct.test02* %p to i8*
  %py8 = getelementptr i8, i8* %p2, i64 8
  %py = bitcast i8* %py8 to i32*
  %y = load i32, i32* %py
  ret void
}
; CHECK-LABEL: void @test02(
; CHECK: %py = bitcast i8* %py8 to i32*
; CHECK-NEXT:    LocalPointerInfo:
; CHECK-NEXT:      Aliased types:
; CHECK-NEXT:        i32*
; CHECK-NEXT:        i8*
; CHECK-NEXT:      Element pointees:
; CHECK-NEXT:        %struct.test02 @ 1


; Byte-flattened GEP when first element of structure is i8*
%struct.test03 = type { i8*, i32 }
define void @test03(%struct.test03* %p) {
  %p2 = bitcast %struct.test03* %p to i8*
  %py8 = getelementptr i8, i8* %p2, i64 8
  %py = bitcast i8* %py8 to i32*
  %y = load i32, i32* %py
  ret void
}
; CHECK-LABEL: void @test03(
; CHECK: %py = bitcast i8* %py8 to i32*
; CHECK-NEXT:    LocalPointerInfo:
; CHECK-NEXT:      Aliased types:
; CHECK-NEXT:        i32*
; CHECK-NEXT:        i8*
; CHECK-NEXT:      Element pointees:
; CHECK-NEXT:        %struct.test03 @ 1


; Byte-flattened GEP when first element of structure is array of i8
%struct.test04 = type { [8 x i8], i32 }
define void @test04(%struct.test04* %p) {
  %p2 = bitcast %struct.test04* %p to i8*
  %py8 = getelementptr i8, i8* %p2, i64 8
  %py = bitcast i8* %py8 to i32*
  %y = load i32, i32* %py
  ret void
}
; CHECK-LABEL: void @test04(
; CHECK: %py = bitcast i8* %py8 to i32*
; CHECK-NEXT:    LocalPointerInfo:
; CHECK-NEXT:      Aliased types:
; CHECK-NEXT:        i32*
; CHECK-NEXT:        i8*
; CHECK-NEXT:      Element pointees:
; CHECK-NEXT:        %struct.test04 @ 1


; Byte-flattened GEP when first element of structure is i8* of nested
; structure, and GEP accesses element of innter structure
%struct.test05inner = type { i8*, i32, i32 }
%struct.test05outer = type { %struct.test05inner, i32 }
define void @test05(%struct.test05outer* %p) {
  %p2 = bitcast %struct.test05outer* %p to i8*
  %py8 = getelementptr i8, i8* %p2, i64 8
  %py = bitcast i8* %py8 to i32*
  %y = load i32, i32* %py
  ret void
}
; CHECK-LABEL: void @test05(
; CHECK: %py = bitcast i8* %py8 to i32*
; CHECK-NEXT:    LocalPointerInfo:
; CHECK-NEXT:      Aliased types:
; CHECK-NEXT:        i32*
; CHECK-NEXT:        i8*
; CHECK-NEXT:      Element pointees:
; CHECK-NEXT:        %struct.test05inner @ 1

; Byte-flattened GEP when first element of structure is i8* of nested
; structure, and GEP accesses element of outer structure
%struct.test06inner = type { i8*, i32, i32 }
%struct.test06outer = type { %struct.test05inner, i32 }
define void @test06(%struct.test06outer* %p) {
  %p2 = bitcast %struct.test06outer* %p to i8*
  %py8 = getelementptr i8, i8* %p2, i64 16
  %py = bitcast i8* %py8 to i32*
  %y = load i32, i32* %py
  ret void
}
; CHECK-LABEL: void @test06(
; CHECK: %py = bitcast i8* %py8 to i32*
; CHECK-NEXT:    LocalPointerInfo:
; CHECK-NEXT:      Aliased types:
; CHECK-NEXT:        i32*
; CHECK-NEXT:        i8*
; CHECK-NEXT:      Element pointees:
; CHECK-NEXT:        %struct.test06outer @ 1

