; RUN:  opt -S -o - -whole-program-assume -dtrans-resolvetypes \
; RUN:      -debug-only=dtrans-resolvetypes-verbose %s 2>&1 | FileCheck %s
; REQUIRES: asserts

; This test is here to verify that compatible but not equivalent types are
; handled correctly across several levels of pointer nesting.

%A = type { i32, %B* }
%A.1 = type { i32, %B.1* }

%B = type { i32, %C* }
%B.1 = type { i32, %C.1* }

%C = type { i32, %C* }
%C.1 = type { i32, %D* }

%D = type { i8*, i8* }

define void @f(%A* %a) {
  call void bitcast (void (%A.1*)* @useA1 to void (%A*)*)(%A* %a)
  ret void
}

define void @useA1(%A.1* %a) {
  ret void
}

define void @useB(%B* %b) {
  %pc = getelementptr %B, %B* %b, i64 0, i32 1
  %c = load %C*, %C** %pc
  call void @useC(%C* %c)
  ret void
}

define void @useC(%C* %b) {
  ret void
}

; FIXME: The checks below are verifying debug output. When the transformation
;        is fully implemented these should be replaced with checks of the
;        actually corrected IR.

; CHECK-LABEL: compareTypeMembers(A, A.1)
; CHECK-NEXT: compareTypeMembers(B, B.1)
; CHECK-NEXT: compareTypeMembers(C, C.1)
; CHECK-NEXT: Type name mismatch @ 1
; CHECK-NEXT: StElemATy = C
; CHECK-NEXT: StElemBTy = D
; CHECK-NEXT: All members equivalent or compatible.
; CHECK-NEXT: Element member mismatch @ 1
; CHECK-NEXT: All members equivalent or compatible.
; CHECK-NEXT: Element member mismatch @ 1
; CHECK-NEXT: All members equivalent or compatible.
; CHECK-NEXT: resolve-types: Types are compatible.
; CHECK-NEXT:    Base: %A = type { i32, %B* }
; CHECK-NEXT:    Cand: %A.1 = type { i32, %B.1* }
; CHECK-NEXT: resolve-types: Types are not equivalent.
; CHECK-NEXT:    Base: %A = type { i32, %B* }
; CHECK-NEXT:    Cand: %A.1 = type { i32, %B.1* }
