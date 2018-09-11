; RUN:  opt -S -o - -whole-program-assume -dtrans-resolvetypes %s | FileCheck %s

; This test verifies that the dtrans::ResolveTypes correctly combines
; types that are nested within other structures when there is a mismatched
; base-named type for the nested structure and one of the matching types is
; not used.

; These types should all be combined.
%A = type { %B.3 }
%A.1 = type { %B.2 }
%B = type { i64, i64 }
%B.2 = type { i32, i32 }
%B.3 = type { i32, i32 }

; CHECK-DAG: %B = type { i64, i64 }
; CHECK-DAG: %__DTRT_A = type { %__DTRT_B.2 }
; CHECK-DAG: %__DTRT_B.2 = type { i32, i32 }
; CHECK-NOT: %B.3 = type { i32, i32 }

; The call interfaces are the important thing in the tests. We don't actually
; need to do anything with the elements.

define void @useB(%B* %p) {
  ret void
}
; CHECK: void @useB(%B* %p)

define void @useB2(%B.2* %p) {
  ret void
}
; CHECK-NOT: void @useB2(%B.2* %p)

define void @test(%A.1* %a) {
  %b = getelementptr %A.1, %A.1* %a, i64 0, i32 0
  call void @useB2(%B.2* %b)
  ret void
}

; CHECK-LABEL: void @useB2.1(%__DTRT_B.2* %p)

; CHECK-LABEL: void @test.2(%__DTRT_A* %a)
; CHECK:  %b = getelementptr %__DTRT_A, %__DTRT_A* %a, i64 0, i32 0
; CHECK:  call void @useB2.1(%__DTRT_B.2* %b)
