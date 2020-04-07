; REQUIRES: asserts

; RUN: opt -disable-output -whole-program-assume -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-CUR
; RUN: opt -disable-output -whole-program-assume -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-CUR

; Test pointer type recovery on getelementptr instructions involving 2 or
; more indices

; Lines marked with CHECK-CUR are tests for the current form of IR.
; Lines marked with CHECK-FUT are placeholders for check lines that will
;   changed when the future opaque pointer form of IR is used.
; Lines marked with CHECK should remain the same when changing to use opaque
;   pointers.

; Test accesses type detection of structure with simple GEP.
%struct.test01 = type { i64, %struct.test01* }
define internal void @test01(%struct.test01* %in) !dtrans_type !4 {
  %f0 = getelementptr %struct.test01, %struct.test01* %in, i64 0, i32 0
  %v0 = load i64, i64* %f0

  %f1 = getelementptr %struct.test01, %struct.test01* %in, i64 0, i32 1
  %v1 = load %struct.test01*, %struct.test01** %f1

  ret void
}

; CHECK-LABEL: void @test01
; CHECK-CUR: %f0 = getelementptr %struct.test01, %struct.test01* %in, i64 0, i32 0
; CHECK-FUT: %f0 = getelementptr %struct.test01, p0 %in, i64 0, i32 0
; CHECK:    LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK-NEXT:   i64*
; CHECK-NEXT: Element pointees:
; CHECK-NEXT:   %struct.test01 @ 0

; CHECK-CUR: %v0 = load i64, i64* %f0
; CHECK-FUT: %v0 = load i64, p0 %f0
; CHECK-CUR: %f1 = getelementptr %struct.test01, %struct.test01* %in, i64 0, i32 1
; CHECK-FUT: %f1 = getelementptr %struct.test01, p0 %in, i64 0, i32 1
; CHECK:    LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK-NEXT:   %struct.test01**
; CHECK-NEXT: Element pointees:
; CHECK-NEXT:    %struct.test01 @ 1

; CHECK-CUR: %v1 = load %struct.test01*, %struct.test01** %f1
; CHECK-FUT: %v1 = load p0, p0 %f1
; CHECK:    LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK-NEXT:   %struct.test01*
; CHECK-NEXT: No element pointees.


; Test access with literal structure
@test_var02 = internal global { i64, i32 } zeroinitializer
define internal i32 @test02() {
  %f0 = getelementptr inbounds { i64, i32 }, { i64, i32 }* @test_var02, i64 0, i32 0
  %f1 = getelementptr inbounds { i64, i32 }, { i64, i32 }* @test_var02, i64 0, i32 1
  %v0 = load i64, i64* %f0
  %v1 = load i32, i32* %f1
  ret i32 %v1
}
; CHECK-LABEL: i32 @test02()
; CHECK-CUR:   %f0 = getelementptr inbounds { i64, i32 }, { i64, i32 }* @test_var02, i64 0, i32 0
; CHECK-FUT:   %f0 = getelementptr inbounds { i64, i32 }, p0 @test_var02, i64 0, i32 0
; CHECK:    LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK-NEXT:   i64*
; CHECK-NEXT: Element pointees:
; CHECK-NEXT:  { i64, i32 } @ 0

; CHECK-CUR:   %f1 = getelementptr inbounds { i64, i32 }, { i64, i32 }* @test_var02, i64 0, i32 1
; CHECK-FUT:   %f1 = getelementptr inbounds { i64, i32 }, p0 @test_var02, i64 0, i32 1
; CHECK:    LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK-NEXT:   i32*
; CHECK-NEXT: Element pointees:
; CHECK-NEXT:   { i64, i32 } @ 1


; Test access to single dimension array element
@test_var3 = internal global [20 x i64] zeroinitializer
define internal i64 @test03() {
  %ar8 =  getelementptr inbounds [20 x i64], [20 x i64]* @test_var3, i64 0, i32 8
  %v8 = load i64, i64*%ar8
  ret i64 %v8
}
; CHECK-LABEL: i64 @test03
; CHECK-CUR:  %ar8 = getelementptr inbounds [20 x i64], [20 x i64]* @test_var3, i64 0, i32 8
; CHECK-FUT:  %ar8 = getelementptr inbounds [20 x i64], p0 @test_var3, i64 0, i32 8
; CHECK:    LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK-NEXT:   i64*
; CHECK-NEXT: Element pointees:
; CHECK-NEXT:   [20 x i64] @ 8


