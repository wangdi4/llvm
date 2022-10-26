; RUN: opt -opaque-pointers -S -whole-program-assume -intel-libirc-allowed -passes=dtrans-normalizeop < %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Check that DTrans normalize pass inserts a GEP for the store
; that corresponds to an element zero access when working with
; nested structures.

; Single level of nesting
%struct.test00outer = type { %struct.test00inner }
%struct.test00inner = type { ptr }
%struct.test00inner_impl = type { i32, i32, i32 }
define internal void @test00(ptr "intel_dtrans_func_index"="1" %p) !intel.dtrans.func.type !5 {
  %elem_zero_addr1 = getelementptr %struct.test00outer, ptr %p, i64 0, i32 0, i32 0

  ; Loading and storing the pointer stored at the first location of the nested structure.
  %val1 = load ptr, ptr %elem_zero_addr1
  store ptr %val1, ptr %p
  ret void
}
; CHECK-LABEL: define internal void @test00
; CHECK: %dtnorm = getelementptr %struct.test00outer, ptr %p, i64 0, i32 0, i32 0
; CHECK: store ptr %val1, ptr  %dtnorm


; Test with multiple nested structure elements. Check that the normalized IR
; creates GEPs for the element zero accesses.
%struct.test01outer = type { %struct.test01middle }
%struct.test01middle = type { %struct.test01inner }
%struct.test01inner = type { ptr }
%struct.test01inner_impl = type { i32, i32, i32 }
define internal void @test01(ptr "intel_dtrans_func_index"="1" %p) !intel.dtrans.func.type !10 {
  ; Loading and storing the pointer stored at the first location of the nested structure.
  %elem_zero_addr1 = getelementptr %struct.test01outer, ptr %p, i64 0, i32 0, i32 0, i32 0
  %val1 = load ptr, ptr %elem_zero_addr1
  store ptr %val1, ptr %p
  ret void
}
; CHECK-LABEL: define internal void @test01
; CHECK: %dtnorm = getelementptr %struct.test01outer, ptr %p, i64 0, i32 0, i32 0, i32 0
; CHECK: store ptr %val1, ptr %dtnorm

; Test with a pointer that is a non-aggregate type.
%struct.test02outer = type { %struct.test02middle }
%struct.test02middle = type { %struct.test02inner }
%struct.test02inner = type { %struct.test02inner_impl }
%struct.test02inner_impl = type { ptr, ptr }
define internal void @test02(ptr "intel_dtrans_func_index"="1" %p) !intel.dtrans.func.type !16 {
  %elem_zero_addr1 = getelementptr %struct.test02outer, ptr %p, i64 0, i32 0, i32 0, i32 0, i32 0
  %val1 = load ptr, ptr %elem_zero_addr1
  store ptr %val1, ptr %p
  ret void
}
; CHECK-LABEL: define internal void @test02
; CHECK: %dtnorm = getelementptr %struct.test02outer, ptr %p, i64 0, i32 0, i32 0, i32 0, i32 0
; CHECK: store ptr %val1, ptr %dtnorm


; Test with a scalar type, after several levels of nested types
%struct.test03outer = type { %struct.test03middle }
%struct.test03middle = type { %struct.test03inner }
%struct.test03inner = type { %struct.test03inner_impl }
%struct.test03inner_impl = type { i64, i64 }
define internal void @test03(ptr "intel_dtrans_func_index"="1" %p) !intel.dtrans.func.type !22 {
  %elem_zero_addr1 = getelementptr %struct.test03outer, ptr %p, i64 0, i32 0, i32 0, i32 0, i32 0
  %val1 = load i64, ptr %elem_zero_addr1
  store i64 %val1, ptr %p

  ret void
}
; CHECK-LABEL: define internal void @test03
; CHECK: %dtnorm = getelementptr %struct.test03outer, ptr %p, i64 0, i32 0, i32 0, i32 0, i32 0
; CHECK: store i64 %val1, ptr %dtnorm

