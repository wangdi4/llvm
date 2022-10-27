; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE

; Test pointer type recovery on getelementptr instructions involving 2 or
; more indices

; Lines marked with CHECK-NONOPAQUE are tests for the current form of IR.
; Lines marked with CHECK-OPAQUE are placeholders for check lines that will
;   changed when the future opaque pointer form of IR is used.
; Lines marked with CHECK should remain the same when changing to use opaque
;   pointers.

; Test accesses type detection of structure with simple GEP.
%struct.test01 = type { i64, %struct.test01* }
define internal void @test01(%struct.test01* "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !3 {
  %f0 = getelementptr %struct.test01, %struct.test01* %in, i64 0, i32 0
  %v0 = load i64, i64* %f0

  %f1 = getelementptr %struct.test01, %struct.test01* %in, i64 0, i32 1
  %v1 = load %struct.test01*, %struct.test01** %f1

  ret void
}

; CHECK-LABEL: void @test01
; CHECK-NONOPAQUE: %f0 = getelementptr %struct.test01, %struct.test01* %in, i64 0, i32 0
; CHECK-OPAQUE: %f0 = getelementptr %struct.test01, ptr %in, i64 0, i32 0
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i64*{{ *$}}
; CHECK-NEXT: Element pointees:
; CHECK-NEXT:   %struct.test01 @ 0

; CHECK-NONOPAQUE: %v0 = load i64, i64* %f0
; CHECK-OPAQUE: %v0 = load i64, ptr %f0
; CHECK-NONOPAQUE: %f1 = getelementptr %struct.test01, %struct.test01* %in, i64 0, i32 1
; CHECK-OPAQUE: %f1 = getelementptr %struct.test01, ptr %in, i64 0, i32 1
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   %struct.test01**{{ *$}}
; CHECK-NEXT: Element pointees:
; CHECK-NEXT:    %struct.test01 @ 1

; CHECK-NONOPAQUE: %v1 = load %struct.test01*, %struct.test01** %f1
; CHECK-OPAQUE: %v1 = load ptr, ptr %f1
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   %struct.test01*{{ *$}}
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
; CHECK-NONOPAQUE:   %f0 = getelementptr inbounds { i64, i32 }, { i64, i32 }* @test_var02, i64 0, i32 0
; CHECK-OPAQUE:   %f0 = getelementptr inbounds { i64, i32 }, ptr @test_var02, i64 0, i32 0
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i64*{{ *$}}
; CHECK-NEXT: Element pointees:
; CHECK-NEXT:  { i64, i32 } @ 0

; CHECK-NONOPAQUE:   %f1 = getelementptr inbounds { i64, i32 }, { i64, i32 }* @test_var02, i64 0, i32 1
; CHECK-OPAQUE:   %f1 = getelementptr inbounds { i64, i32 }, ptr @test_var02, i64 0, i32 1
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i32*{{ *$}}
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
; CHECK-NONOPAQUE:  %ar8 = getelementptr inbounds [20 x i64], [20 x i64]* @test_var3, i64 0, i32 8
; CHECK-OPAQUE:  %ar8 = getelementptr inbounds [20 x i64], ptr @test_var3, i64 0, i32 8
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i64*{{ *$}}
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
; CHECK-NONOPAQUE:  %ar1 = getelementptr inbounds [6 x [8 x [12 x i64]]], [6 x [8 x [12 x i64]]]* @test_var4, i64 0, i32 2, i32 3, i32 7
; CHECK-OPAQUE:  %ar1 = getelementptr inbounds [6 x [8 x [12 x i64]]], ptr @test_var4, i64 0, i32 2, i32 3, i32 7
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i64*{{ *$}}
; CHECK-NEXT: Element pointees:
; CHECK-NEXT:   [12 x i64] @ 7

