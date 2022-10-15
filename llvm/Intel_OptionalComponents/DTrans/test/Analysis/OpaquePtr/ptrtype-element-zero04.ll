; REQUIRES: asserts
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -dtrans-outofboundsok=false -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test pointer type collection when indexing into an element zero type of a
; structure using a pointer to the structure type. This case is similar to
; ptrtype-element-zero02.ll. However, the pointer used in the element-zero
; GEP access was dynamically allocated, so contains an i8* type, which will
; not contribute to identifying the result type of the GEP.

%struct._ZTS9_NodeInfo._NodeInfo = type { [16 x ptr], ptr, i64, i64 }
%struct._ZTS12_ColorPacket._ColorPacket = type { %struct._ZTS12_PixelPacket._PixelPacket, i16, i64 }
%struct._ZTS12_PixelPacket._PixelPacket = type { i16, i16, i16, i16 }

@globNodeInfo = internal global %struct._ZTS9_NodeInfo._NodeInfo zeroinitializer

define void @test1(i64 %idx) {
  %alloc = call ptr @malloc(i64 176)
  store ptr %alloc, ptr @globNodeInfo

  ; Index into the element zero type, which is an array. This should resolve to the
  ; element type stored in the array, rather than being marked as ''UNHANDLED''.
  %i278 = getelementptr inbounds [16 x ptr], ptr %alloc, i64 0, i64 %idx
; CHECK:      %i278 = getelementptr inbounds [16 x ptr], ptr %alloc, i64 0, i64 %idx
; CHECK-NEXT:   LocalPointerInfo:
; CHECK-NEXT:     Aliased types:
; CHECK-NEXT:       %struct._ZTS9_NodeInfo._NodeInfo**{{ *$}}
; CHECK-NEXT:     Element pointees:
; CHECK-NEXT:       [16 x %struct._ZTS9_NodeInfo._NodeInfo*] @ 0

  %i279 = load ptr, ptr %i278
; CHECK:      %i279 = load ptr, ptr %i278, align 8
; CHECK-NEXT:   LocalPointerInfo:
; CHECK-NEXT:     Aliased types:
; CHECK-NEXT:       %struct._ZTS9_NodeInfo._NodeInfo*{{ *$}}
; CHECK-NEXT:     No element pointees.

  %i280 = icmp eq ptr %i279, null
  br i1 %i280, label %exit, label %doit

doit:
  %i98 = getelementptr inbounds %struct._ZTS9_NodeInfo._NodeInfo, ptr %i279, i64 0, i32 1
  %i99 = load ptr, ptr %i98
  %i239 = getelementptr inbounds %struct._ZTS12_ColorPacket._ColorPacket, ptr %i99, i64 0
  %i240 = getelementptr inbounds %struct._ZTS12_PixelPacket._PixelPacket, ptr %i239, i64 0, i32 2
  %i241 = load i16, ptr %i240
  br label %exit

exit:
  ret void
}

declare !intel.dtrans.func.type !8 "intel_dtrans_func_index"="1" ptr @malloc(i64) #0

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }

!1 = !{!"A", i32 16, !2}  ; [16 x %struct._ZTS9_NodeInfo._NodeInfo*]
!2 = !{%struct._ZTS9_NodeInfo._NodeInfo zeroinitializer, i32 1}  ; %struct._ZTS9_NodeInfo._NodeInfo*
!3 = !{%struct._ZTS12_ColorPacket._ColorPacket zeroinitializer, i32 1}  ; %struct._ZTS12_ColorPacket._ColorPacket*
!4 = !{i64 0, i32 0}  ; i64
!5 = !{%struct._ZTS12_PixelPacket._PixelPacket zeroinitializer, i32 0}  ; %struct._ZTS12_PixelPacket._PixelPacket
!6 = !{i16 0, i32 0}  ; i16
!7 = !{i8 0, i32 1}  ; i8*
!8 = distinct !{!7}
!9 = !{!"S", %struct._ZTS9_NodeInfo._NodeInfo zeroinitializer, i32 4, !1, !3, !4, !4} ; { [16 x %struct._ZTS9_NodeInfo._NodeInfo*], %struct._ZTS12_ColorPacket._ColorPacket*, i64, i64 }
!10 = !{!"S", %struct._ZTS12_ColorPacket._ColorPacket zeroinitializer, i32 3, !5, !6, !4} ; { %struct._ZTS12_PixelPacket._PixelPacket, i16, i64 }
!11 = !{!"S", %struct._ZTS12_PixelPacket._PixelPacket zeroinitializer, i32 4, !6, !6, !6, !6} ; { i16, i16, i16, i16 }

!intel.dtrans.types = !{!9, !10, !11}
