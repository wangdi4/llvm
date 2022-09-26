; UNSUPPORTED: enable-opaque-pointers
; RUN:  opt -S -o - -whole-program-assume -dtrans-resolvetypes %s | FileCheck %s
; RUN:  opt -S -o - -whole-program-assume -passes=dtrans-resolvetypes %s | FileCheck %s

; This test is to verify that resolve types considers conflicting types that
; are referenced in global variable initializers when deciding whether it is
; possible to remap a type. In this case there are nested types, so neither
; the outer type or the nested type should be allowed to be remapped.

; These types are compatible. The conflicting field will be accessed via a
; global variable initializer for one of the types, and via code for the other
; type. The resolve types pass would like to replace the type %struct.outer
; with the type %struct.outer.0 to eliminate the bitcast in @test1. It would
; also like to replace %struct.middle with the type %struct.middle.1. However,
; the resolve type pass does not support modifying types within global variables,
; so both of these remapping should be disallowed.

%struct.outer = type { i32, i32, %struct.middle }
%struct.outer.0 = type { i32, i32, %struct.middle.1 }

; These types are compatible but cannot be remapped because they have
; uses of the conflicting fields.
%struct.middle = type { i32,  %struct.inner*, i32 }
%struct.middle.1 = type { i32, %struct.inner.1*, i32 }

; non-equivalent/non-compatible types.
%struct.inner = type { i32*, i32 }
%struct.inner.1 = type { i16*, i32 }

; Global variable that is initialized, and prevents remapping compatible types
@var = global %struct.outer { i32 4, i32 8, %struct.middle { i32 0, %struct.inner* null, i32 1 } }

; The original types for all compatible types should all be preserved.
; CHECK-DAG: %struct.outer = type { i32, i32, %struct.middle }
; CHECK-DAG: %struct.outer.0 = type { i32, i32, %struct.middle.1 }
; CHECK-DAG: %struct.middle = type { i32,  %struct.inner*, i32 }
; CHECK-DAG: %struct.middle.1 = type { i32, %struct.inner.1*, i32 }
; CHECK-DAG: %struct.inner = type { i32*, i32 }
; CHECK-DAG: %struct.inner.1 = type { i16*, i32 }

define void @test1(%struct.outer* %o) {
  call void bitcast (void (%struct.outer.0*)* @use_outer0
                  to void (%struct.outer*)*)(%struct.outer* %o)
  ret void
}

define void @test2(%struct.middle* %m) {
  %a = call i32 bitcast (i32 (%struct.middle.1*)* @use_middle1
                      to i32 (%struct.middle*)*)(%struct.middle* %m)
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
  %mfield = getelementptr %struct.outer.0, %struct.outer.0* %o, i64 0, i32 2
  %maddr = getelementptr %struct.middle.1, %struct.middle.1* %mfield, i64 0, i32 0
  %m = load i32, i32* %maddr
  ret void
}

