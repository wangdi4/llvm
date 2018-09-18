; RUN:  opt -S -o - -whole-program-assume -dtrans-resolvetypes %s | FileCheck %s

; This test is here to verify that compatible but not equivalent types are
; handled correctly across several levels of pointer nesting.

%A = type { i32, %B* }
%A.1 = type { i32, %B.1* }

%B = type { i32, %C* }
%B.1 = type { i32, %C.1* }

%C = type { i32, %C* }
%C.1 = type { i32, %D* }

%D = type { i8*, i8* }

; %A can be remapped to %A.1. All other types remain the same.

; CHECK-DAG: %__DTRT_A.1 = type { i32, %B.1* }
; CHECK-DAG: %B = type { i32, %C* }
; CHECK-DAG: %B.1 = type { i32, %C.1* }
; CHECK-DAG: %C = type { i32, %C* }
; CHECK-DAG: %C.1 = type { i32, %D* }
; CHECK-DAG: %D = type { i8*, i8* }

define void @f(%A* %a) {
  call void bitcast (void (%A.1*)* @useA1 to void (%A*)*)(%A* %a)
  ret void
}

; CHECK-LABEL: define{{.+}}void @f.1(%__DTRT_A.1* %a) {
; CHECK: call void @useA1.2(%__DTRT_A.1* %a)

define void @useA1(%A.1* %a) {
  ret void
}

; CHECK-LABEL: define{{.+}}void @useA1.2(%__DTRT_A.1* %a) {

define void @useB(%B* %b) {
  %pc = getelementptr %B, %B* %b, i64 0, i32 1
  %c = load %C*, %C** %pc
  call void @useC(%C* %c)
  ret void
}

define void @useC(%C* %b) {
  ret void
}
