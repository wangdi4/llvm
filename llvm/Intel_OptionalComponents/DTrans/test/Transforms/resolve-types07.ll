; RUN:  opt -S -o - -whole-program-assume -dtrans-resolvetypes %s | FileCheck %s

; This test verifies that we correctly handle the case where an outer type,
; which is not being remapped, is correctly updated when one of its elements
; is a pointer to a type that is being remapped, but another element is
; a pointer to a type that is not being remapped.

%struct.outer = type { i32, i32, %struct.middle*, %struct.bar* }
%struct.middle = type { i32, i32, %struct.foo.1* }
%struct.foo.1 = type { i16, i16, i32 }
%struct.bar = type { i32, i32 }
%struct.outer.0 = type { i32, i32, %struct.middle.1*, %struct.foo* }
%struct.middle.1 = type { i32, i32, %struct.foo* }
%struct.foo = type { i16, i16, i32 }

; CHECK-DAG: %__DTRT_struct.outer = type { i32, i32, %__DTRT_struct.middle*, %struct.bar* }
; CHECK-DAG: %__DTRT_struct.middle = type { i32, i32, %__DTRT_struct.foo* }
; CHECK-DAG: %__DTRT_struct.foo = type { i16, i16, i32 }
; CHECK-DAG: %struct.bar = type { i32, i32 }
; CHECK-DAG: %__DTRT_struct.outer.0 = type { i32, i32, %__DTRT_struct.middle*, %__DTRT_struct.foo* }

define void @f1(%struct.outer* %o) {
  %ppmiddle = getelementptr %struct.outer, %struct.outer* %o, i64 0, i32 2
  %pmiddle = load %struct.middle*, %struct.middle** %ppmiddle
  call void @use_middle(%struct.middle* %pmiddle)
  call void bitcast (void (%struct.outer.0*)* @use_outer to
                     void (%struct.outer*)*)(%struct.outer* %o)
  ret void
}

define void @use_middle(%struct.middle* %m) {
  ret void
}

define void @use_bar(%struct.bar* %b) {
  ret void
}

define void @use_outer(%struct.outer.0* %o) {
  %ppmiddle = getelementptr %struct.outer.0, %struct.outer.0* %o, i64 0, i32 2
  %pmiddle = load %struct.middle.1*, %struct.middle.1** %ppmiddle
  call void @use_other_middle(%struct.middle.1* %pmiddle)
  ret void
}

define void @use_other_middle(%struct.middle.1* %m) {
  ret void
}

; CHECK-LABEL: define{{.+}}void @use_bar(%struct.bar* %b) {

; CHECK-LABEL: define{{.+}}void @f1.1(%__DTRT_struct.outer* %o) {
; CHECK:  %ppmiddle = getelementptr %__DTRT_struct.outer,
; CHECK-SAME:                       %__DTRT_struct.outer* %o, i64 0, i32 2
; CHECK:  %pmiddle = load %__DTRT_struct.middle*,
; CHECK-SAME:             %__DTRT_struct.middle** %ppmiddle
; CHECK:  call void @use_middle.2(%__DTRT_struct.middle* %pmiddle)
; CHECK:  call void bitcast (void (%__DTRT_struct.outer.0*)* @use_outer.3 to
; CHECK-SAME:          void (%__DTRT_struct.outer*)*)(%__DTRT_struct.outer* %o)

; CHECK-LABEL: define{{.+}}void @use_middle.2(%__DTRT_struct.middle* %m) {

; CHECK-LABEL: define{{.+}}void @use_outer.3(%__DTRT_struct.outer.0* %o) {
; CHECK:  %ppmiddle = getelementptr %__DTRT_struct.outer.0,
; CHECK-SAME:                       %__DTRT_struct.outer.0* %o, i64 0, i32 2
; CHECK:  %pmiddle = load %__DTRT_struct.middle*,
; CHECK-SAME:             %__DTRT_struct.middle** %ppmiddle
; CHECK:  call void @use_other_middle.4(%__DTRT_struct.middle* %pmiddle)

; CHECK-LABEL: define{{.+}}void @use_other_middle.4(%__DTRT_struct.middle* %m) {
