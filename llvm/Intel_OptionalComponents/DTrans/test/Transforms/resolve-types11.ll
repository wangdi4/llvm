; RUN:  opt -S -o - -whole-program-assume -dtrans-resolvetypes %s | FileCheck %s

; This test verifies that the dtrans::ResolveTypes does not combine types when
; one of a pair of types that could otherwise be remapped is used by an
; external function.

; These types should not be combined.
%A = type { %B.2 }
%A.1 = type { %B }
%B = type { i32, i32 }
%B.2 = type { i32, i32 }
%C = type { i64, i64 }
%C.3 = type { i64, i64 }
%D = type { i16, i16 }
%D.4 = type { i16, i16 }

; Here E.5 should not be replaced, but E and E.6 can be merged.
%E = type { [4 x i16] }
%E.5 = type { [4 x i16] }
%E.6 = type { [4 x i16] }

; Here F has an extern dependency but F.7 and F.8 do not. Theoretically, we
; could merge F.7 and F.8 but the current implementation doesn't do that
; because the 'base' type is rejected. Given that there is no need to handle
; this case it doesn't seem worth restructuring the code to make it happen.
%F = type { [4 x i32] }
%F.7 = type { [4 x i32] }
%F.8 = type { [4 x i32] }

; This is checking the case where the type with an external dependency is
; inside an array.
%H = type { [2 x %A] }
%H.9 = type { [2 x %A.1] }

; CHECK-DAG: %A = type { %B.2 }
; CHECK-DAG: %A.1 = type { %B }
; CHECK-DAG: %B = type { i32, i32 }
; CHECK-DAG: %B.2 = type { i32, i32 }
; CHECK-DAG: %C = type { i64, i64 }
; CHECK-DAG: %C.3 = type { i64, i64 }
; CHECK-DAG: %D = type { i16, i16 }
; CHECK-DAG: %D.4 = type { i16, i16 }
; CHECK-DAG: %__DTRT_E = type { [4 x i16] }
; CHECK-DAG: %E.5 = type { [4 x i16] }
; CHECK-DAG: %F = type { [4 x i32] }
; CHECK-DAG: %F.7 = type { [4 x i32] }
; CHECK-DAG: %F.8 = type { [4 x i32] }
; CHECK-DAG: %H = type { [2 x %A] }
; CHECK-DAG: %H.9 = type { [2 x %A.1] }

; CHECK-NOT: %E.6 = type { [4 x i16] }

; The call interfaces are the important thing in the tests. We don't actually
; need to do anything with the elements.

define void @useH(%H* %h) {
  ret void
}
; CHECK: void @useH(%H* %h)

define void @useH9(%H.9* %h) {
  ret void
}
; CHECK: void @useH9(%H.9* %h)

declare void @useF(%F*)
; CHECK: void @useF(%F*)

define void @useF7(%F.7* %f) {
  ret void
}
; CHECK: void @useF7(%F.7* %f)

define void @useF8(%F.8* %f) {
  ret void
}
; CHECK: void @useF8(%F.8* %f)

define void @useE(%E* %e) {
  ret void
}
; CHECK-NOT: void @useE(%E* %e)

declare void @useE5(%E.5*)
; CHECK: void @useE5(%E.5*)

define void @useE6(%E.6* %e) {
  ret void
}
; CHECK-NOT: void @useE6(%E.6* %e)

declare %D.4* @retD4()
; CHECK: %D.4* @retD4()

define void @useD(%D* %d) {
  ret void
}
; CHECK: void @useD(%D* %d)

declare void @useC(%C*)
; CHECK: void @useC(%C*)

define void @useC3(%C.3* %c) {
  ret void
}
; CHECK: void @useC3(%C.3* %c)

define void @useB(%B* %b) {
  ret void
}
; CHECK: void @useB(%B* %b)

declare void @useB2(%B.2*)
; CHECK: void @useB2(%B.2*)

define void @useA1(%A.1* %a) {
  ret void
}
; CHECK: void @useA1(%A.1* %a)

define void @test(%A* %a) {
  %b = getelementptr %A, %A* %a, i64 0, i32 0
  call void @useB2(%B.2* %b)
  ret void
}
; CHECK: void @test(%A* %a)

; CHECK: void @useE.1(%__DTRT_E* %e)
; CHECK: void @useE6.2(%__DTRT_E* %e)
