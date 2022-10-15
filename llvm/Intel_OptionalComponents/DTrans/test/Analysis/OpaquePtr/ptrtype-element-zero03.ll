; REQUIRES: asserts
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -dtrans-outofboundsok=false -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test pointer type collection when indexing into an element zero type of a
; structure using a pointer to the structure type. In this case, there is a
; nested structure, which has an array at element zero. It is possible that the
; last argument of the GEP is not a compiler constant. The type should resolve
; to the type of the array element.

%struct.outer = type { %struct._ZTS9_NodeInfo._NodeInfo }
%struct._ZTS9_NodeInfo._NodeInfo = type { [16 x ptr], ptr, i64, i64 }
%struct._ZTS12_ColorPacket._ColorPacket = type { %struct._ZTS12_PixelPacket._PixelPacket, i16, i64 }
%struct._ZTS12_PixelPacket._PixelPacket = type { i16, i16, i16, i16 }

define void @test1(%struct.outer* "intel_dtrans_func_index"="1" %arg, i64 %idx) !intel.dtrans.func.type !9 {

  ; Indexing into the element zero type, which is an array in the nested structure.
  ; This should resolve to the element type stored in the array, rather than being
  ; marked as 'UNHANDLED'.
  ; Note: Unknown array index accesses resolve to an array index 0 for the
  ; element pointee tracking.
  %i278 = getelementptr inbounds [16 x ptr], ptr %arg, i64 0, i64 %idx
; CHECK:      %i278 = getelementptr inbounds [16 x ptr], ptr %arg, i64 0, i64 %idx
; CHECK-NEXT:   LocalPointerInfo:
; CHECK-NEXT:     Aliased types:
; CHECK-NEXT:       %struct._ZTS9_NodeInfo._NodeInfo**{{ *$}}
; CHECK-NEXT:      Element pointees:
; CHECK-NEXT:        [16 x %struct._ZTS9_NodeInfo._NodeInfo*] @ 0

  %i279 = load ptr, ptr %i278
; CHECK:      %i279 = load ptr, ptr %i278, align 8
; CHECK-NEXT:   LocalPointerInfo:
; CHECK-NEXT:     Aliased types:
; CHECK-NEXT:       %struct._ZTS9_NodeInfo._NodeInfo*{{ *$}}
; CHECK-NEXT:     No element pointees.

  %i280 = icmp eq ptr %i279, null
  ret void
}

!1 = !{%struct._ZTS9_NodeInfo._NodeInfo zeroinitializer, i32 0}  ; %struct._ZTS9_NodeInfo._NodeInfo
!2 = !{!"A", i32 16, !3}  ; [16 x %struct._ZTS9_NodeInfo._NodeInfo*]
!3 = !{%struct._ZTS9_NodeInfo._NodeInfo zeroinitializer, i32 1}  ; %struct._ZTS9_NodeInfo._NodeInfo*
!4 = !{%struct._ZTS12_ColorPacket._ColorPacket zeroinitializer, i32 1}  ; %struct._ZTS12_ColorPacket._ColorPacket*
!5 = !{i64 0, i32 0}  ; i64
!6 = !{%struct._ZTS12_PixelPacket._PixelPacket zeroinitializer, i32 0}  ; %struct._ZTS12_PixelPacket._PixelPacket
!7 = !{i16 0, i32 0}  ; i16
!8 = !{%struct.outer zeroinitializer, i32 1}  ; %struct.outer*
!9 = distinct !{!8}
!10 = !{!"S", %struct.outer zeroinitializer, i32 1, !1} ; { %struct._ZTS9_NodeInfo._NodeInfo }
!11 = !{!"S", %struct._ZTS9_NodeInfo._NodeInfo zeroinitializer, i32 4, !2, !4, !5, !5} ; { [16 x %struct._ZTS9_NodeInfo._NodeInfo*], %struct._ZTS12_ColorPacket._ColorPacket*, i64, i64 }
!12 = !{!"S", %struct._ZTS12_ColorPacket._ColorPacket zeroinitializer, i32 3, !6, !7, !5} ; { %struct._ZTS12_PixelPacket._PixelPacket, i16, i64 }
!13 = !{!"S", %struct._ZTS12_PixelPacket._PixelPacket zeroinitializer, i32 4, !7, !7, !7, !7} ; { i16, i16, i16, i16 }

!intel.dtrans.types = !{!10, !11, !12, !13}
