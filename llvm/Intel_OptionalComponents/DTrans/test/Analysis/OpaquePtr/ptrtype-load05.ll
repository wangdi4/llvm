; REQUIRES: asserts

; RUN: opt -disable-output -whole-program-assume -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-CUR
; RUN: opt -disable-output -whole-program-assume -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-CUR

; Test pointer type recovery when loading the element-zero member of a
; nested structure using a pointer to the structure itself.

; Lines marked with CHECK-CUR are tests for the current form of IR.
; Lines marked with CHECK-FUT are placeholders for check lines that will
;   changed when the future opaque pointer form of IR is used.
; Lines marked with CHECK should remain the same when changing to use opaque
;   pointers.


; In this case, element zero is loaded using different IR sequences that are all
; semantically equivalent. Check that the pointer type recovery captures the
; type of the underlying pointer that is being loaded.
%struct.test01outer = type { %struct.test01middle }
%struct.test01middle = type { %struct.test01inner }
%struct.test01inner = type { %struct.test01inner_impl* }
%struct.test01inner_impl = type { i32, i32, i32 }

define internal void @test01(%struct.test01outer* %p) !dtrans_type !6 {
  ; Loading the pointer stored at the first location of the nested structure.
  %elem_zero_addr1 = getelementptr %struct.test01outer, %struct.test01outer* %p, i64 0, i32 0, i32 0, i32 0
  %val1 = load %struct.test01inner_impl*, %struct.test01inner_impl** %elem_zero_addr1

  ; Also, loading the pointer stored at the first location of the nested
  ; structure.
  %elem_zero_addr2 = bitcast %struct.test01outer* %p to %struct.test01inner_impl**
  %val2 = load %struct.test01inner_impl*, %struct.test01inner_impl** %elem_zero_addr2

  ; Also, loading the pointer stored at the first location of the nested
  ; structure.
  %elem_zero_addr3 = bitcast %struct.test01outer* %p to i64*
  %val3 = load i64, i64* %elem_zero_addr3

  ret void
}
; CHECK-LABEL: define internal void @test01
; CHECK-CUR: %val1 = load %struct.test01inner_impl*, %struct.test01inner_impl** %elem_zero_addr1
; CHECK-FUT: %val1 = load p0, p0 %elem_zero_addr1
; CHECK-NEXT:  LocalPointerInfo:
; CHECK-NEXT:    Aliased types:
; CHECK-NEXT:      %struct.test01inner_impl*{{ *$}}
; CHECK-NEXT:    No element pointees.

; CHECK-CUR: %val2 = load %struct.test01inner_impl*, %struct.test01inner_impl** %elem_zero_addr2
; CHECK-FUT: %val2 = load p0, p0 %elem_zero_addr2
; CHECK-NEXT:  LocalPointerInfo:
; CHECK-NEXT:    Aliased types:
; CHECK-NEXT:      %struct.test01inner_impl*{{ *$}}
; CHECK-NEXT:    No element pointees.

; CHECK-CUR: %val3 = load i64, i64* %elem_zero_addr3
; CHECK-FUT: %val3 = load i64, p0 %elem_zero_addr3
; CHECK-NEXT:  LocalPointerInfo:
; CHECK-NEXT:    Aliased types:
; CHECK-NEXT:      %struct.test01inner_impl*{{ *$}}
; CHECK-NEXT:    No element pointees.


; Test with loading a pointer that is a non-aggregate type.
%struct.test02outer = type { %struct.test02middle }
%struct.test02middle = type { %struct.test02inner }
%struct.test02inner = type { %struct.test02inner_impl }
%struct.test02inner_impl = type { i64*, i64* }

define internal void @test02(%struct.test02outer* %p) !dtrans_type !14 {
  %elem_zero_addr1 = getelementptr %struct.test02outer, %struct.test02outer* %p, i64 0, i32 0, i32 0, i32 0, i32 0
  %val1 = load i64*, i64** %elem_zero_addr1

  %elem_zero_addr2 = bitcast %struct.test02outer* %p to i64**
  %val2 = load i64*, i64** %elem_zero_addr2

  ret void
}
; CHECK-LABEL: define internal void @test02
; CHECK-CUR: %val1 = load i64*, i64** %elem_zero_addr1
; CHECK-FUT: %val1 = load p0, p0 %elem_zero_addr1
; CHECK-NEXT:  LocalPointerInfo:
; CHECK-NEXT:    Aliased types:
; CHECK-NEXT:      i64*{{ *$}}
; CHECK-NEXT:    No element pointees.

; CHECK-CUR: %val2 = load i64*, i64** %elem_zero_addr2
; CHECK-FUT: %val2 = load p0, p0 %elem_zero_addr2
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

