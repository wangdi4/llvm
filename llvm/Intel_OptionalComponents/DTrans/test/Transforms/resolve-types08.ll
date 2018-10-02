; RUN:  opt -S -o - -whole-program-assume -dtrans-resolvetypes %s | FileCheck %s

; This test verifies that the dtrans::ResolveTypes correctly combines
; types that are nested within other structures.

; These types should all be combined.
%A = type { i32, %B* }
%B = type { i32, i32 }
%B.1 = type { i32, i32 }

; CHECK-DAG: %__DTRT_A = type { i32, %__DTRT_B* }
; CHECK-DAG: %__DTRT_B = type { i32, i32 }
; CHECK-NOT: %B = type { i32, i32 }
; CHECK-NOT: %B.1 = type { i32, i32 }

; The call interfaces are the important thing in the tests. We don't actually
; need to do anything with the elements.

define void @useB(%B* %p) {
  ret void
}
; CHECK-NOT: void @useB(%B* %p)

define void @useB1(%B.1* %p) {
  ret void
}
; CHECK-NOT: void @useB1(%B.1* %p)

define void @test(%A* %a) {
  %pb = getelementptr %A, %A* %a, i64 0, i32 1
  %b = load %B*, %B** %pb
  call void @useB(%B* %b)
  call void bitcast (void (%B.1*)* @useB1 to void (%B*)*) (%B* %b)
  ret void
}

; CHECK-LABEL: void @useB.1(%__DTRT_B* %p)

; CHECK-LABEL: void @useB1.2(%__DTRT_B* %p)

; CHECK-LABEL: void @test.3(%__DTRT_A* %a)
; CHECK:  %pb = getelementptr %__DTRT_A, %__DTRT_A* %a, i64 0, i32 1
; CHECK:  %b = load %__DTRT_B*, %__DTRT_B** %pb
; CHECK:  call void @useB.1(%__DTRT_B* %b)
; CHECK:  call void @useB1.2(%__DTRT_B* %b)
