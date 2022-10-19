; REQUIRES: asserts
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1

; Test the type collection for a GEP that addresses a field within a structure
; by using an offset that is a multiple of the size of a pointer when the GEP
; result is to a pointer to a structure type in an array.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.test = type { ptr, ptr, ptr, [14 x ptr], ptr, [14 x %struct.foo*], ptr }
%struct.foo = type { i32, i32 }

define void @test(ptr "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !8 {
  ; The normal way of accessing an element within an array that is within a
  ; structure.
  %elem = getelementptr %struct.test, ptr %in, i64 0, i32 5, i64 2
  %val = load ptr, ptr %elem

  ; Another way of accessing the array element by computing a 160-byte offset
  ; from the start of the structure.
  %also_elem = getelementptr ptr, ptr %in, i64 20
  store ptr %val, ptr %also_elem
  ret void
}

; CHECK: %elem = getelementptr %struct.test, ptr %in, i64 0, i32 5, i64 2
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT:   Aliased types:
; CHECK-NEXT:     %struct.foo**{{$}}
; CHECK-NEXT:   Element pointees:
; CHECK-NEXT:     [14 x %struct.foo*] @ 2 ElementOf: %struct.test@5

; TODO: The element pointee in this case is currently only tracked with one
; level of nesting, not all levels, like the above GEP.

; CHECK: %also_elem = getelementptr ptr, ptr %in, i64 20
; CHECK-NEXT:   LocalPointerInfo:
; CHECK-NEXT:     Aliased types:
; CHECK-NEXT:       %struct.foo**{{$}}
; CHECK-NEXT:     Element pointees:
; CHECK-NEXT:       %struct.test @ 5

!intel.dtrans.types = !{!9, !10}

!1 = !{i32 0, i32 1}  ; i32*
!2 = !{i64 0, i32 1}  ; i64*
!3 = !{!"A", i32 14, !4}  ; [14 x %struct.foo*]
!4 = !{%struct.foo zeroinitializer, i32 1}  ; %struct.foo*
!5 = !{i16 0, i32 1}  ; i16*
!6 = !{i32 0, i32 0}  ; i32
!7 = !{%struct.test zeroinitializer, i32 1}  ; %struct.test*
!8 = distinct !{!7}
!9 = !{!"S", %struct.test zeroinitializer, i32 7, !1, !1, !2, !3, !2, !3, !5} ; { i32*, i32*, i64*, [14 x %struct.foo*], i64*, [14 x %struct.foo*], i16* }
!10 = !{!"S", %struct.foo zeroinitializer, i32 2, !6, !6} ; { i32, i32 }

