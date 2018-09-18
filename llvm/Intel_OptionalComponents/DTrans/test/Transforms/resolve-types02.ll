; RUN:  opt -S -o - -whole-program-assume -dtrans-resolvetypes \
; RUN:      -debug-only=dtrans-resolvetypes-verbose,dtrans-resolvetypes-compat %s 2>&1 | FileCheck %s
; REQUIRES: asserts

; Verify that types which are similar but not compatible or equivalent are
; not incorrectly merged.

; Because the last elements of %struct.outer and %struct.outer.0 have different
; levels of indirection they cannot be combined.

%struct.outer = type { i32, i32, %struct.middle* }
%struct.middle = type { i32, i32, %struct.bar }
%struct.bar = type { i32, i32 }
%struct.outer.0 = type { i32, i32, %struct.middle.1 }
%struct.middle.1 = type { i32, i32, %struct.foo* }
%struct.foo = type { i16, i16, i32 }

; CHECK-DAG: %struct.outer = type { i32, i32, %struct.middle* }
; CHECK-DAG: %struct.middle = type { i32, i32, %struct.bar }
; CHECK-DAG: %struct.bar = type { i32, i32 }
; CHECK-DAG: %struct.outer.0 = type { i32, i32, %struct.middle.1 }
; CHECK-DAG: %struct.middle.1 = type { i32, i32, %struct.foo* }
; CHECK-DAG: %struct.foo = type { i16, i16, i32 }

define void @f1(%struct.outer* %o) {
  call void bitcast (void (%struct.outer.0*)* @use_outer to void (%struct.outer*)*)(%struct.outer* %o)
  ret void
}

; CHECK-LABEL: define{{.+}}void @f1(%struct.outer* %o) {
; CHECK: call void bitcast (void (%struct.outer.0*)* @use_outer to
; CHECK-SAME:               void (%struct.outer*)*)(%struct.outer* %o)

define void @use_middle(%struct.middle* %m) {
  %pbar = getelementptr %struct.middle, %struct.middle* %m, i64 0, i32 2
  call void @use_bar(%struct.bar* %pbar)
  ret void
}

define void @use_bar(%struct.bar* %b) {
  ret void
}

define void @use_outer(%struct.outer.0* %o) {
  ret void
}

; CHECK-LABEL: define{{.+}}void @use_outer(%struct.outer.0* %o)
