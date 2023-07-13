; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s

; Verify that the pointer type analyzer does not crash when a pointer to a
; structure type is addressed as a array type to compute an address of an
; element within a array stored in a nested structure.

%struct.x264_t = type { i32, i32, %struct.anon.10 }
%struct.anon.10 = type { i32, i32, i32, i32, [2 x [32 x ptr]], [2 x [32 x ptr]] }

; Function that takes a pointer to a %struct.x264_t type.
define void @test(ptr "intel_dtrans_func_index"="1" %in, i64 %in1, i64 %in2) !intel.dtrans.func.type !9 {
  ; Address into one of the arrays contained within the nested structure
  ; contained in the %struct.x264_t type.
  ;
  ; Within the type tracking collected by the PtrTypeAnalyzer, it is not
  ; currently supported to mark the outermost structure as having an unknown
  ; offset of the structure as being addressed for this GEP. Also, the result
  ; type of the GEP cannot be determined. Therefore, this case should cause
  ; the pointer information to be marked as "UNHANDLED", as there is not a
  ; way to know what is being accessed, even if the array type specified by
  ; the GEP is to be believed.
  %addr = getelementptr [2 x [32 x ptr]], ptr %in, i64 0, i64 %in1, i64 %in2
  store ptr null, ptr %addr
  ret void
}

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.anon.10 zeroinitializer, i32 0}  ; %struct.anon.10
!3 = !{!"A", i32 2, !4}  ; [2 x [32 x [2 x i16]*]]
!4 = !{!"A", i32 32, !5}  ; [32 x [2 x i16]*]
!5 = !{!6, i32 1}  ; [2 x i16]*
!6 = !{!"A", i32 2, !7}  ; [2 x i16]
!7 = !{i16 0, i32 0}  ; i16
!8 = !{%struct.x264_t zeroinitializer, i32 1}  ; %struct.x264_t*
!9 = distinct !{!8}
!10 = !{!"S", %struct.x264_t zeroinitializer, i32 3, !1, !1, !2} ; { i32, i32, %struct.anon.10 }
!11 = !{!"S", %struct.anon.10 zeroinitializer, i32 6, !1, !1, !1, !1, !3, !3} ; { i32, i32, i32, i32, [2 x [32 x [2 x i16]*]], [2 x [32 x [2 x i16]*]] }

!intel.dtrans.types = !{!10, !11}

; CHECK:   %addr = getelementptr [2 x [32 x ptr]], ptr %in, i64 0, i64 %in1, i64 %in2
; CHECK-NEXT:    LocalPointerInfo:
; CHECK-SAME: <DEPENDS ON UNHANDLED>
