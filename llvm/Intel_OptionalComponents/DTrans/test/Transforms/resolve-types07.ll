; RUN:  opt -S -o - -whole-program-assume -dtrans-resolvetypes \
; RUN:      -debug-only=dtrans-resolvetypes-verbose,dtrans-resolvetypes-compat %s 2>&1 | FileCheck %s
; REQUIRES: asserts

; This test is here to verify that compatible but not equivalent types are
; handled correctly. When that transformation is fully implemented this
; will be converted to test the type mapping. For now, ResolveTypes only
; identifies compatible types but does not check to see if they can be
; safely remapped or attempt to remap them, so the test is instead checking
; debug output for the identification.

; This case differs from resolve-types04.ll in that the %struct.middle and
; %struct.middle.1 types are equivalent and will be remapped to one another.

%struct.outer = type { i32, i32, %struct.middle*, %struct.bar* }
%struct.middle = type { i32, i32, %struct.foo.1* }
%struct.foo.1 = type { i16, i16, i32 }
%struct.bar = type { i32, i32 }
%struct.outer.0 = type { i32, i32, %struct.middle.1*, %struct.foo* }
%struct.middle.1 = type { i32, i32, %struct.foo* }
%struct.foo = type { i16, i16, i32 }

define void @f1(%struct.outer* %o) {
  %ppmiddle = getelementptr %struct.outer, %struct.outer* %o, i64 0, i32 2
  %pmiddle = load %struct.middle*, %struct.middle** %ppmiddle
  call void @use_middle(%struct.middle* %pmiddle)
  call void bitcast (void (%struct.outer.0*)* @use_outer to void (%struct.outer*)*)(%struct.outer* %o)
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

; FIXME: The checks below are verifying debug output. When the transformation
;        is fully implemented these should be replaced with checks of the
;        actually corrected IR.

; CHECK-LABEL: compareTypeMembers(struct.outer, struct.outer.0)
; CHECK-NEXT: compareTypeMembers(struct.middle, struct.middle.1)
; CHECK-NEXT: All members equivalent or compatible.
; CHECK-NEXT: Element struct member match @ 2
; CHECK-NEXT: Type name mismatch @ 3
; CHECK-NEXT: StElemATy = struct.bar
; CHECK-NEXT: StElemBTy = struct.foo
; CHECK-NEXT: All members equivalent or compatible.
; CHECK-NEXT: resolve-types: Types are compatible.
; CHECK-NEXT:    Base: %struct.outer = type { i32, i32, %struct.middle*,
; CHECK-SAME:                                 %struct.bar* }
; CHECK-NEXT:    Cand: %struct.outer.0 = type { i32, i32, %struct.middle.1*,
; CHECK-SAME:                                   %struct.foo* }
; CHECK-NEXT: resolve-types: Types are not equivalent.
; CHECK-NEXT:    Base: %struct.outer = type { i32, i32, %struct.middle*,
; CHECK-SAME:                                 %struct.bar* }
; CHECK-NEXT:    Cand: %struct.outer.0 = type { i32, i32, %struct.middle.1*,
; CHECK-SAME:                                   %struct.foo* }

; CHECK-LABEL: DTRT-compat: Type data
; CHECK: ===== Group =====
; CHECK: %struct.outer = type { i32, i32, %struct.middle*, %struct.bar* }
; CHECK:   Leader non-scalar fields accessed: 2

; CHECK: struct.outer
; CHECK-NEXT:  Bitcast to:
; CHECK-NEXT:    struct.outer.0
; CHECK-NEXT:  Bitcast from: None
;
; CHECK: struct.outer.0
; CHECK-NEXT: %struct.outer.0 = type { i32, i32, %struct.middle.1*,
; CHECK-SAME:                          %struct.foo* }
; CHECK-NEXT:  Conflicting fields accessed: 2
; CHECK-NEXT:  Bitcast to: None
; CHECK-NEXT:  Bitcast from:
; CHECK-NEXT:    struct.outer
