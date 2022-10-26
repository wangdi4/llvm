; RUN: opt -opaque-pointers -S -whole-program-assume -intel-libirc-allowed -passes=dtrans-normalizeop < %s 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; This test verifies that (GEP, 0, 0) is generated for %i before using it in
; %i5.

; CHECK: %i = tail call ptr @Alloc(i64 40, ptr null)
; CHECK: %dtnorm = getelementptr %IconvTransService, ptr %i, i64 0, i32 0
; CHECK: %i5 = phi ptr [ %dtnorm, %bb1 ], [ null, %bb ]

%MemoryManager = type { ptr }
%OutOfMemoryException = type { i8 }
%XMLTransService = type { ptr }
%IconvTransService = type { %XMLTransService }
%MemoryManagerImpl = type { %MemoryManager }
%XalanDummyMemoryManager = type { %MemoryManager }
%"std::bad_alloc" = type { %"std::exception" }
%"std::exception" = type { ptr }

define "intel_dtrans_func_index"="1" ptr @foo() personality ptr null !intel.dtrans.func.type !15 {
bb:
  br label %bb4

bb1:                                              ; No predecessors!
  %i = tail call ptr @Alloc(i64 40, ptr null)
  invoke void @Ctor1(ptr %i)
          to label %bb4 unwind label %bb2

bb2:                                              ; preds = %bb1
  %i3 = landingpad { ptr, i32 }
          cleanup
  ret ptr null

bb4:                                              ; preds = %bb1, %bb
  %i5 = phi ptr [ %i, %bb1 ], [ null, %bb ]
  ret ptr %i5
}

declare ptr @__cxa_allocate_exception(i64)

declare void @__cxa_throw(ptr, ptr, ptr)

declare ptr @_Znwm(i64)

define ptr @Alloc(i64 %arg, ptr "intel_dtrans_func_index"="2" %arg1) !intel.dtrans.func.type !17 {
bb:
  %i = add i64 %arg, 8
  %i2 = tail call ptr @MemAlloc(ptr %arg1, i64 %i)
  store ptr %arg1, ptr %i2, align 8
  %i3 = getelementptr inbounds i8, ptr %i2, i64 8
  ret ptr %i3
}

define ptr @MemAlloc(ptr "intel_dtrans_func_index"="2" %arg, i64 %arg1) !intel.dtrans.func.type !20 {
bb:
  %i = call ptr @_Znwm(i64 %arg1)
  ret ptr %i
}

declare !intel.dtrans.func.type !22 void @Ctor1(ptr "intel_dtrans_func_index"="1")

!intel.dtrans.types = !{!0, !4, !6, !7, !9, !11, !12, !14}

!0 = !{!"S", %MemoryManager zeroinitializer, i32 1, !1}
!1 = !{!2, i32 2}
!2 = !{!"F", i1 true, i32 0, !3}
!3 = !{i32 0, i32 0}
!4 = !{!"S", %OutOfMemoryException zeroinitializer, i32 1, !5}
!5 = !{i8 0, i32 0}
!6 = !{!"S", %XMLTransService zeroinitializer, i32 1, !1}
!7 = !{!"S", %IconvTransService zeroinitializer, i32 1, !8}
!8 = !{%XMLTransService zeroinitializer, i32 0}
!9 = !{!"S", %MemoryManagerImpl zeroinitializer, i32 1, !10}
!10 = !{%MemoryManager zeroinitializer, i32 0}
!11 = !{!"S", %XalanDummyMemoryManager zeroinitializer, i32 1, !10}
!12 = !{!"S", %"std::bad_alloc" zeroinitializer, i32 1, !13}
!13 = !{%"std::exception" zeroinitializer, i32 0}
!14 = !{!"S", %"std::exception" zeroinitializer, i32 1, !1}
!15 = distinct !{!16}
!16 = !{%XMLTransService zeroinitializer, i32 1}
!17 = distinct !{!18, !19}
!18 = !{i8 0, i32 1}
!19 = !{%MemoryManager zeroinitializer, i32 1}
!20 = distinct !{!18, !21}
!21 = !{%XalanDummyMemoryManager zeroinitializer, i32 1}
!22 = distinct !{!23}
!23 = !{%IconvTransService zeroinitializer, i32 1}
