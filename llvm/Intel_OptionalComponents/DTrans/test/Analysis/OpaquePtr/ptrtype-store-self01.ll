; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s

; This test is to verify that a store of a pointer into the location that the
; pointer points to does not result in treating a type as being many levels of
; pointer indirection. For example:
;   store ptr %x, ptr %y
;
; If %x and %y use a common PHInode, then %x and %y are the same type, but the
; inference implies that %y is a pointer to the type of %x. Check that this does
; not cause type propagation that keeps creating additional levels of
; indirection.


%struct._ZTS8_Cluster._Cluster = type { ptr, %struct._ZTS13_ExtentPacket._ExtentPacket, %struct._ZTS13_ExtentPacket._ExtentPacket, %struct._ZTS13_ExtentPacket._ExtentPacket, i64, i64 }
%struct._ZTS13_ExtentPacket._ExtentPacket = type { float, i64, i64, i64 }

declare !intel.dtrans.func.type !6 ptr @AcquireMagickMemory(i64)

define i32 @SegmentImage() {
bb:
  br label %bb1

bb1:                                              ; preds = %bb2, %bb
  br label %bb2

bb2:                                              ; preds = %bb1
  br i1 false, label %bb3, label %bb1

bb3:                                              ; preds = %bb2
  br label %bb4

bb4:                                              ; preds = %bb3
  ret i32 0

bb5:                                              ; preds = %bb6
  br i1 false, label %bb7, label %bb6

bb6:                                              ; preds = %bb5
  br i1 false, label %bb9, label %bb5

bb7:                                              ; preds = %bb5
  br label %bb8

bb8:                                              ; preds = %bb7
  br label %bb10

bb9:                                              ; preds = %bb17, %bb6
  %i = phi ptr [ %i12, %bb6 ], [ null, %bb17 ]
  ret i32 0

bb10:                                             ; preds = %bb8
  br i1 false, label %bb13, label %bb11

bb11:                                             ; preds = %bb10
  %i12 = call ptr @AcquireMagickMemory(i64 0)
  store ptr %i12, ptr %i16, align 8
  br label %bb15

bb13:                                             ; preds = %bb10
  %i14 = call ptr @AcquireMagickMemory(i64 0)
  br label %bb15

bb15:                                             ; preds = %bb13, %bb11
  %i16 = phi ptr [ %i12, %bb11 ], [ %i14, %bb13 ]
; CHECK:  %i16 = phi ptr [ %i12, %bb11 ], [ %i14, %bb13 ]
; CHECK: LocalPointerInfo:
; CHECK:   Aliased types:
; CHECK:     %struct._ZTS8_Cluster._Cluster*{{ *}}
; CHECK:     i8*{{ *}}

  br label %bb17

bb17:                                             ; preds = %bb15
  %i18 = getelementptr inbounds %struct._ZTS8_Cluster._Cluster, ptr %i16, i64 0, i32 4
  br label %bb9
}

!intel.dtrans.types = !{!0, !4}

!0 = !{!"S", %struct._ZTS8_Cluster._Cluster zeroinitializer, i32 6, !1, !2, !2, !2, !3, !3}
!1 = !{%struct._ZTS8_Cluster._Cluster zeroinitializer, i32 1}
!2 = !{%struct._ZTS13_ExtentPacket._ExtentPacket zeroinitializer, i32 0}
!3 = !{i64 0, i32 0}
!4 = !{!"S", %struct._ZTS13_ExtentPacket._ExtentPacket zeroinitializer, i32 4, !5, !3, !3, !3}
!5 = !{float 0.000000e+00, i32 0}
!6 = distinct !{!7}
!7 = !{i8 0, i32 1}