; Test access to multi-dimension array element when using constant and
; non-constant indices.
@test_var4 = internal global [6 x [8 x [12 x i64]]] zeroinitializer
define internal i64 @test04(i32 %x, i32 %y) {
  %ar1 = getelementptr inbounds [6 x [8 x [12 x i64]]], [6 x [8 x [12 x i64]]]* @test_var4, i64 0, i32 2, i32 3, i32 7
  %v1 = load i64, i64* %ar1

  %ar2 = getelementptr inbounds [6 x [8 x [12 x i64]]], [6 x [8 x [12 x i64]]]* @test_var4, i64 0, i32 %x, i32 %y, i32 7
  %v2 = load i64, i64* %ar2

  ret i64 %v2
}
; CHECK-LABEL: i64 @test04
; CHECK-CUR:  %ar1 = getelementptr inbounds [6 x [8 x [12 x i64]]], [6 x [8 x [12 x i64]]]* @test_var4, i64 0, i32 2, i32 3, i32 7
; CHECK-FUT:  %ar1 = getelementptr inbounds [6 x [8 x [12 x i64]]], p0 @test_var4, i64 0, i32 2, i32 3, i32 7
; CHECK:    LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK-NEXT:   i64*
; CHECK-NEXT: Element pointees:
; CHECK-NEXT:   [12 x i64] @ 7

; CHECK-CUR:  %ar2 = getelementptr inbounds [6 x [8 x [12 x i64]]], [6 x [8 x [12 x i64]]]* @test_var4, i64 0, i32 %x, i32 %y, i32 7
; CHECK-FUT:  %ar2 = getelementptr inbounds [6 x [8 x [12 x i64]]], p0 @test_var4, i64 0, i32 %x, i32 %y, i32 7
; CHECK:    LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK-NEXT:   i64*
; CHECK-NEXT: Element pointees:
; CHECK-NEXT:   [12 x i64] @ 7


; Test access with nested structure element
%struct.test05inner = type { i64, i32, i64* }
%struct.test05mid = type { i64, i64, %struct.test05inner }
%struct.test05outer = type { i64*, %struct.test05mid }
define internal i64 @test05(%struct.test05outer* %in) !dtrans_type !10 {
  %addr0 = getelementptr inbounds %struct.test05outer, %struct.test05outer* %in, i64 0, i32 1, i32 2, i32 0
  %val0 = load i64, i64* %addr0
  %addr2 = getelementptr inbounds %struct.test05outer, %struct.test05outer* %in, i64 0, i32 1, i32 2, i32 2
  %ptr = load i64*, i64** %addr2
  %val2 = load i64, i64* %ptr
  ret i64 %val0
}
; CHECK-LABEL: i64 @test05
; CHECK-CUR: %addr0 = getelementptr inbounds %struct.test05outer, %struct.test05outer* %in, i64 0, i32 1, i32 2, i32 0
; CHECK-FUT: %addr0 = getelementptr inbounds %struct.test05outer, p0 %in, i64 0, i32 1, i32 2, i32 0
; CHECK:    LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK-NEXT:   i64*
; CHECK-NEXT: Element pointees:
; CHECK-NEXT:   %struct.test05inner @ 0

; CHECK-CUR: %addr2 = getelementptr inbounds %struct.test05outer, %struct.test05outer* %in, i64 0, i32 1, i32 2, i32 2
; CHECK-FUT: %addr2 = getelementptr inbounds %struct.test05outer, p0 %in, i64 0, i32 1, i32 2, i32 2
; CHECK:    LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK-NEXT:   i64**
; CHECK-NEXT: Element pointees:
; CHECK-NEXT:   %struct.test05inner @ 2

; CHECK-CUR: %ptr = load i64*, i64** %addr2
; CHECK-FUT: %ptr = load p0, p0 %addr2
; CHECK:    LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK-NEXT:   i64*
; CHECK-NEXT:  No element pointees.


; Test access to an element of an array member of a structure.
; TODO: The legacy LocalPointerAnalyzer does not include struct.test06 @ 1 as
; an element pointee, and therefore does not track that field as being used.
; The PointerTypeAnalyzer will behave the same for tracking.
; Because none of the transformations are operating on array fields, this is
; safe.
%struct.test06 = type { i32, [20 x i64] }
define internal i64 @test06(%struct.test06* %in) !dtrans_type !14 {
  %elem_addr = getelementptr inbounds %struct.test06, %struct.test06* %in, i64 0, i32 1, i64 4
  %val = load i64, i64* %elem_addr
  ret i64 %val
}
; CHECK-LABEL: i64 @test06
; CHECK-CUR: %elem_addr = getelementptr inbounds %struct.test06, %struct.test06* %in, i64 0, i32 1, i64 4
; CHECK-FUT: %elem_addr = getelementptr inbounds %struct.test06, p0 %in, i64 0, i32 1, i64 4
; CHECK:    LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK-NEXT:   i64*
; CHECK-NEXT: Element pointees:
; CHECK-NEXT:   [20 x i64] @ 4


; Test access to an element that is an array of pointers
%struct.test07 = type { i64, i64 }
@var07 = internal global [16 x %struct.test07*] zeroinitializer, !dtrans_type !17
define internal void @test07() {
  %addr = getelementptr [16 x %struct.test07*], [16 x %struct.test07*]* @var07, i64 0, i32 2
  %sptr = load %struct.test07*, %struct.test07** %addr
  ret void
}
; CHECK-LABEL: void @test07
; CHECK-CUR: %addr = getelementptr [16 x %struct.test07*], [16 x %struct.test07*]* @var07, i64 0, i32 2
; CHECK-FUT: %addr = getelementptr [16 x p0], p0 @var07, i64 0, i32 2
; CHECK:    LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK-NEXT:   %struct.test07**
; CHECK-NEXT: Element pointees:
; CHECK-NEXT:   [16 x %struct.test07*] @ 2

