; This test verifies that intel_dtrans_type metadata is not
; ignored when init list of @llvm.global_ctors is optimized in
; GlobalOpt pass.
;
; RUN: opt < %s -opaque-pointers -S -passes=globalopt 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: @llvm.global_ctors = appending global [0 x { i32, ptr, ptr }] zeroinitializer, !intel_dtrans_type ![[DT0:[0-9]+]]

; CHECK: ![[DT0]] = !{!"A", i32 0, ![[DT1:[0-9]+]]}
; CHECK: ![[DT1]] = !{![[DT2:[0-9]+]], i32 0}
; CHECK: ![[DT2]] = !{!"L", i32 3, ![[DT3:[0-9]+]], ![[DT4:[0-9]+]], ![[DT7:[0-9]+]]}
; CHECK: ![[DT3]] = !{i32 0, i32 0}
; CHECK: ![[DT4]] = !{![[DT5:[0-9]+]], i32 1}
; CHECK: ![[DT5]] = !{!"F", i1 false, i32 0, ![[DT6:[0-9]+]]}
; CHECK: ![[DT6]] = !{!"void", i32 0}
; CHECK: ![[DT7]] = !{i8 0, i32 1}


@llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @_GLOBAL__sub_I_DOMTypeInfoImpl.cpp, ptr null }], !intel_dtrans_type !0

define internal void @_GLOBAL__sub_I_DOMTypeInfoImpl.cpp() personality ptr null {
entry:
  ret void
}

!0 = !{!"A", i32 1, !1}
!1 = !{!2, i32 0}
!2 = !{!"L", i32 3, !3, !4, !7}
!3 = !{i32 0, i32 0}
!4 = !{!5, i32 1}
!5 = !{!"F", i1 false, i32 0, !6}
!6 = !{!"void", i32 0}
!7 = !{i8 0, i32 1}
