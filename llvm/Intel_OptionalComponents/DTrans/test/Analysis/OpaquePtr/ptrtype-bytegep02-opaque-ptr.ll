; REQUIRES: asserts
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s

; Test various cases of byte-flattened GEP accesses. In particular, verify
; cases where first element of an aggregate matches the element-zero handling
; rules for DTrans. This test is based on the cases within
; DTrans/test/Analysis/byte-flattened-gep.ll

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"


; Byte-flattend GEP without special handling of first element of the structure.
%struct.test01 = type { i64, i32 }
define void @test01(ptr "intel_dtrans_func_index"="1" %p) !intel.dtrans.func.type !4 {
  %py8 = getelementptr i8, ptr %p, i64 8
  %y = load i32, ptr %py8
  ret void
}
; CHECK-LABEL: void @test01(
; CHECK: %py8 = getelementptr i8, ptr %p, i64 8
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT:   Aliased types:
; CHECK-NEXT:     i32*{{ *$}}
; CHECK-NEXT:   Element pointees:
; CHECK-NEXT:     %struct.test01 @ 1

; Byte-flattened GEP when first element of the structure is ptr-to-ptr.
%struct.test02 = type { ptr, i32 }
define void @test02(ptr "intel_dtrans_func_index"="1" %p) !intel.dtrans.func.type !7 {
  %py8 = getelementptr i8, ptr %p, i64 8
  %y = load i32, ptr %py8
  ret void
}
; CHECK-LABEL: void @test02(
; CHECK: %py8 = getelementptr i8, ptr %p, i64 8
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT:   Aliased types:
; CHECK-NEXT:     i32*{{ *$}}
; CHECK-NEXT:   Element pointees:
; CHECK-NEXT:     %struct.test02 @ 1


; Byte-flattened GEP when first element of the structure is i8*, based on
; the metadata info.
%struct.test03 = type { ptr, i32 }
define void @test03(ptr "intel_dtrans_func_index"="1" %p) !intel.dtrans.func.type !10 {
  %py8 = getelementptr i8, ptr %p, i64 8
  %y = load i32, ptr %py8
  ret void
}
; CHECK-LABEL: void @test03(
; CHECK: %py8 = getelementptr i8, ptr %p, i64 8
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT:   Aliased types:
; CHECK-NEXT:     i32*{{ *$}}
; CHECK-NEXT:  Element pointees:
; CHECK-NEXT:     %struct.test03 @ 1


; Byte-flattened GEP when first element of the structure is an array of i8.
%struct.test04 = type { [8 x i8], i32 }
define void @test04(ptr "intel_dtrans_func_index"="1" %p) !intel.dtrans.func.type !14 {
  %py8 = getelementptr i8, ptr %p, i64 8
  %y = load i32, ptr %py8
  ret void
}
; CHECK-LABEL: void @test04(
; CHECK: %py8 = getelementptr i8, ptr %p, i64 8
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT:   Aliased types:
; CHECK-NEXT:     i32*{{ *$}}
; CHECK-NEXT:  Element pointees:
; CHECK-NEXT:    %struct.test04 @ 1


; Byte-flattened GEP when first element of a nested structure is i8*,
; and GEP accesses element of the inner structure.
%struct.test05inner = type { ptr, i32, i32 }
%struct.test05outer = type { %struct.test05inner, i32 }
define void @test05(ptr "intel_dtrans_func_index"="1" %p) !intel.dtrans.func.type !17 {
  %py8 = getelementptr i8, ptr %p, i64 8
  %y = load i32, ptr %py8
  ret void
}
; CHECK-LABEL: void @test05(
; CHECK: %py8 = getelementptr i8, ptr %p, i64 8
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT:   Aliased types:
; CHECK-NEXT:     i32*{{ *$}}
; CHECK-NEXT:   Element pointees:
; CHECK-NEXT:     %struct.test05inner @ 1

; Byte-flattened GEP when first element of a nested structure is i8*,
; and GEP accesses element of the outer structure.
%struct.test06inner = type { ptr, i32, i32 }
%struct.test06outer = type { %struct.test05inner, i32 }
define void @test06(ptr "intel_dtrans_func_index"="1" %p) !intel.dtrans.func.type !19 {
  %py8 = getelementptr i8, ptr %p, i64 16
  %y = load i32, ptr %py8
  ret void
}
; CHECK-LABEL: void @test06(
; CHECK: %py8 = getelementptr i8, ptr %p, i64 16
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT:   Aliased types:
; CHECK-NEXT:     i32*{{ *$}}
; CHECK-NEXT:   Element pointees:
; CHECK-NEXT:     %struct.test06outer @ 1


; Byte-flattened GEP when first element of the structure is
; a multi-dimension array of i8.
%struct.test07 = type { [2 x [4 x i8]], i32 }
define void @test07(ptr "intel_dtrans_func_index"="1" %p) !intel.dtrans.func.type !23 {
  %py8 = getelementptr i8, ptr %p, i64 8
  %y = load i32, ptr %py8
  ret void
}
; CHECK-LABEL: void @test07(
; CHECK: %py8 = getelementptr i8, ptr %p, i64 8
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT:   Aliased types:
; CHECK-NEXT:     i32*{{ *$}}
; CHECK-NEXT:  Element pointees:
; CHECK-NEXT:    %struct.test07 @ 1

!1 = !{i64 0, i32 0}  ; i64
!2 = !{i32 0, i32 0}  ; i32
!3 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!4 = distinct !{!3}
!5 = !{i64 0, i32 2}  ; i64**
!6 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!7 = distinct !{!6}
!8 = !{i8 0, i32 1}  ; i8*
!9 = !{%struct.test03 zeroinitializer, i32 1}  ; %struct.test03*
!10 = distinct !{!9}
!11 = !{!"A", i32 8, !12}  ; [8 x i8]
!12 = !{i8 0, i32 0}  ; i8
!13 = !{%struct.test04 zeroinitializer, i32 1}  ; %struct.test04*
!14 = distinct !{!13}
!15 = !{%struct.test05inner zeroinitializer, i32 0}  ; %struct.test05inner
!16 = !{%struct.test05outer zeroinitializer, i32 1}  ; %struct.test05outer*
!17 = distinct !{!16}
!18 = !{%struct.test06outer zeroinitializer, i32 1}  ; %struct.test06outer*
!19 = distinct !{!18}
!20 = !{!"A", i32 2, !21}  ; [2 x [4 x i8]]
!21 = !{!"A", i32 4, !12}  ; [4 x i8]
!22 = !{%struct.test07 zeroinitializer, i32 1}  ; %struct.test07*
!23 = distinct !{!22}
!24 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !2} ; { i64, i32 }
!25 = !{!"S", %struct.test02 zeroinitializer, i32 2, !5, !2} ; { i64**, i32 }
!26 = !{!"S", %struct.test03 zeroinitializer, i32 2, !8, !2} ; { i8*, i32 }
!27 = !{!"S", %struct.test04 zeroinitializer, i32 2, !11, !2} ; { [8 x i8], i32 }
!28 = !{!"S", %struct.test05inner zeroinitializer, i32 3, !8, !2, !2} ; { i8*, i32, i32 }
!29 = !{!"S", %struct.test05outer zeroinitializer, i32 2, !15, !2} ; { %struct.test05inner, i32 }
!30 = !{!"S", %struct.test06inner zeroinitializer, i32 3, !8, !2, !2} ; { i8*, i32, i32 }
!31 = !{!"S", %struct.test06outer zeroinitializer, i32 2, !15, !2} ; { %struct.test05inner, i32 }
!32 = !{!"S", %struct.test07 zeroinitializer, i32 2, !20, !2} ; { [2 x [4 x i8]], i32 }

!intel.dtrans.types = !{!24, !25, !26, !27, !28, !29, !30, !31, !32}
