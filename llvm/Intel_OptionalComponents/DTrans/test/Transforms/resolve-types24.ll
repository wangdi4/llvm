; UNSUPPORTED: enable-opaque-pointers
; RUN:  opt -S -o - -whole-program-assume -dtrans-resolvetypes %s | FileCheck %s
; RUN:  opt -S -o - -whole-program-assume -passes=dtrans-resolvetypes %s | FileCheck %s

; This test is to verify that resolve types considers conflicting types that
; are referenced in global variable initializers when deciding whether it is
; possible to remap a type.

; These types are compatible. The conflicting field will be accessed via a
; global variable initializer for one of the types, and via code for the other
; type. The resolve types pass would like to replace the type %struct.outer
; with the type %struct.outer.0 to eliminate the bitcast in @test1. However,
; doing this remapping would also require %struct.middle to be replaced with
; %struct.middle.1, or having the remap types pass handle updating the
; the initializer of @var to rely on %struct.middle.1. This may be possible in
; some cases in the future, but is not currently supported by resolve types.
; Therefore, this remapping should be disallowed, otherwise an assertion will
; be hit when the base class tries to map the variable's initializer to a
; remapped type.
%struct.outer = type { i32, i32, %struct.middle* }
%struct.outer.0 = type { i32, i32, %struct.middle.1* }

; These types are compatible but currently do not try to remap because
; there is no function bitcast involving them.
%struct.middle = type { i32,  %struct.inner*, i32 }
%struct.middle.1 = type { i32, %struct.inner.1*, i32 }

; non-equivalent/non-compatible types.
%struct.inner = type { i32*, i32 }
%struct.inner.1 = type { i16*, i32 }

; Global variable that is initialized, and prevents remapping compatible types
@var = global %struct.outer { i32 4, i32 8, %struct.middle* null }

; The original types for all compatible types should all be preserved.
; CHECK-DAG: %struct.outer = type { i32, i32, %struct.middle* }
; CHECK-DAG: %struct.outer.0 = type { i32, i32, %struct.middle.1* }
; CHECK-DAG: %struct.middle = type { i32,  %struct.inner*, i32 }
; CHECK-DAG: %struct.middle.1 = type { i32, %struct.inner.1*, i32 }
; CHECK-DAG: %struct.inner = type { i32*, i32 }
; CHECK-DAG: %struct.inner.1 = type { i16*, i32 }

define void @test1(%struct.outer* %o) {
  call void bitcast (void (%struct.outer.0*)* @use_outer0
                  to void (%struct.outer*)*)(%struct.outer* %o)
  ret void
}

; Function to make sure there are uses of fields in %struct.middle.
define void @use_middle(%struct.middle* %m) {
  %f0 = getelementptr %struct.middle, %struct.middle* %m, i64 0, i32 0
  %v0 = load i32, i32* %f0
  ret void
}

; Function to make sure there are uses of fields in %struct.middle.1
define i32 @use_middle1(%struct.middle.1* %m) {
  %f0 = getelementptr %struct.middle.1, %struct.middle.1* %m, i64 0, i32 0
  %v0 = load i32, i32* %f0
  %f1 = getelementptr %struct.middle.1, %struct.middle.1* %m, i64 0, i32 2
  %v1 = load i32, i32* %f1
  %add = add i32 %v0, %v1
  ret i32 %add
}

; Function that will be called with bitcast argument.
define void @use_outer0(%struct.outer.0* %o) {
  %maddr = getelementptr %struct.outer.0, %struct.outer.0* %o, i64 0, i32 2
  %m = load %struct.middle.1*, %struct.middle.1** %maddr
  ret void
}

