; RUN:  opt -S -o - -whole-program-assume -dtrans-resolvetypes %s | FileCheck %s

; This test is here to verify that compatible but not equivalent types are
; handled correctly. When that transformation is fully implemented this
; will be converted to test the type mapping. For now, ResolveTypes only
; identifies compatible types but does not check to see if they can be
; safely remapped or attempt to remap them, so the test is instead checking
; debug output for the identification.

; The IR below is from a case where the LLVM IR linker mismatched types.
; In the first input module, %struct.outer was defined but %struct.middle
; was declared as opaque. The first module also defined @f1 as seen below
; but had only a declaration of @use_outer. In the second input module,
; %struct.middle was defined as it is seen below and the function @use_middle
; was defined. In the third input module, %struct.outer was defined as
; %struct.outer.0 is seen below and %struct.middle was defined as
; %struct.middle.1 is seen below. The third module provided the definition of
; @use_outer.
;
; This is an example of a kind of mismapping in the LLVM IR linker that is
; very common when using LTO with source code that uses templates.

%struct.outer = type { i32, i32, %struct.middle* }
%struct.middle = type { i32, i32, %struct.bar }
%struct.bar = type { i32, i32 }
%struct.outer.0 = type { i32, i32, %struct.middle.1* }
%struct.middle.1 = type { i32, i32, %struct.foo* }
%struct.foo = type { i16, i16, i32 }

; CHECK-NOT: %struct.outer
; CHECK-DAG: %__DTRT_struct.outer.0 = type { i32, i32, %struct.middle.1* }
; CHECK-DAG: %struct.middle = type { i32, i32, %struct.bar }
; CHECK-DAG: %struct.bar = type { i32, i32 }
; CHECK-DAG: %struct.middle.1 = type { i32, i32, %struct.foo* }
; CHECK-DAG: %struct.foo = type { i16, i16, i32 }

define void @f1(%struct.outer* %o) {
  call void bitcast (void (%struct.outer.0*)* @use_outer to void (%struct.outer*)*)(%struct.outer* %o)
  ret void
}

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


; These functions shouldn't be modified and will appear first.

; CHECK-LABEL: define{{.+}}void @use_middle(%struct.middle* %m)
; CHECK-LABEL: define{{.+}}void @use_bar(%struct.bar* %b)


; These functions will be cloned and appear at the end.

; CHECK-LABEL: define{{.+}}void @f1.1(%__DTRT_struct.outer.0* %o)
; CHECK: call void @use_outer.2(%__DTRT_struct.outer.0* %o)

; CHECK-LABEL: define{{.+}}void @use_outer.2(%__DTRT_struct.outer.0* %o)
