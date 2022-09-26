; UNSUPPORTED: enable-opaque-pointers
; RUN:  opt -S -o - -whole-program-assume -dtrans-resolvetypes %s | FileCheck %s
; RUN:  opt -S -o - -whole-program-assume -passes=dtrans-resolvetypes %s | FileCheck %s

; This test verifies that equivalent types that are directly
; dependent on external types do not get remapped, but the external
; type does not prevent remapping of types which may be reached along
; the dependency chain that would not trigger GEP conflicts or require
; remapping types that are used externally.

; It is NOT ok to merge these because they contain a type that is not allowed
; to be remapped because %struct.inner will be passed to an extern function.
; Remapping them would cause the GEP instructions to have an incorrect type.
%struct.premiddle = type { %struct.middle* }
%struct.premiddle.1 = type { %struct.middle.1* }
%struct.middle = type { %struct.inner* }
%struct.middle.1 = type { %struct.inner.1* }

; It is NOT ok to merge these because they are passed to external functions.
%struct.inner = type { i32 }
%struct.inner.1 = type { i32 }

; These types can be remapped because %struct.outer is not going to be a
; candidate for remapping, and nothing in the dependency chain beneath it
; will be allowed to be remapped. These ensures that %struct.outer will
; stay the same type, thus breaking the dependency chain coming from the
; types that are used externally.
%struct.outer2 = type { %struct.outer*, %struct.foo* }
%struct.outer2.1 = type { %struct.outer*, %struct.foo.1* }
%struct.foo = type { i16, i16 }
%struct.foo.1 = type { i16, i16 }

%struct.outer = type { %struct.premiddle* }

; CHECK-DAG: %struct.middle = type { %struct.inner* }
; CHECK-DAG: %struct.middle.1 = type { %struct.inner.1* }
; CHECK-DAG: %struct.premiddle = type { %struct.middle* }
; CHECK-DAG: %struct.premiddle.1 = type { %struct.middle.1* }
; CHECK-DAG: %struct.inner = type { i32 }
; CHECK-DAG: %struct.inner.1 = type { i32 }
; CHECK-DAG: %__DTRT_struct.outer2 = type { %struct.outer*, %__DTRT_struct.foo* }
; CHECK-DAG: %__DTRT_struct.foo = type { i16, i16 }

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

; Just references the types to be sure they are kept
define void @use_outer2(%struct.outer2 %o) {
  ret void
}

define void @use_outer2.1(%struct.outer2.1 %o) {
  ret void
}

define void @use_premiddle(%struct.premiddle* %p) {
  ret void
}

define void @use_premiddle.1(%struct.premiddle.1* %p) {
  ret void
}

declare void @ext_test01(%struct.inner* %b)
declare void @ext_test01.1(%struct.inner.1* %b)
