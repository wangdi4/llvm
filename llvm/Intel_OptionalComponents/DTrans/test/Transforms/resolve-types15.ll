; UNSUPPORTED: enable-opaque-pointers
; RUN:  opt -S -o - -whole-program-assume -dtrans-resolvetypes %s | FileCheck %s
; RUN:  opt -S -o - -whole-program-assume -passes=dtrans-resolvetypes %s | FileCheck %s

; This test verifies that equivalent types that have external types as
; dependent types do not get remapped because doing so would lead to
; incorrect types in a GEP instruction.

; It is NOT ok to merge these because they contain a type that is not allowed
; to be remapped AND the field is accessed via a GEP.
%struct.outer = type { %struct.inner* }
%struct.outer.1 = type { %struct.inner.1* }

; It is NOT ok to merge these because they are passed to external functions.
%struct.inner = type { i32 }
%struct.inner.1 = type { i32 }

; CHECK-DAG: %struct.outer = type { %struct.inner* }
; CHECK-DAG: %struct.outer.1 = type { %struct.inner.1* }
; CHECK-DAG: %struct.inner = type { i32 }
; CHECK-DAG: %struct.inner.1 = type { i32 }

define void @use_test01(%struct.outer* %o) {
  %ptr = getelementptr %struct.outer, %struct.outer* %o, i64 0, i32 0
  %i = load %struct.inner*, %struct.inner** %ptr
  call void @ext_test01(%struct.inner* %i)
  ret void
}

define void @use_test01.1(%struct.outer.1* %o) {
  %ptr = getelementptr %struct.outer.1, %struct.outer.1* %o, i64 0, i32 0
  %i = load %struct.inner.1*, %struct.inner.1** %ptr
  call void @ext_test01.1(%struct.inner.1* %i)
  ret void
}

declare void @ext_test01(%struct.inner* %b)
declare void @ext_test01.1(%struct.inner.1* %b)
