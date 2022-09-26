; UNSUPPORTED: enable-opaque-pointers
; RUN:  opt -S -o - -whole-program-assume -dtrans-resolvetypes %s | FileCheck %s
; RUN:  opt -S -o - -whole-program-assume -passes=dtrans-resolvetypes %s | FileCheck %s

; This test is here to verify that compatible but not equivalent types are
; handled correctly when a type that is part of a compatibility set is involved
; in a bitcast of a type that is not a member of the set.

; This is derived from the example in resolve-types04.ll, but modified to
; contain a bitcast where a type with compatibility candidates has a function
; call bitcast between derived and base types, which are not of the same
; compatibility set.

%struct.middle = type { i32, i32, %struct.bar }
%struct.middle.1 = type { i32, i32, %struct.foo* }
%struct.bar = type { i32, i32 }
%struct.foo = type { i16, i16, i32 }

%struct.base = type { i32, i32, %struct.middle* }

; The types here contain a different number of members than the type that will
; be seen within a bitcast of a function call type.
%struct.derived = type { i32, i32, %struct.middle*, i32 }
%struct.derived.1 = type { i32, i32, %struct.middle.1*, i32 }

; CHECK-DAG: %struct.middle = type { i32, i32, %struct.bar }
; CHECK-DAG: %struct.middle.1 = type { i32, i32, %struct.foo* }
; CHECK-DAG: %struct.bar = type { i32, i32 }
; CHECK-DAG: %struct.foo = type { i16, i16, i32 }
; CHECK-DAG: %struct.derived = type { i32, i32, %struct.middle*, i32 }
; CHECK-DAG: %struct.derived.1 = type { i32, i32, %struct.middle.1*, i32 }

; The function call bitcast seen within this function is to create a cast
; involving a member of a compatibility set (%struct.derived) and a type that is
; not a part of that set. This will prevent the type from being remapped.
define void @f1(%struct.derived* %o) {
  call void bitcast (void (%struct.base*)* @use_base to void (%struct.derived*)*)(%struct.derived* %o)
  ret void
}

define void @use_middle(%struct.middle* %m) {
  %pbar = getelementptr %struct.middle, %struct.middle* %m, i64 0, i32 2
  call void @use_bar(%struct.bar* %pbar)
  ret void
}

define void @use_derived(%struct.derived.1* %m) {
  %pmiddle = getelementptr %struct.derived.1, %struct.derived.1* %m, i64 0, i32 2
  ret void
}

define void @use_bar(%struct.bar* %b) {
  ret void
}

define void @use_base(%struct.base* %d) {
  %pbase = getelementptr %struct.base, %struct.base* %d, i64 0, i32 1
  ret void
}
