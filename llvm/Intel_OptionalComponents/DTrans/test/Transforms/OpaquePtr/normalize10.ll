; RUN: opt -opaque-pointers -S -whole-program-assume -intel-libirc-allowed -passes=dtrans-normalizeop < %s 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; This test verifies that (GEP, 0, 0) is generated for %i6 before using it in
; %i7.

; CHECK: %i6 = phi ptr [ %i4, %bb3 ], [ %i2, %bb1 ]
; CHECK: %dtnorm = getelementptr %RefVectorOf, ptr %i6, i64 0, i32 0
; CHECK: %i7 = getelementptr inbounds %BaseRefVectorOf, ptr %dtnorm, i64 0, i32 2

%AbstractDOMParser = type { %XMLDocumentHandler, ptr }
%XMLDocumentHandler = type { i32 }
%BaseRefVectorOf = type { i64, i8, i32, i32, i64, i64 }
%MemoryManager = type { ptr }
%RefVectorOf = type { %BaseRefVectorOf }
%MemoryManagerImpl = type { %MemoryManager }

define void @foo(ptr "intel_dtrans_func_index"="1" %arg) !intel.dtrans.func.type !15 {
bb:
  br label %bb1

bb1:                                              ; preds = %bb
  %i = getelementptr inbounds %AbstractDOMParser, ptr %arg, i64 0, i32 1
  %i2 = load ptr, ptr %i, align 8
  br i1 false, label %bb3, label %bb5

bb3:                                              ; preds = %bb1
  %i4 = tail call ptr @Alloc(i64 40, ptr null)
  store ptr %i4, ptr %i, align 8
  br label %bb5

bb5:                                              ; preds = %bb3, %bb1
  %i6 = phi ptr [ %i4, %bb3 ], [ %i2, %bb1 ]
  %i7 = getelementptr inbounds %BaseRefVectorOf, ptr %i6, i64 0, i32 2
  ret void
}

declare ptr @_Znwm(i64)

define ptr @MemAlloc(ptr "intel_dtrans_func_index"="2" %arg, i64 %arg1) personality ptr null !intel.dtrans.func.type !17 {
bb:
  %i = call ptr @_Znwm(i64 %arg1)
  ret ptr %i
}

define ptr @Alloc(i64 %arg, ptr "intel_dtrans_func_index"="2" %arg1) !intel.dtrans.func.type !20 {
bb:
  %i = add i64 %arg, 8
  %i2 = tail call ptr @MemAlloc(ptr %arg1, i64 %i)
  store ptr %arg1, ptr %i2, align 8
  %i3 = getelementptr inbounds i8, ptr %i2, i64 8
  ret ptr %i3
}

!intel.dtrans.types = !{!0, !4, !7, !8, !10, !13}

!0 = !{!"S", %MemoryManager zeroinitializer, i32 1, !1}
!1 = !{!2, i32 2}
!2 = !{!"F", i1 true, i32 0, !3}
!3 = !{i32 0, i32 0}
!4 = !{!"S", %AbstractDOMParser zeroinitializer, i32 2, !5, !6}
!5 = !{%XMLDocumentHandler zeroinitializer, i32 0}
!6 = !{%RefVectorOf zeroinitializer, i32 1}
!7 = !{!"S", %XMLDocumentHandler zeroinitializer, i32 1, !3}
!8 = !{!"S", %RefVectorOf zeroinitializer, i32 1, !9}
!9 = !{%BaseRefVectorOf zeroinitializer, i32 0}
!10 = !{!"S", %BaseRefVectorOf zeroinitializer, i32 6, !11, !12, !3, !3, !11, !11}
!11 = !{i64 0, i32 0}
!12 = !{i8 0, i32 0}
!13 = !{!"S", %MemoryManagerImpl zeroinitializer, i32 1, !14}
!14 = !{%MemoryManager zeroinitializer, i32 0}
!15 = distinct !{!16}
!16 = !{%AbstractDOMParser zeroinitializer, i32 1}
!17 = distinct !{!18, !19}
!18 = !{i8 0, i32 1}
!19 = !{%MemoryManagerImpl zeroinitializer, i32 1}
!20 = distinct !{!18, !21}
!21 = !{%MemoryManager zeroinitializer, i32 1}
