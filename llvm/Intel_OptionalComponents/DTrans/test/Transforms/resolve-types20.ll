; UNSUPPORTED: enable-opaque-pointers
; RUN:  opt -S -o - -whole-program-assume -dtrans-resolvetypes %s | FileCheck %s
; RUN:  opt -S -o - -whole-program-assume -passes=dtrans-resolvetypes %s | FileCheck %s

; This test is here to verify that compatible types are handled correctly when
; a deeply nested type cannot be remapped, and therefore should prevent the
; outer types from remapping.

; These types are compatible to each other, but would require multiple levels
; of nested types to be remapped in the same direction.
%struct.farouter = type { %struct.outer }
%struct.farouter.0 = type { %struct.outer.0 }

%struct.outer = type { i32, i32, %struct.middle.1, %struct.middlec.1* }
%struct.outer.0 = type { i32, i32, %struct.middle, %struct.middlec* }

; These types should be compatible but cannot be remapped because they
; have uses of the conflicting fields.
%struct.middle = type { i32,  %struct.bar*, i32 }
%struct.middle.1 = type { i32, %struct.bar.1*, i32 }

; These types are here to fill in the other types that are under test.
%struct.middlec = type { i32, i32, %struct.bar* }
%struct.bar = type { i32*, i32 }
%struct.bar.1 = type { i16*, i32 }
%struct.middlec.1 = type { i32, i32, %struct.foo* }
%struct.foo = type { i16, i16, i32 }

; The original types for all compatible types should all be preserved.
; CHECK-DAG: %struct.farouter = type { %struct.outer }
; CHECK-DAG: %struct.farouter.0 = type { %struct.outer.0 }
; CHECK-DAG: %struct.outer = type { i32, i32, %struct.middle.1, %struct.middlec.1* }
; CHECK-DAG: %struct.outer.0 = type { i32, i32, %struct.middle, %struct.middlec* }
; CHECK-DAG: %struct.middle = type { i32,  %struct.bar*, i32 }
; CHECK-DAG: %struct.middle.1 = type { i32, %struct.bar.1*, i32 }

define void @f1(%struct.farouter* %o) {
  call void bitcast (void (%struct.farouter.0*)* @use_farouter to void (%struct.farouter*)*)(%struct.farouter* %o)
  ret void
}

define void @use_middle(%struct.middle* %m) {
  %f0 = getelementptr %struct.middle, %struct.middle* %m, i64 0, i32 0
  %v0 = load i32, i32* %f0
  %ppbar = getelementptr %struct.middle, %struct.middle* %m, i64 0, i32 1
  ret void
}

; Use %struct.middle.1, including the field that conflicts with %struct.middle
define i32 @use_middle1(%struct.middle.1* %m) {
  %f0 = getelementptr %struct.middle.1, %struct.middle.1* %m, i64 0, i32 0
  %v0 = load i32, i32* %f0
  %f1 = getelementptr %struct.middle.1, %struct.middle.1* %m, i64 0, i32 2
  %v1 = load i32, i32* %f1

  %ppbar = getelementptr %struct.middle.1, %struct.middle.1* %m, i64 0, i32 1
  %add = add i32 %v0, %v1
  ret i32 %add
}

define void @use_bar(%struct.bar** %b) {
  ret void
}

define void @use_outer(%struct.outer.0* %o) {
  ret void
}

define void @use_farouter(%struct.farouter.0* %o) {
  ret void
}