!1 = !{%struct.test00inner zeroinitializer, i32 0}  ; %struct.test00inner
!2 = !{%struct.test00inner_impl zeroinitializer, i32 1}  ; %struct.test00inner_impl*
!3 = !{i32 0, i32 0}  ; i32
!4 = !{%struct.test00outer zeroinitializer, i32 1}  ; %struct.test00outer*
!5 = distinct !{!4}
!6 = !{%struct.test01middle zeroinitializer, i32 0}  ; %struct.test01middle
!7 = !{%struct.test01inner zeroinitializer, i32 0}  ; %struct.test01inner
!8 = !{%struct.test01inner_impl zeroinitializer, i32 1}  ; %struct.test01inner_impl*
!9 = !{%struct.test01outer zeroinitializer, i32 1}  ; %struct.test01outer*
!10 = distinct !{!9}
!11 = !{%struct.test02middle zeroinitializer, i32 0}  ; %struct.test02middle
!12 = !{%struct.test02inner zeroinitializer, i32 0}  ; %struct.test02inner
!13 = !{%struct.test02inner_impl zeroinitializer, i32 0}  ; %struct.test02inner_impl
!14 = !{i64 0, i32 1}  ; i64*
!15 = !{%struct.test02outer zeroinitializer, i32 1}  ; %struct.test02outer*
!16 = distinct !{!15}
!17 = !{%struct.test03middle zeroinitializer, i32 0}  ; %struct.test03middle
!18 = !{%struct.test03inner zeroinitializer, i32 0}  ; %struct.test03inner
!19 = !{%struct.test03inner_impl zeroinitializer, i32 0}  ; %struct.test03inner_impl
!20 = !{i64 0, i32 0}  ; i64
!21 = !{%struct.test03outer zeroinitializer, i32 1}  ; %struct.test03outer*
!22 = distinct !{!21}
!23 = !{!"S", %struct.test00outer zeroinitializer, i32 1, !1} ; { %struct.test00inner }
!24 = !{!"S", %struct.test00inner zeroinitializer, i32 1, !2} ; { %struct.test00inner_impl* }
!25 = !{!"S", %struct.test00inner_impl zeroinitializer, i32 3, !3, !3, !3} ; { i32, i32, i32 }
!26 = !{!"S", %struct.test01outer zeroinitializer, i32 1, !6} ; { %struct.test01middle }
!27 = !{!"S", %struct.test01middle zeroinitializer, i32 1, !7} ; { %struct.test01inner }
!28 = !{!"S", %struct.test01inner zeroinitializer, i32 1, !8} ; { %struct.test01inner_impl* }
!29 = !{!"S", %struct.test01inner_impl zeroinitializer, i32 3, !3, !3, !3} ; { i32, i32, i32 }
!30 = !{!"S", %struct.test02outer zeroinitializer, i32 1, !11} ; { %struct.test02middle }
!31 = !{!"S", %struct.test02middle zeroinitializer, i32 1, !12} ; { %struct.test02inner }
!32 = !{!"S", %struct.test02inner zeroinitializer, i32 1, !13} ; { %struct.test02inner_impl }
!33 = !{!"S", %struct.test02inner_impl zeroinitializer, i32 2, !14, !14} ; { i64*, i64* }
!34 = !{!"S", %struct.test03outer zeroinitializer, i32 1, !17} ; { %struct.test03middle }
!35 = !{!"S", %struct.test03middle zeroinitializer, i32 1, !18} ; { %struct.test03inner }
!36 = !{!"S", %struct.test03inner zeroinitializer, i32 1, !19} ; { %struct.test03inner_impl }
!37 = !{!"S", %struct.test03inner_impl zeroinitializer, i32 2, !20, !20} ; { i64, i64 }

!intel.dtrans.types = !{!23, !24, !25, !26, !27, !28, !29, !30, !31, !32, !33, !34, !35, !36, !37}
