; RUN: opt -opaque-pointers -S -whole-program-assume -intel-libirc-allowed -passes=dtrans-normalizeop < %s 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; This test verifies that (GEP, 0, 0) is generated for use of %i in foo.

; CHECK: %a = tail call noundef ptr @MemAlloc(i64 noundef 16, ptr null)
; CHECK: %p = getelementptr inbounds %RefHashTableOf, ptr %a, i64 0, i32 1
; CHECK: %dtnorm = getelementptr %HashPtr, ptr %i, i64 0, i32 0
; CHECK: store ptr %dtnorm, ptr %p, align 8

%RefHashTableOf = type { i32, ptr }
%MemoryManager = type { ptr }
%OutOfMemoryException = type { i8 }
%HashBase = type { ptr }
%HashPtr = type { %HashBase }
%MemoryManagerImpl = type { %MemoryManager }
%XalanDummyMemoryManager = type { %MemoryManager }
%"std::bad_alloc" = type { %"std::exception" }
%"std::exception" = type { ptr }

define void @foo() personality ptr null {
  %i = invoke ptr @MemAlloc(i64 16, ptr null)
          to label %b2 unwind label %b5

b2:                                                ; preds = %b0
  invoke void @Ctor1(ptr %i)
          to label %b3 unwind label %b5

b3:                                                ; preds = %b2
  %a = tail call noundef ptr @MemAlloc(i64 noundef 16, ptr null)
  %p = getelementptr inbounds %RefHashTableOf, ptr %a, i64 0, i32 1
  store ptr %i, ptr %p, align 8
  br label %common.ret

common.ret:                                       ; preds = %b5, %b3
  ret void

b5:                                                ; preds = %b2, %b0
  %l = landingpad { ptr, i32 }
          cleanup
  br label %common.ret
}

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

!intel.dtrans.types = !{!0, !4, !6, !7, !9, !11, !12, !14, !15}

!0 = !{!"S", %MemoryManager zeroinitializer, i32 1, !1}
!1 = !{!2, i32 2}
!2 = !{!"F", i1 true, i32 0, !3}
!3 = !{i32 0, i32 0}
!4 = !{!"S", %OutOfMemoryException zeroinitializer, i32 1, !5}
!5 = !{i8 0, i32 0}
!6 = !{!"S", %HashBase zeroinitializer, i32 1, !1}
!7 = !{!"S", %HashPtr zeroinitializer, i32 1, !8}
!8 = !{%HashBase zeroinitializer, i32 0}
!9 = !{!"S", %MemoryManagerImpl zeroinitializer, i32 1, !10}
!10 = !{%MemoryManager zeroinitializer, i32 0}
!11 = !{!"S", %XalanDummyMemoryManager zeroinitializer, i32 1, !10}
!12 = !{!"S", %"std::bad_alloc" zeroinitializer, i32 1, !13}
!13 = !{%"std::exception" zeroinitializer, i32 0}
!14 = !{!"S", %"std::exception" zeroinitializer, i32 1, !1}
!15 = !{!"S", %RefHashTableOf zeroinitializer, i32 2, !3, !16}
!16 = !{%HashBase zeroinitializer, i32 1}
!17 = distinct !{!18, !19}
!18 = !{i8 0, i32 1}
!19 = !{%MemoryManager zeroinitializer, i32 1}
!20 = distinct !{!18, !21}
!21 = !{%XalanDummyMemoryManager zeroinitializer, i32 1}
!22 = distinct !{!23}
!23 = !{%HashPtr zeroinitializer, i32 1}
