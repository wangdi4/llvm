; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s

; Test pointer type recovery when loading the element-zero member of a
; nested structure using a pointer to the structure itself.



; In this case, element zero is loaded using different IR sequences that are all
; semantically equivalent. Check that the pointer type recovery captures the
; type of the underlying pointer that is being loaded.
%struct.test01outer = type { %struct.test01middle }
%struct.test01middle = type { %struct.test01inner }
%struct.test01inner = type { ptr }
%struct.test01inner_impl = type { i32, i32, i32 }

define internal void @test01(ptr "intel_dtrans_func_index"="1" %p) !intel.dtrans.func.type !6 {
  ; Loading the pointer stored at the first location of the nested structure.
  %elem_zero_addr1 = getelementptr %struct.test01outer, ptr %p, i64 0, i32 0, i32 0, i32 0
  %val1 = load ptr, ptr %elem_zero_addr1

  ; Also, loading the pointer stored at the first location of the nested
  ; structure.
  %elem_zero_addr2 = bitcast ptr %p to ptr
  %val2 = load ptr, ptr %elem_zero_addr2

  ; Also, loading the pointer stored at the first location of the nested
  ; structure.
  %elem_zero_addr3 = bitcast ptr %p to ptr
  %val3 = load i64, ptr %elem_zero_addr3

  ret void
}
; CHECK-LABEL: define internal void @test01
; CHECK: %val1 = load ptr, ptr %elem_zero_addr1
; CHECK-NEXT:  LocalPointerInfo:
; CHECK-NEXT:    Aliased types:
; CHECK-NEXT:      %struct.test01inner_impl*{{ *$}}
; CHECK-NEXT:    No element pointees.

; CHECK: %val2 = load ptr, ptr %elem_zero_addr2
; CHECK-NEXT:  LocalPointerInfo:
; CHECK-NEXT:    Aliased types:
; CHECK-NEXT:      %struct.test01inner_impl*{{ *$}}
; CHECK-NEXT:    No element pointees.

; CHECK: %val3 = load i64, ptr %elem_zero_addr3
; CHECK-NEXT:  LocalPointerInfo:
; CHECK-NEXT:    Aliased types:
; CHECK-NEXT:      %struct.test01inner_impl*{{ *$}}
; CHECK-NEXT:    No element pointees.


; Test with loading a pointer that is a non-aggregate type.
%struct.test02outer = type { %struct.test02middle }
%struct.test02middle = type { %struct.test02inner }
%struct.test02inner = type { %struct.test02inner_impl }
%struct.test02inner_impl = type { ptr, ptr }

define internal void @test02(ptr "intel_dtrans_func_index"="1" %p) !intel.dtrans.func.type !12 {
  %elem_zero_addr1 = getelementptr %struct.test02outer, ptr %p, i64 0, i32 0, i32 0, i32 0, i32 0
  %val1 = load ptr, ptr %elem_zero_addr1

  %elem_zero_addr2 = bitcast ptr %p to ptr
  %val2 = load ptr, ptr %elem_zero_addr2

  ret void
}
; CHECK-LABEL: define internal void @test02
; CHECK: %val1 = load ptr, ptr %elem_zero_addr1
; CHECK-NEXT:  LocalPointerInfo:
; CHECK-NEXT:    Aliased types:
; CHECK-NEXT:      i64*{{ *$}}
; CHECK-NEXT:    No element pointees.

; CHECK: %val2 = load ptr, ptr %elem_zero_addr2
; CHECK-NEXT:  LocalPointerInfo:
; CHECK-NEXT:    Aliased types:
; CHECK-NEXT:      i64*{{ *$}}
; CHECK-NEXT:    No element pointees.


; In this case, element zero the loaded value is not a pointer type, so should
; not result in local pointer containing an aggregate or pointer type for the
; load.
%struct.test03outer = type { %struct.test03middle }
%struct.test03middle = type { %struct.test03inner }
%struct.test03inner = type { %struct.test03inner_impl }
%struct.test03inner_impl = type { i64, i64 }

