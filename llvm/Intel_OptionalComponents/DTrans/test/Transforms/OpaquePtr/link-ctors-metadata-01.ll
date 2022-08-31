; RUN: llvm-link  -opaque-pointers -S %s %S/Inputs/link-ctors-metadata-01a.ll | FileCheck %s

; Tests that intel_dtrans_type metadata is not ignored when merging in
; global appending variables that have DTrans metadata attachments.

; CHECK: @llvm.global_ctors = appending global [2 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @f, ptr @v }, { i32, ptr, ptr } { i32 65534, ptr @g, ptr @u }], !intel_dtrans_type ![[DT0:[0-9]+]]

; CHECK: ![[DT0]] = !{!"A", i32 2, ![[DT1:[0-9]+]]}
; CHECK: ![[DT1]] = !{![[DT2:[0-9]+]], i32 0}
; CHECK: ![[DT2]] = !{!"L", i32 3, ![[DT3:[0-9]+]], ![[DT4:[0-9]+]], ![[DT7:[0-9]+]]}
; CHECK: ![[DT3]] = !{i32 0, i32 0}
; CHECK: ![[DT4]] = !{![[DT5:[0-9]+]], i32 1}
; CHECK: ![[DT5]] = !{!"F", i1 false, i32 0, ![[DT6:[0-9]+]]}
; CHECK: ![[DT6]] = !{!"void", i32 0}
; CHECK: ![[DT7]] = !{i8 0, i32 1}

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
