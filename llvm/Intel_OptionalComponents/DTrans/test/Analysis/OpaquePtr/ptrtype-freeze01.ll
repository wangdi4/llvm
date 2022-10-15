; REQUIRES: asserts
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -dtrans-outofboundsok=false -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test that the pointer type analyzer marks the result type of a 'freeze'
; instruction to be the same type as the input operand, rather than being
; treated as an unhandled instruction.

%struct._NexusInfo = type { i32, %struct._RectangleInfo, i64, ptr, ptr, i32, ptr, i64 }
%struct._RectangleInfo = type { i64, i64, i64, i64 }
%struct._PixelPacket = type { i16, i16, i16, i16 }

define void @test(ptr "intel_dtrans_func_index"="1" %arg1, i64 %idx) !intel.dtrans.func.type !8 {
  %i70 = getelementptr inbounds %struct._NexusInfo, ptr %arg1, i64 0, i32 4
  %i71 = load ptr, ptr %i70
  %i274 = freeze ptr %i71
  %i275 = icmp eq ptr %i274, null
  ret void
}

; CHECK: %i274 = freeze ptr %i71
; CHECK: LocalPointerInfo:
; CHECK-NOT: UNHANDLED
; CHECK:      Aliased types:
; CHECK:        %struct._PixelPacket*{{ *$}}
; CHECK:      No element pointees.

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct._RectangleInfo zeroinitializer, i32 0}  ; %struct._RectangleInfo
!3 = !{i64 0, i32 0}  ; i64
!4 = !{%struct._PixelPacket zeroinitializer, i32 1}  ; %struct._PixelPacket*
!5 = !{i16 0, i32 1}  ; i16*
!6 = !{i16 0, i32 0}  ; i16
!7 = !{%struct._NexusInfo zeroinitializer, i32 1}  ; %struct._NexusInfo*
!8 = distinct !{!7}
!9 = !{!"S", %struct._NexusInfo zeroinitializer, i32 8, !1, !2, !3, !4, !4, !1, !5, !3} ; { i32, %struct._RectangleInfo, i64, %struct._PixelPacket*, %struct._PixelPacket*, i32, i16*, i64 }
!10 = !{!"S", %struct._RectangleInfo zeroinitializer, i32 4, !3, !3, !3, !3} ; { i64, i64, i64, i64 }
!11 = !{!"S", %struct._PixelPacket zeroinitializer, i32 4, !6, !6, !6, !6} ; { i16, i16, i16, i16 }

!intel.dtrans.types = !{!9, !10, !11}
