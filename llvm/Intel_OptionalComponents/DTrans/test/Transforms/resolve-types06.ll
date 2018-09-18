; RUN:  opt -S -o - -whole-program-assume -dtrans-resolvetypes %s | FileCheck %s

; This case differs from resolve-types04.ll in that the %struct.middle*
; member of %struct.outer is accessed. In this case, remapping %struct.outer
; to %struct.outer.0 would eliminate the function bitcast in the call to
; @use_outer in @f1 but it would introduce a function bitcast in the call to
; @use_middle so the remapping isn't done.

%struct.outer = type { i32, i32, %struct.middle* }
%struct.middle = type { i32, i32, %struct.bar }
%struct.bar = type { i32, i32 }
%struct.outer.0 = type { i32, i32, %struct.middle.1* }
%struct.middle.1 = type { i32, i32, %struct.foo* }
%struct.foo = type { i16, i16, i32 }

; CHECK-DAG: %struct.outer = type { i32, i32, %struct.middle* }
; CHECK-DAG: %struct.middle = type { i32, i32, %struct.bar }
; CHECK-DAG: %struct.bar = type { i32, i32 }
; CHECK-DAG: %struct.outer.0 = type { i32, i32, %struct.middle.1* }
; CHECK-DAG: %struct.middle.1 = type { i32, i32, %struct.foo* }
; CHECK-DAG: %struct.foo = type { i16, i16, i32 }

define void @f1(%struct.outer* %o) {
  %ppmiddle = getelementptr %struct.outer, %struct.outer* %o, i64 0, i32 2
  %pmiddle = load %struct.middle*, %struct.middle** %ppmiddle
  call void @use_middle(%struct.middle* %pmiddle)
  call void bitcast (void (%struct.outer.0*)* @use_outer to void (%struct.outer*)*)(%struct.outer* %o)
  ret void
}

; CHECK-LABEL: define{{.*}}void @f1(%struct.outer* %o) {
; CHECK:  %ppmiddle = getelementptr %struct.outer, %struct.outer* %o, i64 0,
; CHECK-SAME:                       i32 2
; CHECK:  %pmiddle = load %struct.middle*, %struct.middle** %ppmiddle
; CHECK:  call void @use_middle(%struct.middle* %pmiddle)
; CHECK:  call void bitcast (void (%struct.outer.0*)* @use_outer to
; CHECK-SAME:                void (%struct.outer*)*)(%struct.outer* %o)

define void @use_middle(%struct.middle* %m) {
  %pbar = getelementptr %struct.middle, %struct.middle* %m, i64 0, i32 2
  call void @use_bar(%struct.bar* %pbar)
  ret void
}

; CHECK-LABEL: define{{.+}}void @use_middle(%struct.middle* %m) {
; CHECK:  %pbar = getelementptr %struct.middle, %struct.middle* %m, i64 0, i32 2
; CHECK:  call void @use_bar(%struct.bar* %pbar)

define void @use_bar(%struct.bar* %b) {
  ret void
}

; CHECK-LABEL: define{{.+}}void @use_bar(%struct.bar* %b) {

define void @use_outer(%struct.outer.0* %o) {
  %ppmiddle = getelementptr %struct.outer.0, %struct.outer.0* %o, i64 0, i32 2
  %pmiddle = load %struct.middle.1*, %struct.middle.1** %ppmiddle
  call void @use_other_middle(%struct.middle.1* %pmiddle)
  ret void
}

; CHECK-LABEL: define{{.+}}void @use_outer(%struct.outer.0* %o) {
; CHECK:  %ppmiddle = getelementptr %struct.outer.0, %struct.outer.0* %o,
; CHECK-SAME:                       i64 0, i32 2
; CHECK:  %pmiddle = load %struct.middle.1*, %struct.middle.1** %ppmiddle
; CHECK:  call void @use_other_middle(%struct.middle.1* %pmiddle)

define void @use_other_middle(%struct.middle.1* %m) {
  ret void
}

; CHECK-LABEL: define{{.+}}void @use_other_middle(%struct.middle.1* %m) {