; CHECK-NONOPAQUE:  %ar2 = getelementptr inbounds [6 x [8 x [12 x i64]]], [6 x [8 x [12 x i64]]]* @test_var4, i64 0, i32 %x, i32 %y, i32 7
; CHECK-OPAQUE:  %ar2 = getelementptr inbounds [6 x [8 x [12 x i64]]], ptr @test_var4, i64 0, i32 %x, i32 %y, i32 7
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i64*{{ *$}}
; CHECK-NEXT: Element pointees:
; CHECK-NEXT:   [12 x i64] @ 7


; Test access with nested structure element
%struct.test05inner = type { i64, i32, i64* }
%struct.test05mid = type { i64, i64, %struct.test05inner }
%struct.test05outer = type { i64*, %struct.test05mid }
define internal i64 @test05(%struct.test05outer* "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !9 {
  %addr0 = getelementptr inbounds %struct.test05outer, %struct.test05outer* %in, i64 0, i32 1, i32 2, i32 0
  %val0 = load i64, i64* %addr0
  %addr2 = getelementptr inbounds %struct.test05outer, %struct.test05outer* %in, i64 0, i32 1, i32 2, i32 2
  %ptr = load i64*, i64** %addr2
  %val2 = load i64, i64* %ptr
  ret i64 %val0
}
; CHECK-LABEL: i64 @test05
; CHECK-NONOPAQUE: %addr0 = getelementptr inbounds %struct.test05outer, %struct.test05outer* %in, i64 0, i32 1, i32 2, i32 0
; CHECK-OPAQUE: %addr0 = getelementptr inbounds %struct.test05outer, ptr %in, i64 0, i32 1, i32 2, i32 0
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i64*{{ *$}}
; CHECK-NEXT: Element pointees:
; CHECK-NEXT:   %struct.test05inner @ 0

; CHECK-NONOPAQUE: %addr2 = getelementptr inbounds %struct.test05outer, %struct.test05outer* %in, i64 0, i32 1, i32 2, i32 2
; CHECK-OPAQUE: %addr2 = getelementptr inbounds %struct.test05outer, ptr %in, i64 0, i32 1, i32 2, i32 2
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i64**{{ *$}}
; CHECK-NEXT: Element pointees:
; CHECK-NEXT:   %struct.test05inner @ 2

; CHECK-NONOPAQUE: %ptr = load i64*, i64** %addr2
; CHECK-OPAQUE: %ptr = load ptr, ptr %addr2
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i64*{{ *$}}
; CHECK-NEXT:  No element pointees.


; Test access to an element of an array member of a structure.
; TODO: The legacy LocalPointerAnalyzer does not include struct.test06 @ 1 as
; an element pointee, and therefore does not track that field as being used.
; The PointerTypeAnalyzer will behave the same for tracking.
; Because none of the transformations are operating on array fields, this is
; safe.
%struct.test06 = type { i32, [20 x i64] }
define internal i64 @test06(%struct.test06* "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !12 {
  %elem_addr = getelementptr inbounds %struct.test06, %struct.test06* %in, i64 0, i32 1, i64 4
  %val = load i64, i64* %elem_addr
  ret i64 %val
}
; CHECK-LABEL: i64 @test06
; CHECK-NONOPAQUE: %elem_addr = getelementptr inbounds %struct.test06, %struct.test06* %in, i64 0, i32 1, i64 4
; CHECK-OPAQUE: %elem_addr = getelementptr inbounds %struct.test06, ptr %in, i64 0, i32 1, i64 4
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i64*{{ *$}}
; CHECK-NEXT: Element pointees:
; CHECK-NEXT:   [20 x i64] @ 4


; Test access to an element that is an array of pointers
%struct.test07 = type { i64, i64 }
@var07 = internal global [16 x %struct.test07*] zeroinitializer, !intel_dtrans_type !13
define internal void @test07() {
  %addr = getelementptr [16 x %struct.test07*], [16 x %struct.test07*]* @var07, i64 0, i32 2
  %sptr = load %struct.test07*, %struct.test07** %addr
  ret void
}
; CHECK-LABEL: void @test07
; CHECK-NONOPAQUE: %addr = getelementptr [16 x %struct.test07*], [16 x %struct.test07*]* @var07, i64 0, i32 2
; CHECK-OPAQUE: %addr = getelementptr [16 x ptr], ptr @var07, i64 0, i32 2
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   %struct.test07**{{ *$}}
; CHECK-NEXT: Element pointees:
; CHECK-NEXT:   [16 x %struct.test07*] @ 2

