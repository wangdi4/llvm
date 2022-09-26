; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN:  opt -opaque-pointers -disable-output -whole-program-assume -dtrans-resolvetypes -debug-only=dtrans-resolvetypes %s 2>&1 | FileCheck %s
; RUN:  opt -opaque-pointers -disable-output -whole-program-assume -passes=dtrans-resolvetypes -debug-only=dtrans-resolvetypes %s 2>&1 | FileCheck %s

; Verify that the resolve types pass gets inhibited when opaque pointers are
; used, rather than crashing. The resolve types pass is deprecated and will
; be removed when switching to opaque pointers.

; CHECK: resolve-types inhibited: opaque pointers passes in use

%struct.outer = type { i32, i32, ptr }
%struct.middle = type { i32, i32, %struct.bar }
%struct.bar = type { i32, i32 }
%struct.outer.0 = type { i32, i32, %struct.middle.1 }
%struct.middle.1 = type { i32, i32, ptr }
%struct.foo = type { i16, i16, i32 }

define void @f1(ptr %o) {
  call void bitcast (ptr @use_outer to ptr)(ptr %o)
  ret void
}

define void @use_middle(ptr %m) {
  %pbar = getelementptr %struct.middle, ptr %m, i64 0, i32 2
  call void @use_bar(ptr %pbar)
  ret void
}

define void @use_bar(ptr %b) {
  ret void
}

define void @use_outer(ptr %o) {
  ret void
}
