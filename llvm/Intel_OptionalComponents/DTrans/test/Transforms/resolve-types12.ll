; UNSUPPORTED: enable-opaque-pointers
; RUN:  opt -S -o - -whole-program-assume -intel-libirc-allowed -dtrans-resolvetypes %s | FileCheck %s
; RUN:  opt -S -o - -whole-program-assume -intel-libirc-allowed -passes=dtrans-resolvetypes %s | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; This test verifies that the dtrans::ResolveTypes does not combine types when
; a type that points to one of a pair of mutually referencing types that could
; otherwise be remapped is used by an external function.

; These types should not be combined.
%A = type { %B.2* }
%A.1 = type { %B* }
%B = type { %A.1* }
%B.2 = type { %A* }

; This is checking the case where the type with an external dependency is
; inside an array.
%C = type { [2 x %A] }
%C.3 = type { [2 x %A.1] }

; CHECK-DAG: %A = type { %B.2* }
; CHECK-DAG: %A.1 = type { %B* }
; CHECK-DAG: %B = type { %A.1* }
; CHECK-DAG: %B.2 = type { %A* }
; CHECK-DAG: %C = type { [2 x %A] }
; CHECK-DAG: %C.3 = type { [2 x %A.1] }

; The call interfaces are the important thing in the tests. We don't actually
; need to call the functions, but we need to have a user of the declarations.

define void @useC(%C* %c) {
  ret void
}
; CHECK: void @useC(%C* %c)

@useC3_user = global void(%C.3*)* @useC3
declare void @useC3(%C.3*)
; CHECK: void @useC3(%C.3*)

define void @useB(%B* %b) {
  ret void
}
; CHECK: void @useB(%B* %b)

define void @useB2(%B.2**) {
  ret void
}
; CHECK: void @useB2(%B.2** %0)

define void @useA1(%A.1* %a) {
  ret void
}
; CHECK: void @useA1(%A.1* %a)

define void @test(%A* %a) {
  %b = getelementptr %A, %A* %a, i64 0, i32 0
  call void @useB2(%B.2** %b)
  ret void
}
; CHECK: void @test(%A* %a)