define internal void @test03(ptr "intel_dtrans_func_index"="1" %p) !intel.dtrans.func.type !18 {
  %elem_zero_addr1 = getelementptr %struct.test03outer, ptr %p, i64 0, i32 0, i32 0, i32 0, i32 0
  %val1 = load i64, ptr %elem_zero_addr1

  %elem_zero_addr2 = bitcast ptr %p to ptr
  %val2 = load i64, ptr %elem_zero_addr2

  ret void
}
; CHECK-LABEL: define internal void @test03
; CHECK: %val1 = load i64, ptr %elem_zero_addr1
; CHECK-NOT:   LocalPointerInfo:
; CHECK: %elem_zero_addr2 = bitcast
; CHECK: %val2 = load i64, ptr %elem_zero_addr2
; CHECK-NOT:   LocalPointerInfo:
; CHECK: ret void


!1 = !{%struct.test01middle zeroinitializer, i32 0}  ; %struct.test01middle
!2 = !{%struct.test01inner zeroinitializer, i32 0}  ; %struct.test01inner
!3 = !{%struct.test01inner_impl zeroinitializer, i32 1}  ; %struct.test01inner_impl*
!4 = !{i32 0, i32 0}  ; i32
!5 = !{%struct.test01outer zeroinitializer, i32 1}  ; %struct.test01outer*
!6 = distinct !{!5}
!7 = !{%struct.test02middle zeroinitializer, i32 0}  ; %struct.test02middle
!8 = !{%struct.test02inner zeroinitializer, i32 0}  ; %struct.test02inner
!9 = !{%struct.test02inner_impl zeroinitializer, i32 0}  ; %struct.test02inner_impl
!10 = !{i64 0, i32 1}  ; i64*
!11 = !{%struct.test02outer zeroinitializer, i32 1}  ; %struct.test02outer*
!12 = distinct !{!11}
!13 = !{%struct.test03middle zeroinitializer, i32 0}  ; %struct.test03middle
!14 = !{%struct.test03inner zeroinitializer, i32 0}  ; %struct.test03inner
!15 = !{%struct.test03inner_impl zeroinitializer, i32 0}  ; %struct.test03inner_impl
!16 = !{i64 0, i32 0}  ; i64
!17 = !{%struct.test03outer zeroinitializer, i32 1}  ; %struct.test03outer*
!18 = distinct !{!17}
!19 = !{!"S", %struct.test01outer zeroinitializer, i32 1, !1} ; { %struct.test01middle }
!20 = !{!"S", %struct.test01middle zeroinitializer, i32 1, !2} ; { %struct.test01inner }
!21 = !{!"S", %struct.test01inner zeroinitializer, i32 1, !3} ; { %struct.test01inner_impl* }
!22 = !{!"S", %struct.test01inner_impl zeroinitializer, i32 3, !4, !4, !4} ; { i32, i32, i32 }
!23 = !{!"S", %struct.test02outer zeroinitializer, i32 1, !7} ; { %struct.test02middle }
!24 = !{!"S", %struct.test02middle zeroinitializer, i32 1, !8} ; { %struct.test02inner }
!25 = !{!"S", %struct.test02inner zeroinitializer, i32 1, !9} ; { %struct.test02inner_impl }
!26 = !{!"S", %struct.test02inner_impl zeroinitializer, i32 2, !10, !10} ; { i64*, i64* }
!27 = !{!"S", %struct.test03outer zeroinitializer, i32 1, !13} ; { %struct.test03middle }
!28 = !{!"S", %struct.test03middle zeroinitializer, i32 1, !14} ; { %struct.test03inner }
!29 = !{!"S", %struct.test03inner zeroinitializer, i32 1, !15} ; { %struct.test03inner_impl }
!30 = !{!"S", %struct.test03inner_impl zeroinitializer, i32 2, !16, !16} ; { i64, i64 }

!intel.dtrans.types = !{!19, !20, !21, !22, !23, !24, !25, !26, !27, !28, !29, !30}
