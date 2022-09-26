; RUN: llvm-link  -opaque-pointers -S %s %S/Inputs/link-ctors-metadata-03a.ll | FileCheck %s

; @llvm.global_ctors doesn't have intel_dtrans_type metadata. Tests that
; intel_dtrans_type metadata is ignored when intel_dtrans_type metadata is
; missing on some of the definitions of @llvm.global_ctors.

; CHECK: @llvm.global_ctors = appending global [2 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @f, ptr @v }, { i32, ptr, ptr } { i32 65534, ptr @g, ptr @u }]
; CHECK-NOT: intel_dtrans_type

define void @f() {
  ret void
}

@v = linkonce global i8 42

@llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @f, ptr @v }]
