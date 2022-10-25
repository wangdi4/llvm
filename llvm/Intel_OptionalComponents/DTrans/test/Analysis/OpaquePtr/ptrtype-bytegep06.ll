; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s

; Test identification of byte-flattened GEPs on a pointer which is also
; used as an 'i8**' type because the element zero of the structure is an
; i8*.

%struct.test = type { ptr, i64, i64, ptr, ptr, ptr, ptr, ptr }

define void @test(ptr "intel_dtrans_func_index"="1" %arg) !intel.dtrans.func.type !6 {
  %i = alloca { i64, ptr, ptr, ptr, ptr, ptr }, !intel_dtrans_type !7
  call void @llvm.memset.p0.i64(ptr %i, i8 0, i64 48, i1 false)
  store ptr null, ptr %arg
  %f1 = getelementptr inbounds i8, ptr %arg, i64 8
; CHECK: %f1 = getelementptr inbounds i8, ptr %arg, i64 8
; CHECK-NEXT:    LocalPointerInfo:
; CHECK-NEXT:      Aliased types:
; CHECK-NEXT:        i64*
; CHECK-NEXT:      Element pointees:
; CHECK-NEXT:        %struct.test @ 1

  store i64 -1, ptr %f1
  %f2 = getelementptr inbounds i8, ptr %arg, i64 16
; This also gets i8* because the use in the memcpy call.
; CHECK: %f2 = getelementptr inbounds i8, ptr %arg, i64 16
; CHECK-NEXT:   LocalPointerInfo:
; CHECK-NEXT:      Aliased types:
; CHECK-NEXT:        i64*
; CHECK-NEXT:        i8*
; CHECK-NEXT:      Element pointees:
; CHECK-NEXT:        %struct.test @ 2

  call void @llvm.memcpy.p0.p0.i64(ptr %f2, ptr %i, i64 48, i1 false)
  ret void
}

declare void @llvm.memset.p0.i64(ptr, i8, i64, i1)
declare void @llvm.memcpy.p0.p0.i64(ptr, ptr, i64, i1)

!1 = !{i8 0, i32 1}  ; i8*
!2 = !{i64 0, i32 0}  ; i64
!3 = !{i64 0, i32 1}  ; i64*
!4 = !{i32 0, i32 1}  ; i32*
!5 = !{%struct.test zeroinitializer, i32 1}  ; %struct.test*
!6 = distinct !{!5}
!7 = !{!"L", i32 6, !2, !3, !3, !4, !4, !4}  ; { i64, i64*, i64*, i32*, i32*, i32* }
!8 = !{!"S", %struct.test zeroinitializer, i32 8, !1, !2, !2, !3, !3, !4, !4, !4} ; { i8*, i64, i64, i64*, i64*, i32*, i32*, i32* }

!intel.dtrans.types = !{!8}

