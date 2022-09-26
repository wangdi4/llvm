; UNSUPPORTED: enable-opaque-pointers
; RUN:  opt -S -o - -whole-program-assume -dtrans-resolvetypes %s | FileCheck %s
; RUN:  opt -S -o - -whole-program-assume -passes=dtrans-resolvetypes %s | FileCheck %s

; This test is here to verify that compatible but not equivalent types are
; handled correctly when they have type equivalent types nested within them.

; These types are compatible to each other, but contain a nested type.
; The nested type will have been resolved as being type equivalent to
; each other.
%struct.outer = type { i32, i32, %struct.middle.1, %struct.middlec.1* }
%struct.outer.0 = type { i32, i32, %struct.middle, %struct.middlec* }

; These are identified as being type equivalent types.
%struct.middle = type { i32,  %struct.bar* , i32}
%struct.middle.1 = type { i32, %struct.bar*, i32 }

; These types are here to fill in the other types that are under test.
%struct.middlec = type { i32, i32, %struct.bar* }
%struct.bar = type { i32*, i32 }
%struct.middlec.1 = type { i32, i32, %struct.foo* }
%struct.foo = type { i16, i16, i32 }

; CHECK-DAG: %__DTRT_struct.outer.0 = type { i32, i32, %__DTRT_struct.middle, %struct.middlec* }
; CHECK-DAG: %__DTRT_struct.middle = type { i32, %struct.bar*, i32 }

define void @f1(%struct.outer* %o) {
  call void bitcast (void (%struct.outer.0*)* @use_outer to void (%struct.outer*)*)(%struct.outer* %o)
  ret void
}

define void @use_middle(%struct.middle* %m) {
  ret void
}

define void @use_middle1(%struct.middle.1* %m) {
  ret void
}

define void @use_bar(%struct.bar** %b) {
  ret void
}

define void @use_outer(%struct.outer.0* %o) {
  ret void
}

