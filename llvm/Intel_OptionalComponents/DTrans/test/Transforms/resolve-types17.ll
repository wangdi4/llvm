; UNSUPPORTED: enable-opaque-pointers
; RUN:  opt -S -o - -whole-program-assume -dtrans-resolvetypes %s | FileCheck %s
; RUN:  opt -S -o - -whole-program-assume -passes=dtrans-resolvetypes %s | FileCheck %s

; This test verifies that the analysis of the dependency chains for
; external types works in the case of cyclic chains. No types should
; be remapped in this test.

; It is NOT ok to merge these because they contain a type that is not allowed
; to be remapped because %struct.inner will be passed to an extern function.
; Remapping them would cause the GEP instructions to have an incorrect type.
%struct.premiddle = type { %struct.postmiddle* }
%struct.premiddle.1 = type { %struct.postmiddle.1* }
%struct.postmiddle = type { %struct.middle* }
%struct.postmiddle.1 = type { %struct.middle.1* }
%struct.middle = type { %struct.inner*, %struct.premiddle* }
%struct.middle.1 = type { %struct.inner.1*, %struct.premiddle.1* }

; It is NOT ok to merge these because they are passed to external functions.
%struct.inner = type { i32 }
%struct.inner.1 = type { i32 }

%struct.outer = type { %struct.premiddle* }

; CHECK-DAG: %struct.premiddle = type { %struct.postmiddle* }
; CHECK-DAG: %struct.premiddle.1 = type { %struct.postmiddle.1* }
; CHECK-DAG: %struct.postmiddle = type { %struct.middle* }
; CHECK-DAG: %struct.postmiddle.1 = type { %struct.middle.1* }
; CHECK-DAG: %struct.middle = type { %struct.inner*, %struct.premiddle* }
; CHECK-DAG: %struct.middle.1 = type { %struct.inner.1*, %struct.premiddle.1* }
; CHECK-DAG: %struct.inner = type { i32 }
; CHECK-DAG: %struct.inner.1 = type { i32 }
; CHECK-DAG: %struct.outer = type { %struct.premiddle* }

define void @use_middle(%struct.middle* %m) {
  %ptr = getelementptr %struct.middle, %struct.middle* %m, i64 0, i32 0
  %i = load %struct.inner*, %struct.inner** %ptr
  call void @ext_test01(%struct.inner* %i)
  ret void
}

define void @use_middle.1(%struct.middle.1* %m) {
  %ptr = getelementptr %struct.middle.1, %struct.middle.1* %m, i64 0, i32 0
  %i = load %struct.inner.1*, %struct.inner.1** %ptr
  call void @ext_test01.1(%struct.inner.1* %i)
  ret void
}

; Just references the type to be sure it is kept
define void @use_outer(%struct.outer %o) {
  ret void
}

declare void @ext_test01(%struct.inner* %b)
declare void @ext_test01.1(%struct.inner.1* %b)
