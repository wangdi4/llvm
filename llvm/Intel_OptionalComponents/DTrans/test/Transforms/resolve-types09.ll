; RUN:  opt -S -o - -whole-program-assume -dtrans-resolvetypes %s | FileCheck %s

; This test verifies that the dtrans::ResolveTypes correctly combines
; types that are nested within other structures when the base-named type
; for the nested structure is missing.

; These types should all be combined.
%A = type { %B.2 }
%A.1 = type { %B.3 }
%B.2 = type { i32, i32 }
%B.3 = type { i32, i32 }

; CHECK-DAG: %__DTRT_A = type { %__DTRT_B }
; CHECK-DAG: %__DTRT_B = type { i32, i32 }
; CHECK-NOT: %B.2 = type { i32, i32 }
; CHECK-NOT: %B.3 = type { i32, i32 }

; The call interfaces are the important thing in the tests. We don't actually
; need to do anything with the elements.

define void @useB2(%B.2* %p) {
  ret void
}
; CHECK-NOT: void @useB2(%B.2* %p)

define void @useB3(%B.3* %p) {
  ret void
}
; CHECK-NOT: void @useB3(%B.3* %p)

define void @test(%A* %a) {
  %b = getelementptr %A, %A* %a, i64 0, i32 0
  call void @useB2(%B.2* %b)
  call void bitcast (void (%B.3*)* @useB3 to void (%B.2*)*) (%B.2* %b)
  ret void
}

define void @test1(%A.1* %a) {
  %b = getelementptr %A.1, %A.1* %a, i64 0, i32 0
  call void @useB3(%B.3* %b)
  call void bitcast (void (%B.2*)* @useB2 to void (%B.3*)*) (%B.3* %b)
  ret void
}

; CHECK-LABEL: void @useB2.1(%__DTRT_B* %p)

; CHECK-LABEL: void @useB3.2(%__DTRT_B* %p)

; CHECK-LABEL: void @test.3(%__DTRT_A* %a)
; CHECK:  %b = getelementptr %__DTRT_A, %__DTRT_A* %a, i64 0, i32 0
; CHECK:  call void @useB2.1(%__DTRT_B* %b)
; CHECK:  call void @useB3.2(%__DTRT_B* %b)

; CHECK-LABEL: void @test1.4(%__DTRT_A* %a)
; CHECK:  %b = getelementptr %__DTRT_A, %__DTRT_A* %a, i64 0, i32 0
; CHECK:  call void @useB3.2(%__DTRT_B* %b)
; CHECK:  call void @useB2.1(%__DTRT_B* %b)
