; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE

; Test that pointer types are inferred for functions declared as returning i8*
; types when the result of one function call requires analysis of another function
; call, which also needs to be inferred..

%struct._ZTS10_NexusInfo._NexusInfo = type { i32, %struct._ZTS14_RectangleInfo._RectangleInfo, i64, %struct._ZTS12_PixelPacket._PixelPacket*, %struct._ZTS12_PixelPacket._PixelPacket*, i32, i16*, i64 }
%struct._ZTS14_RectangleInfo._RectangleInfo = type { i64, i64, i64, i64 }
%struct._ZTS12_PixelPacket._PixelPacket = type { i16, i16, i16, i16 }
%struct._ZTS14_ExceptionInfo._ExceptionInfo = type { i32, i32, i8*, i8*, i8*, i32, %struct._ZTS13SemaphoreInfo.SemaphoreInfo*, i64 }
%struct._ZTS13SemaphoreInfo.SemaphoreInfo = type { i64, i32, i64, i64 }


define "intel_dtrans_func_index"="1" %struct._ZTS10_NexusInfo._NexusInfo** @AcquirePixelCacheNexus(i64 %i0) !intel.dtrans.func.type !10 {
  ; When trying to infer the type returned by this call, examining the
  ; instructions that use the result will lead to analyzing another function
  ; call that returns i8*. This will result in the type being inferred as
  ; appearing to be:
  ; - 'i8*' due to the call return type
  ; - 'i8**' due to an i8* being stored into the memory location
  ; - '%struct._ZTS10_NexusInfo._NexusInfo**' due to the type returned by this
  ;   function.
  %i4 = tail call i8* @AcquireAlignedMemory(i64 %i0, i64 8)
  %i5 = bitcast i8* %i4 to %struct._ZTS10_NexusInfo._NexusInfo**

  ; This result should be inferred:
  ; - 'i8*' due to the call return type
  ; - '%struct._ZTS10_NexusInfo._NexusInfo*' due to being stored into a memory
  ;   location where the pointer address was inferred as
  ;   '%struct._ZTS10_NexusInfo._NexusInfo**'
  %i16 = tail call i8* @AcquireQuantumMemory(i64 %i0, i64 88)
  %i17 = bitcast i8* %i16 to %struct._ZTS10_NexusInfo._NexusInfo*
  store %struct._ZTS10_NexusInfo._NexusInfo* %i17, %struct._ZTS10_NexusInfo._NexusInfo** %i5

  %i28 = mul i64 %i0, 88
  %i29 = tail call i8* @ResetMagickMemory(i8* nonnull %i16, i32 0, i64 %i28)

  %i33 = load %struct._ZTS10_NexusInfo._NexusInfo*, %struct._ZTS10_NexusInfo._NexusInfo** %i5
  %i34 = getelementptr inbounds %struct._ZTS10_NexusInfo._NexusInfo, %struct._ZTS10_NexusInfo._NexusInfo* %i33, i64 1
  %i35 = getelementptr inbounds %struct._ZTS10_NexusInfo._NexusInfo*, %struct._ZTS10_NexusInfo._NexusInfo** %i5, i64 1
  store %struct._ZTS10_NexusInfo._NexusInfo* %i34, %struct._ZTS10_NexusInfo._NexusInfo** %i35

  ret %struct._ZTS10_NexusInfo._NexusInfo** %i5
}
; CHECK-NONOPAQUE: %i4 = tail call i8* @AcquireAlignedMemory(i64 %i0, i64 8)
; CHECK-OPAQUE: %i4 = tail call ptr @AcquireAlignedMemory(i64 %i0, i64 8)
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT:      Aliased types:
; CHECK-NEXT:        %struct._ZTS10_NexusInfo._NexusInfo**{{ *}}
; CHECK-NEXT:        i8*{{ *}}
; CHECK-NEXT:        i8**{{ *}}

; CHECK-NONOPAQUE: %i16 = tail call i8* @AcquireQuantumMemory(i64 %i0, i64 88)
; CHECK-OPAQUE: %i16 = tail call ptr @AcquireQuantumMemory(i64 %i0, i64 88)
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT:      Aliased types:
; CHECK-NEXT:        %struct._ZTS10_NexusInfo._NexusInfo*{{ *}}
; CHECK-NEXT:        i8*{{ *}}

declare !intel.dtrans.func.type !11 "intel_dtrans_func_index"="1" i8* @AcquireAlignedMemory(i64, i64)
declare !intel.dtrans.func.type !12  "intel_dtrans_func_index"="1" i8* @AcquireQuantumMemory(i64, i64)
declare !intel.dtrans.func.type !13  "intel_dtrans_func_index"="1" i8* @ResetMagickMemory(i8* "intel_dtrans_func_index"="2", i32, i64)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct._ZTS14_RectangleInfo._RectangleInfo zeroinitializer, i32 0}  ; %struct._ZTS14_RectangleInfo._RectangleInfo
!3 = !{i64 0, i32 0}  ; i64
!4 = !{%struct._ZTS12_PixelPacket._PixelPacket zeroinitializer, i32 1}  ; %struct._ZTS12_PixelPacket._PixelPacket*
!5 = !{i16 0, i32 1}  ; i16*
!6 = !{i16 0, i32 0}  ; i16
!7 = !{i8 0, i32 1}  ; i8*
!8 = !{%struct._ZTS13SemaphoreInfo.SemaphoreInfo zeroinitializer, i32 1}  ; %struct._ZTS13SemaphoreInfo.SemaphoreInfo*
!9 = !{%struct._ZTS10_NexusInfo._NexusInfo zeroinitializer, i32 2}  ; %struct._ZTS10_NexusInfo._NexusInfo**
!10 = distinct !{!9}
!11 = distinct !{!7}
!12 = distinct !{!7}
!13 = distinct !{!7, !7}
!14 = !{!"S", %struct._ZTS10_NexusInfo._NexusInfo zeroinitializer, i32 8, !1, !2, !3, !4, !4, !1, !5, !3} ; { i32, %struct._ZTS14_RectangleInfo._RectangleInfo, i64, %struct._ZTS12_PixelPacket._PixelPacket*, %struct._ZTS12_PixelPacket._PixelPacket*, i32, i16*, i64 }
!15 = !{!"S", %struct._ZTS14_RectangleInfo._RectangleInfo zeroinitializer, i32 4, !3, !3, !3, !3} ; { i64, i64, i64, i64 }
!16 = !{!"S", %struct._ZTS12_PixelPacket._PixelPacket zeroinitializer, i32 4, !6, !6, !6, !6} ; { i16, i16, i16, i16 }
!17 = !{!"S", %struct._ZTS14_ExceptionInfo._ExceptionInfo zeroinitializer, i32 8, !1, !1, !7, !7, !7, !1, !8, !3} ; { i32, i32, i8*, i8*, i8*, i32, %struct._ZTS13SemaphoreInfo.SemaphoreInfo*, i64 }
!18 = !{!"S", %struct._ZTS13SemaphoreInfo.SemaphoreInfo zeroinitializer, i32 4, !3, !1, !3, !3} ; { i64, i32, i64, i64 }

!intel.dtrans.types = !{!14, !15, !16, !17, !18}
