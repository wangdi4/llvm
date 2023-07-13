; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s

; Test that a getelementptr instruction that uses the pointer operand
; as an array does not crash due to the analysis having collected the
; type as a structure.

; TODO: Currently, this will result in pointers being marked as UNHANDLED. This
; may need to be improved in the future to infer the type.

%struct.varray_head_tag = type { i64, i64, i32, ptr, %struct.varray_data_tag }
%struct.varray_data_tag = type { [1 x i64] }

define internal void @test01(ptr "intel_dtrans_func_index"="1" %in, i64 %index) !intel.dtrans.func.type !6 {
  %cast_to_struct = bitcast ptr %in to ptr
  %field4 = getelementptr inbounds %struct.varray_head_tag, ptr %cast_to_struct, i64 0, i32 4
  %cast_to_array = bitcast ptr %field4 to ptr

  ; The information collected about the pointer operand in this GEP is a
  ; structure type, but it is being used as an array.
  %elem_addr = getelementptr inbounds [1 x ptr], ptr %cast_to_array, i64 0, i64 %index
  %ptr = load ptr, ptr %elem_addr
  tail call void @test02(ptr %ptr)
  ret void
}
; CHECK-LABEL: define internal void @test01
; CHECK:  %elem_addr = getelementptr inbounds [1 x ptr], ptr %cast_to_array, i64 0, i64 %index
; CHECK-NEXT: LocalPointerInfo:
; CHECK-SAME: UNHANDLED


define internal void @test02(ptr "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !7 {
  ret void
}

!1 = !{i64 0, i32 0}  ; i64
!2 = !{i32 0, i32 0}  ; i32
!3 = !{i8 0, i32 1}  ; i8*
!4 = !{%struct.varray_data_tag zeroinitializer, i32 0}  ; %struct.varray_data_tag
!5 = !{!"A", i32 1, !1}  ; [1 x i64]
!6 = distinct !{!3}
!7 = distinct !{!3}
!8 = !{!"S", %struct.varray_head_tag zeroinitializer, i32 5, !1, !1, !2, !3, !4} ; { i64, i64, i32, i8*, %union.varray_data_tag }
!9 = !{!"S", %struct.varray_data_tag zeroinitializer, i32 1, !5} ; { [1 x i64] }

!intel.dtrans.types = !{!8, !9}
