; RUN: llvm-link  -opaque-pointers -S %s %S/Inputs/link-ctors-metadata-02a.ll | FileCheck %s

; @llvm.global_ctors in Inputs/link-ctors-metadata-02a.ll doesn't have
; intel_dtrans_type metadata. Tests that intel_dtrans_type metadata is
; ignored when intel_dtrans_type metadata is missing on some of the
; definitions of @llvm.global_ctors.

; CHECK: @llvm.global_ctors = appending global [2 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @f, ptr @v }, { i32, ptr, ptr } { i32 65534, ptr @g, ptr @u }]
; CHECK-NOT: intel_dtrans_type 

define void @f() {
  ret void
}

@v = linkonce global i8 42

@llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @f, ptr @v }], !intel_dtrans_type !1

!0 = !{i8 0, i32 1}
!1 = !{!"A", i32 1, !2}
!2 = !{!3, i32 0}
!3 = !{!"L", i32 3, !4, !5, !0}
!4 = !{i32 0, i32 0}
!5 = !{!6, i32 1}
!6 = !{!"F", i1 false, i32 0, !7}
!7 = !{!"void", i32 0}