define internal void @test03(%struct.test03outer* %p) !dtrans_type !21 {
  %elem_zero_addr1 = getelementptr %struct.test03outer, %struct.test03outer* %p, i64 0, i32 0, i32 0, i32 0, i32 0
  %val1 = load i64, i64* %elem_zero_addr1

  %elem_zero_addr2 = bitcast %struct.test03outer* %p to i64*
  %val2 = load i64, i64* %elem_zero_addr2

  ret void
}
; CHECK-LABEL: define internal void @test03
; CHECK-CUR: %val1 = load i64, i64* %elem_zero_addr1
; CHECK-FUT: %val1 = load i64, p0 %elem_zero_addr1
; CHECK-NOT:   LocalPointerInfo:
; CHECK: %elem_zero_addr2 = bitcast
; CHECK-CUR: %val2 = load i64, i64* %elem_zero_addr2
; CHECK-FUT: %val2 = load i64, p0 %elem_zero_addr2
; CHECK-NOT:   LocalPointerInfo:
; CHECK: ret void


!1 = !{!"R", %struct.test01middle zeroinitializer, i32 0}  ; %struct.test01middle
!2 = !{!"R", %struct.test01inner zeroinitializer, i32 0}  ; %struct.test01inner
!3 = !{!4, i32 1}  ; %struct.test01inner_impl*
!4 = !{!"R", %struct.test01inner_impl zeroinitializer, i32 0}  ; %struct.test01inner_impl
!5 = !{i32 0, i32 0}  ; i32
!6 = !{!"F", i1 false, i32 1, !7, !8}  ; void (%struct.test01outer*)
!7 = !{!"void", i32 0}  ; void
!8 = !{!9, i32 1}  ; %struct.test01outer*
!9 = !{!"R", %struct.test01outer zeroinitializer, i32 0}  ; %struct.test01outer
!10 = !{!"R", %struct.test02middle zeroinitializer, i32 0}  ; %struct.test02middle
!11 = !{!"R", %struct.test02inner zeroinitializer, i32 0}  ; %struct.test02inner
!12 = !{!"R", %struct.test02inner_impl zeroinitializer, i32 0}  ; %struct.test02inner_impl
!13 = !{i64 0, i32 1}  ; i64*
!14 = !{!"F", i1 false, i32 1, !7, !15}  ; void (%struct.test02outer*)
!15 = !{!16, i32 1}  ; %struct.test02outer*
!16 = !{!"R", %struct.test02outer zeroinitializer, i32 0}  ; %struct.test02outer
!17 = !{!"R", %struct.test03middle zeroinitializer, i32 0}  ; %struct.test03middle
!18 = !{!"R", %struct.test03inner zeroinitializer, i32 0}  ; %struct.test03inner
!19 = !{!"R", %struct.test03inner_impl zeroinitializer, i32 0}  ; %struct.test03inner_impl
!20 = !{i64 0, i32 0}  ; i64
!21 = !{!"F", i1 false, i32 1, !7, !22}  ; void (%struct.test03outer*)
!22 = !{!23, i32 1}  ; %struct.test03outer*
!23 = !{!"R", %struct.test03outer zeroinitializer, i32 0}  ; %struct.test03outer
!24 = !{!"S", %struct.test01outer zeroinitializer, i32 1, !1} ; { %struct.test01middle }
!25 = !{!"S", %struct.test01middle zeroinitializer, i32 1, !2} ; { %struct.test01inner }
!26 = !{!"S", %struct.test01inner zeroinitializer, i32 1, !3} ; { %struct.test01inner_impl* }
!27 = !{!"S", %struct.test01inner_impl zeroinitializer, i32 3, !5, !5, !5} ; { i32, i32, i32 }
!28 = !{!"S", %struct.test02outer zeroinitializer, i32 1, !10} ; { %struct.test02middle }
!29 = !{!"S", %struct.test02middle zeroinitializer, i32 1, !11} ; { %struct.test02inner }
!30 = !{!"S", %struct.test02inner zeroinitializer, i32 1, !12} ; { %struct.test02inner_impl }
!31 = !{!"S", %struct.test02inner_impl zeroinitializer, i32 2, !13, !13} ; { i64*, i64* }
!32 = !{!"S", %struct.test03outer zeroinitializer, i32 1, !17} ; { %struct.test03middle }
!33 = !{!"S", %struct.test03middle zeroinitializer, i32 1, !18} ; { %struct.test03inner }
!34 = !{!"S", %struct.test03inner zeroinitializer, i32 1, !19} ; { %struct.test03inner_impl }
!35 = !{!"S", %struct.test03inner_impl zeroinitializer, i32 2, !20, !20} ; { i64, i64 }

!dtrans_types = !{!24, !25, !26, !27, !28, !29, !30, !31, !32, !33, !34, !35}
