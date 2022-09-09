; UNSUPPORTED: enable-opaque-pointers
; RUN:  opt -S -o - -whole-program-assume -internalize -dtrans-resolvetypes %s | FileCheck %s
; RUN:  opt -S -o - -whole-program-assume -passes='internalize,dtrans-resolvetypes' %s | FileCheck %s

; This test is to verify that a global variable, whose type is a candidate for
; compatible type remapping, does not prevent the remapping when a
; zeroinitializer is used to initialize the variable.

; These types are compatible, and should be reampped.
%struct.outer = type { i32, i32, %struct.middle* }
%struct.outer.0 = type { i32, i32, %struct.middle.1* }

; These types are compatible but currently do not try to remap because
; there is no function bitcast involving them.
%struct.middle = type { i32,  %struct.inner*, i32 }
%struct.middle.1 = type { i32, %struct.inner.1*, i32 }

; non-equivalent/non-compatible types.
%struct.inner = type { i32*, i32 }
%struct.inner.1 = type { i16*, i32 }

; Global variable that is initialized. The use of a zeroinitializer should not
; prevent remapping of compatible types.
@var = global %struct.outer zeroinitializer

; The original types for all compatible types should all be preserved.
; CHECK-DAG: %__DTRT_struct.outer.0 = type { i32, i32, %struct.middle.1* }
; CHECK-DAG: %struct.middle = type { i32,  %struct.inner*, i32 }
; CHECK-DAG: %struct.middle.1 = type { i32, %struct.inner.1*, i32 }
; CHECK-DAG: %struct.inner = type { i32*, i32 }
; CHECK-DAG: %struct.inner.1 = type { i16*, i32 }

; Global variable should have remapped type
; CHECK: @var = internal global %__DTRT_struct.outer.0 zeroinitializer

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

