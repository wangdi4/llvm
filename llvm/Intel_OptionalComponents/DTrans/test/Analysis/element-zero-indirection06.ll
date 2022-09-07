; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt -whole-program-assume  -dtransanalysis -debug-only=dtrans-lpa-results -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume  -passes='require<dtransanalysis>' -debug-only=dtrans-lpa-results -disable-output %s 2>&1 | FileCheck %s

; Test dominant type resolution when the pointer type information contains 3
; pointer types, where one type is considered the dominant type of the other
; two, but the other two do not directly dominate each other to verify that a
; dominant type is consistently determined. This is a regression test for
; CMPLRLLVM-32215 where the order of evaluation can affect the resolution of the
; dominant type.

%struct.container = type { %struct.outer }
%struct.outer = type { %struct.middle* }
%struct.middle = type { %struct.inner }
%struct.inner = type { i64 }

; A pointer to %struct.container is also a pointer to %struct.outer
; A pointer to %struct.outer is also a pointer-to-pointer of %struct.middle
; A pointer to %struct.middle is also a pointer to %struct.inner
; A pointer-to-pointer of struct.middle is also pointer-to-pointer of %struct.inner

@globV = global %struct.container zeroinitializer

define void @test() {
  %mixed = bitcast %struct.container* @globV to %struct.inner**
  %inner_use = getelementptr %struct.inner*, %struct.inner** %mixed, i64 0

  %mixed2 = bitcast %struct.inner** %mixed to %struct.middle**
  %middle_use = getelementptr %struct.middle*, %struct.middle** %mixed2, i64 0

  %mixed3 = bitcast %struct.middle** %mixed2 to %struct.outer*
  %outer_use = getelementptr %struct.outer, %struct.outer* %mixed3, i64 0

  ret void
}

; CHECK: %outer_use = getelementptr %struct.outer, %struct.outer* %mixed3, i64 0
; CHECK-NEXT:   LocalPointerInfo:
; CHECK-NEXT:      Aliased types:
; CHECK-NEXT:        %struct.inner**{{ *}}
; CHECK-NEXT:        %struct.middle**{{ *}}
; CHECK-NEXT:        %struct.outer*{{ *}}
; CHECK-NEXT:      Element pointees:
; CHECK-NEXT:        %struct.outer @ 0
; CHECK-NEXT:      DomTy: %struct.outer*{{ *}}
