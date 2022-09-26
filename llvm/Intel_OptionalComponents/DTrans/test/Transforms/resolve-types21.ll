; UNSUPPORTED: enable-opaque-pointers
; RUN:  opt -S -o - -whole-program-assume -dtrans-resolvetypes %s | FileCheck %s
; RUN:  opt -S -o - -whole-program-assume -passes=dtrans-resolvetypes %s | FileCheck %s

; This test is here to verify that compatible types are handled correctly when
; one type is nested within another type, and the mapping requires the
; nested type to remap multiple levels of nesting in the same direction.

; These types are compatible to each other, but will require multiple levels
; of nested types to be remapped in the same direction, and seeing the
; type at multiple levels of the nesting.
%struct.farouter = type { %struct.outer, %struct.middle.1 }
%struct.farouter.0 = type { %struct.outer.0, %struct.middle }

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

; CHECK-DAG: %__DTRT_struct.farouter.0 = type { %__DTRT_struct.outer.0, %__DTRT_struct.middle }
; CHECK-DAG: %__DTRT_struct.outer.0 = type { i32, i32, %__DTRT_struct.middle, %struct.middlec* }
; CHECK-DAG: %__DTRT_struct.middle = type { i32, %struct.bar*, i32 }

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

define i32 @use_middle1(%struct.middle.1* %m) {
  %f0 = getelementptr %struct.middle.1, %struct.middle.1* %m, i64 0, i32 0
  %v0 = load i32, i32* %f0
  %f1 = getelementptr %struct.middle.1, %struct.middle.1* %m, i64 0, i32 2
  %v1 = load i32, i32* %f1

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
