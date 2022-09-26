; UNSUPPORTED: enable-opaque-pointers
; RUN:  opt -S -o - -whole-program-assume -dtrans-resolvetypes %s | FileCheck %s
; RUN:  opt -S -o - -whole-program-assume -passes=dtrans-resolvetypes %s | FileCheck %s

; This test is here to verify that compatible types are handled correctly when
; one type is nested within another type, and the mapping requires the
; nested type to remap in the same direction.

; These types are compatible to each other. Remapping to %struct.outer.0
; will require %struct.middle.1 be remapped to %struct.middle
%struct.outer = type { i32, i32, %struct.middle.1, %struct.middlec.1* }
%struct.outer.0 = type { i32, i32, %struct.middle, %struct.middlec* }

; These types should be compatible and can be remapped because they
; do not have uses of the conflicting fields.
%struct.middle = type { i32,  %struct.bar*, i32 }
%struct.middle.1 = type { i32, %struct.bar.1*, i32 }

; These types are here to fill in the other types that are under test.
%struct.middlec = type { i32, i32, %struct.bar* }
%struct.bar = type { i32*, i32 }
%struct.bar.1 = type { i16*, i32 }
%struct.middlec.1 = type { i32, i32, %struct.foo* }
%struct.foo = type { i16, i16, i32 }

; CHECK-DAG: %__DTRT_struct.outer.0 = type { i32, i32, %__DTRT_struct.middle, %struct.middlec* }
; CHECK-DAG: %__DTRT_struct.middle = type { i32, %struct.bar*, i32 }


define void @f1(%struct.outer* %o) {
  call void bitcast (void (%struct.outer.0*)* @use_outer to void (%struct.outer*)*)(%struct.outer* %o)
  ret void
}

define void @use_middle(%struct.middle* %m) {
  %f0 = getelementptr %struct.middle, %struct.middle* %m, i64 0, i32 0
  %v0 = load i32, i32* %f0
  %ppbar = getelementptr %struct.middle, %struct.middle* %m, i64 0, i32 1
  ret void
}

; Use %struct.middle.1, but not the field that conflicts with %struct.middle
define i32 @use_middle1(%struct.middle.1* %m) {
  %f0 = getelementptr %struct.middle.1, %struct.middle.1* %m, i64 0, i32 0
  %v0 = load i32, i32* %f0
  %f1 = getelementptr %struct.middle.1, %struct.middle.1* %m, i64 0, i32 2
  %v1 = load i32, i32* %f1

  %add = add i32 %v0, %v1
  ret i32 %add
}

; These functions are here to access conflicting fields of %struct.middlec, but
; should not affect the remapping of %struct.outer, because that only contains
; a pointer to the type, rather than nesting this type.
define void @use_middlec(%struct.middlec* %m) {
  %ppbar = getelementptr %struct.middlec, %struct.middlec* %m, i64 0, i32 2
  ret void
}

define void @use_middlec.1(%struct.middlec.1* %m) {
  %ppfoo = getelementptr %struct.middlec.1, %struct.middlec.1* %m, i64 0, i32 2
  ret void
}

define void @use_bar(%struct.bar** %b) {
  ret void
}

define void @use_outer(%struct.outer.0* %o) {
  ret void
}

