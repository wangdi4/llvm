; RUN: opt -opaque-pointers -S -whole-program-assume -intel-libirc-allowed -passes=dtrans-normalizeop < %s 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; This test verifies that (GEP, 0, 0) is generated for use of %i5 in
; store and call instructions.

; CHECK:  %i5 = phi ptr [ %i3, %bb2 ], [ null, %bb1 ]
; CHECK:  %dtnorm = getelementptr %SchemaGrammar, ptr %i5, i64 0, i32 0
; CHECK:  store ptr %dtnorm, ptr %i6, align 8
; CHECK:  call void @bar(ptr %dtnorm)


%SGXMLScanner = type { %XMLScanner, i8, i32, i32, ptr }
%XMLScanner = type { i64, ptr }
%Grammar = type { %XSerializable }
%XSerializable = type { i64 }
%MemoryManager = type { ptr }
%SchemaGrammar = type { %Grammar, i64 }
%MemoryManagerImpl = type { %MemoryManager }

define void @_ZN11xercesc_2_712SGXMLScanner9scanResetERKNS_11InputSourceE(ptr "intel_dtrans_func_index"="1" %arg) personality ptr null !intel.dtrans.func.type !21 {
bb:
  br label %bb1

bb1:                                              ; preds = %bb
  %i = getelementptr inbounds %SGXMLScanner, ptr %arg, i64 0, i32 4
  br i1 false, label %bb2, label %bb4

bb2:                                              ; preds = %bb1
  %i3 = tail call ptr @_ZN11xercesc_2_77XMemorynwEmPNS_13MemoryManagerE(i64 0, ptr null)
  store ptr %i3, ptr %i, align 8
  br label %bb4

bb4:                                              ; preds = %bb2, %bb1
  %i5 = phi ptr [ %i3, %bb2 ], [ null, %bb1 ]
  %i6 = getelementptr inbounds %XMLScanner, ptr %arg, i64 0, i32 1
  store ptr %i5, ptr %i6, align 8
  call void @bar(ptr %i5)
  ret void
}

define void @bar(ptr "intel_dtrans_func_index"="1" %arg) !intel.dtrans.func.type !23 {
bb:
  %j = getelementptr inbounds %Grammar, ptr %arg, i64 0, i32 0
  ret void
}

declare ptr @_Znwm(i64)

define ptr @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm(ptr "intel_dtrans_func_index"="2" %arg, i64 %arg1) personality ptr null !intel.dtrans.func.type !18 {
bb:
  %i = call ptr @_Znwm(i64 %arg1)
  ret ptr %i
}

define ptr @_ZN11xercesc_2_77XMemorynwEmPNS_13MemoryManagerE(i64 %arg, ptr "intel_dtrans_func_index"="2" %arg1) !intel.dtrans.func.type !24 {
bb:
  %i = add i64 %arg, 8
  %i2 = tail call ptr @_ZN11xercesc_2_717MemoryManagerImpl8allocateEm(ptr %arg1, i64 %i)
  store ptr %arg1, ptr %i2, align 8
  %i3 = getelementptr inbounds i8, ptr %i2, i64 8
  ret ptr %i3
}

!intel.dtrans.types = !{!0, !4, !6, !8, !10, !12, !14}

!0 = !{!"S", %MemoryManager zeroinitializer, i32 1, !1}
!1 = !{!2, i32 2}
!2 = !{!"F", i1 true, i32 0, !3}
!3 = !{i32 0, i32 0}
!4 = !{!"S", %XSerializable zeroinitializer, i32 1, !5}
!5 = !{i64 0, i32 0}
!6 = !{!"S", %XMLScanner zeroinitializer, i32 2, !5, !7}
!7 = !{%Grammar zeroinitializer, i32 1}
!8 = !{!"S", %Grammar zeroinitializer, i32 1, !9}
!9 = !{%XSerializable zeroinitializer, i32 0}
!10 = !{!"S", %SchemaGrammar zeroinitializer, i32 2, !11, !5}
!11 = !{%Grammar zeroinitializer, i32 0}
!12 = !{!"S", %MemoryManagerImpl zeroinitializer, i32 1, !13}
!13 = !{%MemoryManager zeroinitializer, i32 0}
!14 = !{!"S", %SGXMLScanner zeroinitializer, i32 5, !15, !16, !3, !3, !17}
!15 = !{%XMLScanner zeroinitializer, i32 0}
!16 = !{i8 0, i32 0}
!17 = !{%SchemaGrammar zeroinitializer, i32 1}
!18 = distinct !{!19, !20}
!19 = !{i8 0, i32 1}
!20 = !{%MemoryManagerImpl zeroinitializer, i32 1}
!21 = distinct !{!22}
!22 = !{%SGXMLScanner zeroinitializer, i32 1}
!23 = distinct !{!7}
!24 = distinct !{!19, !25}
!25 = !{%MemoryManager zeroinitializer, i32 1}