; CHECK-NONOPAQUE: %sptr = load %struct.test07*, %struct.test07** %addr
; CHECK-OPAQUE: %sptr = load ptr, ptr %addr
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   %struct.test07*{{ *$}}
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
; CHECK-NONOPAQUE: %addr = getelementptr [16 x %struct.test08], [16 x %struct.test08]* @var08, i64 0, i32 5, i32 1
; CHECK-OPAQUE: %addr = getelementptr [16 x %struct.test08], ptr @var08, i64 0, i32 5, i32 1
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   i64*{{ *$}}
; CHECK-NEXT: Element pointees:
; CHECK-NEXT:   %struct.test08 @ 1


; Test access to vector element to produce a vector of pointers. In this
; case the result of the GEP is a vector of pointers of the same type
define internal void @test09(<4 x i64*>* "intel_dtrans_func_index"="1" %ptr_to_vec) !intel.dtrans.func.type !17 {
  %ptrs = load <4 x i64*>, <4 x i64*>* %ptr_to_vec
  %addr = getelementptr i64, <4 x i64*> %ptrs, <4 x i64> <i64 1, i64 2, i64 2, i64 1>
  %a2 = extractelement <4 x i64*> %addr, i32 2
  %v = load i64, i64* %a2
  ret void
}
; CHECK-LABEL: void @test09
; CHECK-NONOPAQUE: %ptrs = load <4 x i64*>, <4 x i64*>* %ptr_to_vec
; CHECK-OPAQUE: %ptrs = load <4 x ptr>, ptr %ptr_to_vec
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   <4 x i64*>{{ *$}}
; CHECK-NEXT:  No element pointees.


!1 = !{i64 0, i32 0}  ; i64
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = distinct !{!2}
!4 = !{i32 0, i32 0}  ; i32
!5 = !{i64 0, i32 1}  ; i64*
!6 = !{%struct.test05inner zeroinitializer, i32 0}  ; %struct.test05inner
!7 = !{%struct.test05mid zeroinitializer, i32 0}  ; %struct.test05mid
!8 = !{%struct.test05outer zeroinitializer, i32 1}  ; %struct.test05outer*
!9 = distinct !{!8}
!10 = !{!"A", i32 20, !1}  ; [20 x i64]
!11 = !{%struct.test06 zeroinitializer, i32 1}  ; %struct.test06*
!12 = distinct !{!11}
!13 = !{!"A", i32 16, !14}  ; [16 x %struct.test07*]
!14 = !{%struct.test07 zeroinitializer, i32 1}  ; %struct.test07*
!15 = !{!16, i32 1}  ; <4 x i64*>*
!16 = !{!"V", i32 4, !5}  ; <4 x i64*>
!17 = distinct !{!15}
!18 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !2} ; { i64, %struct.test01* }
!19 = !{!"S", %struct.test05inner zeroinitializer, i32 3, !1, !4, !5} ; { i64, i32, i64* }
!20 = !{!"S", %struct.test05mid zeroinitializer, i32 3, !1, !1, !6} ; { i64, i64, %struct.test05inner }
!21 = !{!"S", %struct.test05outer zeroinitializer, i32 2, !5, !7} ; { i64*, %struct.test05mid }
!22 = !{!"S", %struct.test06 zeroinitializer, i32 2, !4, !10} ; { i32, [20 x i64] }
!23 = !{!"S", %struct.test07 zeroinitializer, i32 2, !1, !1} ; { i64, i64 }
!24 = !{!"S", %struct.test08 zeroinitializer, i32 2, !1, !1} ; { i64, i64 }

!intel.dtrans.types = !{!18, !19, !20, !21, !22, !23, !24}
