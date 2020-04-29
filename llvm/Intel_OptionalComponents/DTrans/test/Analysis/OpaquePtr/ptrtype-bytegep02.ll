; REQUIRES: asserts
; RUN: opt -disable-output -whole-program-assume -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-CUR
; RUN: opt -disable-output -whole-program-assume -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-CUR

; Test various cases of byte-flattened GEP accesses. In particular, verify
; cases where first element of an aggregate matches the element-zero handling
; rules for DTrans. This test is based on the cases within
; DTrans/test/Analysis/byte-flattened-gep.ll

; Lines marked with CHECK-CUR are tests for the current form of IR.
; Lines marked with CHECK-FUT are placeholders for check lines that will
;   changed when the future opaque pointer form of IR is used.
; Lines marked with CHECK should remain the same when changing to use opaque
;   pointers.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"


; Byte-flattend GEP without special handling of first element of the structure.
%struct.test01 = type { i64, i32 }
define void @test01(%struct.test01* %p) !dtrans_type !3 {
  %p2 = bitcast %struct.test01* %p to i8*
  %py8 = getelementptr i8, i8* %p2, i64 8
  %py = bitcast i8* %py8 to i32*
  %y = load i32, i32* %py
  ret void
}
; CHECK-LABEL: void @test01(
; CHECK-CUR: %py8 = getelementptr i8, i8* %p2, i64 8
; CHECK-FUT: %py8 = getelementptr i8, p0 %p2, i64 8
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT:   Aliased types:
; CHECK-NEXT:     i32*{{ *$}}
; CHECK-NEXT:     i8*{{ *$}}
; CHECK-NEXT:   Element pointees:
; CHECK-NEXT:     %struct.test01 @ 1

; Byte-flattened GEP when first element of the structure is ptr-to-ptr.
%struct.test02 = type { i64**, i32 }
define void @test02(%struct.test02* %p) !dtrans_type !8 {
  %p2 = bitcast %struct.test02* %p to i8*
  %py8 = getelementptr i8, i8* %p2, i64 8
  %py = bitcast i8* %py8 to i32*
  %y = load i32, i32* %py
  ret void
}
; CHECK-LABEL: void @test02(
; CHECK-CUR: %py8 = getelementptr i8, i8* %p2, i64 8
; CHECK-FUT: %py8 = getelementptr i8, p0 %p2, i64 8
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT:   Aliased types:
; CHECK-NEXT:     i32*{{ *$}}
; CHECK-NEXT:     i8*{{ *$}}
; CHECK-NEXT:   Element pointees:
; CHECK-NEXT:     %struct.test02 @ 1


; Byte-flattened GEP when first element of the structure is i8*.
%struct.test03 = type { i8*, i32 }
define void @test03(%struct.test03* %p) !dtrans_type !12 {
  %p2 = bitcast %struct.test03* %p to i8*
  %py8 = getelementptr i8, i8* %p2, i64 8
  %py = bitcast i8* %py8 to i32*
  %y = load i32, i32* %py
  ret void
}
; CHECK-LABEL: void @test03(
; CHECK-CUR: %py8 = getelementptr i8, i8* %p2, i64 8
; CHECK-FUT: %py8 = getelementptr i8, p0 %p2, i64 8
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT:   Aliased types:
; CHECK-NEXT:     i32*{{ *$}}
; CHECK-NEXT:     i8*{{ *$}}
; CHECK-NEXT:  Element pointees:
; CHECK-NEXT:     %struct.test03 @ 1


; Byte-flattened GEP when first element of the structure is an array of i8.
%struct.test04 = type { [8 x i8], i32 }
define void @test04(%struct.test04* %p) !dtrans_type !17 {
  %p2 = bitcast %struct.test04* %p to i8*
  %py8 = getelementptr i8, i8* %p2, i64 8
  %py = bitcast i8* %py8 to i32*
  %y = load i32, i32* %py
  ret void
}
; CHECK-LABEL: void @test04(
; CHECK-CUR: %py8 = getelementptr i8, i8* %p2, i64 8
; CHECK-FUT: %py8 = getelementptr i8, p0 %p2, i64 8
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT:   Aliased types:
; CHECK-NEXT:     i32*{{ *$}}
; CHECK-NEXT:     i8*{{ *$}}
; CHECK-NEXT:  Element pointees:
; CHECK-NEXT:    %struct.test04 @ 1


; Byte-flattened GEP when first element of a nested structure is i8*,
; and GEP accesses element of the inner structure.
%struct.test05inner = type { i8*, i32, i32 }
%struct.test05outer = type { %struct.test05inner, i32 }
define void @test05(%struct.test05outer* %p) !dtrans_type !21 {
  %p2 = bitcast %struct.test05outer* %p to i8*
  %py8 = getelementptr i8, i8* %p2, i64 8
  %py = bitcast i8* %py8 to i32*
  %y = load i32, i32* %py
  ret void
}
; CHECK-LABEL: void @test05(
; CHECK-CUR: %py8 = getelementptr i8, i8* %p2, i64 8
; CHECK-FUT: %py8 = getelementptr i8, p0 %p2, i64 8
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT:   Aliased types:
; CHECK-NEXT:     i32*{{ *$}}
; CHECK-NEXT:     i8*{{ *$}}
; CHECK-NEXT:   Element pointees:
; CHECK-NEXT:     %struct.test05inner @ 1

; Byte-flattened GEP when first element of a nested structure is i8*,
; and GEP accesses element of the outer structure.
%struct.test06inner = type { i8*, i32, i32 }
%struct.test06outer = type { %struct.test05inner, i32 }
define void @test06(%struct.test06outer* %p) !dtrans_type !24 {
  %p2 = bitcast %struct.test06outer* %p to i8*
  %py8 = getelementptr i8, i8* %p2, i64 16
  %py = bitcast i8* %py8 to i32*
  %y = load i32, i32* %py
  ret void
}
; CHECK-LABEL: void @test06(
; CHECK-CUR: %py8 = getelementptr i8, i8* %p2, i64 16
; CHECK-FUT: %py8 = getelementptr i8, p0 %p2, i64 16
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT:   Aliased types:
; CHECK-NEXT:     i32*{{ *$}}
; CHECK-NEXT:     i8*{{ *$}}
; CHECK-NEXT:   Element pointees:
; CHECK-NEXT:     %struct.test06outer @ 1


; Byte-flattened GEP when first element of the structure is
; a multi-dimension array of i8.
%struct.test07 = type { [2 x [4 x i8]], i32 }
define void @test07(%struct.test07* %p) !dtrans_type !29 {
  %p2 = bitcast %struct.test07* %p to i8*
  %py8 = getelementptr i8, i8* %p2, i64 8
  %py = bitcast i8* %py8 to i32*
  %y = load i32, i32* %py
  ret void
}
; CHECK-LABEL: void @test07(
; CHECK-CUR: %py8 = getelementptr i8, i8* %p2, i64 8
; CHECK-FUT: %py8 = getelementptr i8, p0 %p2, i64 8
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT:   Aliased types:
; CHECK-NEXT:     i32*{{ *$}}
; CHECK-NEXT:     i8*{{ *$}}
; CHECK-NEXT:  Element pointees:
; CHECK-NEXT:    %struct.test07 @ 1

!1 = !{i64 0, i32 0}  ; i64
!2 = !{i32 0, i32 0}  ; i32
!3 = !{!"F", i1 false, i32 1, !4, !5}  ; void (%struct.test01*)
!4 = !{!"void", i32 0}  ; void
!5 = !{!6, i32 1}  ; %struct.test01*
!6 = !{!"R", %struct.test01 zeroinitializer, i32 0}  ; %struct.test01
!7 = !{i64 0, i32 2}  ; i64**
!8 = !{!"F", i1 false, i32 1, !4, !9}  ; void (%struct.test02*)
!9 = !{!10, i32 1}  ; %struct.test02*
!10 = !{!"R", %struct.test02 zeroinitializer, i32 0}  ; %struct.test02
!11 = !{i8 0, i32 1}  ; i8*
!12 = !{!"F", i1 false, i32 1, !4, !13}  ; void (%struct.test03*)
!13 = !{!14, i32 1}  ; %struct.test03*
!14 = !{!"R", %struct.test03 zeroinitializer, i32 0}  ; %struct.test03
!15 = !{!"A", i32 8, !16}  ; [8 x i8]
!16 = !{i8 0, i32 0}  ; i8
!17 = !{!"F", i1 false, i32 1, !4, !18}  ; void (%struct.test04*)
!18 = !{!19, i32 1}  ; %struct.test04*
!19 = !{!"R", %struct.test04 zeroinitializer, i32 0}  ; %struct.test04
!20 = !{!"R", %struct.test05inner zeroinitializer, i32 0}  ; %struct.test05inner
!21 = !{!"F", i1 false, i32 1, !4, !22}  ; void (%struct.test05outer*)
!22 = !{!23, i32 1}  ; %struct.test05outer*
!23 = !{!"R", %struct.test05outer zeroinitializer, i32 0}  ; %struct.test05outer
!24 = !{!"F", i1 false, i32 1, !4, !25}  ; void (%struct.test06outer*)
!25 = !{!26, i32 1}  ; %struct.test06outer*
!26 = !{!"R", %struct.test06outer zeroinitializer, i32 0}  ; %struct.test06outer
!27 = !{!"A", i32 2, !28}  ; [2 x [4 x i8]]
!28 = !{!"A", i32 4, !16}  ; [4 x i8]
!29 = !{!"F", i1 false, i32 1, !4, !30}  ; void (%struct.test07*)
!30 = !{!31, i32 1}  ; %struct.test07*
!31 = !{!"R", %struct.test07 zeroinitializer, i32 0}  ; %struct.test07
!32 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !2} ; { i64, i32 }
!33 = !{!"S", %struct.test02 zeroinitializer, i32 2, !7, !2} ; { i64**, i32 }
!34 = !{!"S", %struct.test03 zeroinitializer, i32 2, !11, !2} ; { i8*, i32 }
!35 = !{!"S", %struct.test04 zeroinitializer, i32 2, !15, !2} ; { [8 x i8], i32 }
!36 = !{!"S", %struct.test05inner zeroinitializer, i32 3, !11, !2, !2} ; { i8*, i32, i32 }
!37 = !{!"S", %struct.test05outer zeroinitializer, i32 2, !20, !2} ; { %struct.test05inner, i32 }
!38 = !{!"S", %struct.test06inner zeroinitializer, i32 3, !11, !2, !2} ; { i8*, i32, i32 }
!39 = !{!"S", %struct.test06outer zeroinitializer, i32 2, !20, !2} ; { %struct.test05inner, i32 }
!40 = !{!"S", %struct.test07 zeroinitializer, i32 2, !27, !2} ; { [2 x [4 x i8]], i32 }

!dtrans_types = !{!32, !33, !34, !35, !36, !37, !38, !39, !40}