; CHECK-CUR: %sptr = load %struct.test07*, %struct.test07** %addr
; CHECK-FUT: %sptr = load p0, p0 %addr
; CHECK:    LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK-NEXT:   %struct.test07*
; CHECK-NEXT:  No element pointees.


; Test access to a structure member from an array of structures
%struct.test08 = type { i64, i64 }
@var08 = internal global [16 x %struct.test08] zeroinitializer
define internal void @test08() {
  %addr = getelementptr [16 x %struct.test08], [16 x %struct.test08]* @var08, i64 0, i32 5, i32 1
  %sptr = load i64, i64* %addr
  ret void
}
; CHECK-LABEL: void @test08
; CHECK-CUR: %addr = getelementptr [16 x %struct.test08], [16 x %struct.test08]* @var08, i64 0, i32 5, i32 1
; CHECK-FUT: %addr = getelementptr [16 x %struct.test08], p0 @var08, i64 0, i32 5, i32 1
; CHECK:    LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK-NEXT:   i64*
; CHECK-NEXT: Element pointees:
; CHECK-NEXT:   %struct.test08 @ 1


; Test access to vector element to produce a vector of pointers. In this
; case the result of the GEP is a vector of pointers of the same type
define internal void @test09(<4 x i64*>* %ptr_to_vec) !dtrans_type !20 {
  %ptrs = load <4 x i64*>, <4 x i64*>* %ptr_to_vec
  %addr = getelementptr i64, <4 x i64*> %ptrs, <4 x i64> <i64 1, i64 2, i64 2, i64 1>
  %a2 = extractelement <4 x i64*> %addr, i32 2
  %v = load i64, i64* %a2
  ret void
}
; CHECK-LABEL: void @test09
; CHECK-CUR: %ptrs = load <4 x i64*>, <4 x i64*>* %ptr_to_vec
; CHECK-FUT: %ptrs = load <4 x p0>, p0 %ptr_to_vec
; CHECK:    LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK-NEXT:   <4 x i64*>
; CHECK-NEXT:  No element pointees.


!1 = !{i64 0, i32 0}  ; i64
!2 = !{!3, i32 1}  ; %struct.test01*
!3 = !{!"R", %struct.test01 zeroinitializer, i32 0}  ; %struct.test01
!4 = !{!"F", i1 false, i32 1, !5, !2}  ; void (%struct.test01*)
!5 = !{!"void", i32 0}  ; void
!6 = !{i32 0, i32 0}  ; i32
!7 = !{i64 0, i32 1}  ; i64*
!8 = !{!"R", %struct.test05inner zeroinitializer, i32 0}  ; %struct.test05inner
!9 = !{!"R", %struct.test05mid zeroinitializer, i32 0}  ; %struct.test05mid
!10 = !{!"F", i1 false, i32 1, !1, !11}  ; i64 (%struct.test05outer*)
!11 = !{!12, i32 1}  ; %struct.test05outer*
!12 = !{!"R", %struct.test05outer zeroinitializer, i32 0}  ; %struct.test05outer
!13 = !{!"A", i32 20, !1}  ; [20 x i64]
!14 = !{!"F", i1 false, i32 1, !1, !15}  ; i64 (%struct.test06*)
!15 = !{!16, i32 1}  ; %struct.test06*
!16 = !{!"R", %struct.test06 zeroinitializer, i32 0}  ; %struct.test06
!17 = !{!"A", i32 16, !18}  ; [16 x %struct.test07*]
!18 = !{!19, i32 1}  ; %struct.test07*
!19 = !{!"R", %struct.test07 zeroinitializer, i32 0}  ; %struct.test07
!20 = !{!"F", i1 false, i32 1, !5, !21}  ; void (<4 x i64*>*)
!21 = !{!22, i32 1}  ; <4 x i64*>*
!22 = !{!"V", i32 4, !7}  ; <4 x i64*>
!23 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !2} ; { i64, %struct.test01* }
!24 = !{!"S", %struct.test05inner zeroinitializer, i32 3, !1, !6, !7} ; { i64, i32, i64* }
!25 = !{!"S", %struct.test05mid zeroinitializer, i32 3, !1, !1, !8} ; { i64, i64, %struct.test05inner }
!26 = !{!"S", %struct.test05outer zeroinitializer, i32 2, !7, !9} ; { i64*, %struct.test05mid }
!27 = !{!"S", %struct.test06 zeroinitializer, i32 2, !6, !13} ; { i32, [20 x i64] }
!28 = !{!"S", %struct.test07 zeroinitializer, i32 2, !1, !1} ; { i64, i64 }
!29 = !{!"S", %struct.test08 zeroinitializer, i32 2, !1, !1} ; { i64, i64 }

!dtrans_types = !{!23, !24, !25, !26, !27, !28, !29}
