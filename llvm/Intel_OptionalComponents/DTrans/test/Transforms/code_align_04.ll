; UNSUPPORTED: enable-opaque-pointers

; This test verifies that codealign doesn't crash and no change of alignment
; of @bar with opaque pointers.
; This test is same as code_align_01.ll.

; RUN: opt < %s -opaque-pointers -dtrans-codealign -whole-program-assume -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -S 2>&1 | FileCheck %s
; RUN: opt < %s -opaque-pointers -passes=dtrans-codealign -whole-program-assume -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -S 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: define dso_local zeroext i1 @bar
; CHECK-SAME: align 2

%"class.xalanc_1_10::XalanDOMStringCache" = type { %"class.xalanc_1_10::XalanVector", %"class.xalanc_1_10::XalanVector", i32, [4 x i8], %"class.xalanc_1_10::XalanDOMStringReusableAllocator" }
%"class.xalanc_1_10::XalanVector" = type { %"class.xercesc_2_7::MemoryManager"*, i64, i64, %"class.xalanc_1_10::XalanDOMString"** }
%"class.xalanc_1_10::XalanDOMString" = type <{ %"class.xalanc_1_10::XalanVector.0", i32, [4 x i8] }>
%"class.xalanc_1_10::XalanVector.0" = type { %"class.xercesc_2_7::MemoryManager"*, i64, i64, i16* }
%"class.xalanc_1_10::XalanDOMStringReusableAllocator" = type { %"class.xalanc_1_10::ReusableArenaAllocator" }
%"class.xalanc_1_10::ReusableArenaAllocator" = type <{ %"class.xalanc_1_10::ArenaAllocator", i8, [7 x i8] }>
%"class.xalanc_1_10::ArenaAllocator" = type { i32 (...)**, i16, %"class.xalanc_1_10::XalanList" }
%"class.xalanc_1_10::XalanList" = type { %"class.xercesc_2_7::MemoryManager"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XalanDOMString> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XalanDOMString> *>::Node"* }
%"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XalanDOMString> *>::Node" = type { %"class.xalanc_1_10::ReusableArenaBlock"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XalanDOMString> *>::Node"*, %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XalanDOMString> *>::Node"* }
%"class.xalanc_1_10::ReusableArenaBlock" = type <{ %"class.xalanc_1_10::ArenaBlockBase", i16, i16, [4 x i8] }>
%"class.xalanc_1_10::ArenaBlockBase" = type { %"class.xalanc_1_10::XalanAllocator", i16, i16, %"class.xalanc_1_10::XalanDOMString"* }
%"class.xalanc_1_10::XalanAllocator" = type { %"class.xercesc_2_7::MemoryManager"* }
%"class.xercesc_2_7::MemoryManager" = type { i32 (...)** }


%__SOADT_class.F = type { %__SOADT_AR_struct.Arr*, i64 }
%__SOADT_AR_struct.Arr = type { i32, [4 x i8], %__SOADT_EL_class.F*, i32, [4 x i8] }
%__SOADT_EL_class.F = type { i32*, float* }

define dso_local void @foo() !dtrans-soatoaos !35 {
  call i1 @bar(%"class.xalanc_1_10::XalanDOMStringCache"* null, %"class.xalanc_1_10::XalanDOMString"* null)
  call i1 @bar(%"class.xalanc_1_10::XalanDOMStringCache"* null, %"class.xalanc_1_10::XalanDOMString"* null)
  ret void
}

; Function Attrs: uwtable
define dso_local zeroext i1 @bar(%"class.xalanc_1_10::XalanDOMStringCache"* "intel_dtrans_func_index"="1" %this, %"class.xalanc_1_10::XalanDOMString"* "intel_dtrans_func_index"="2" %theString) align 2 !intel.dtrans.func.type !36 {
bb_1:
  br label %bb_2

bb_2:
  br label %bb_3

bb_3:
  br label %bb_4

bb_4:
  br label %bb_5

bb_5:
  br label %bb_6

bb_6:
  br label %bb_7

bb_7:
  br label %bb_8

bb_8:
  br label %bb_9

bb_9:
  br label %bb_a

bb_a:
  br label %bb_b

bb_b:
  br label %bb_c

bb_c:
  br label %bb_d

bb_d:
  br label %bb_e

bb_e:
  br label %bb_f

bb_f:
  br label %bb_g

bb_g:
  ret i1 false
}

!intel.dtrans.types = !{!0, !6, !10, !13, !15, !17, !19, !22, !25, !27, !29, !31, !34}

!0 = !{!"S", %"class.xalanc_1_10::XalanDOMStringCache" zeroinitializer, i32 5, !1, !1, !2, !3, !5}
!1 = !{%"class.xalanc_1_10::XalanVector" zeroinitializer, i32 0}
!2 = !{i32 0, i32 0}
!3 = !{!"A", i32 4, !4}
!4 = !{i8 0, i32 0}
!5 = !{%"class.xalanc_1_10::XalanDOMStringReusableAllocator" zeroinitializer, i32 0}
!6 = !{!"S", %"class.xalanc_1_10::XalanVector" zeroinitializer, i32 4, !7, !8, !8, !9}
!7 = !{%"class.xercesc_2_7::MemoryManager" zeroinitializer, i32 1}
!8 = !{i64 0, i32 0}
!9 = !{%"class.xalanc_1_10::XalanDOMString" zeroinitializer, i32 2}
!10 = !{!"S", %"class.xercesc_2_7::MemoryManager" zeroinitializer, i32 1, !11}
!11 = !{!12, i32 2}
!12 = !{!"F", i1 true, i32 0, !2}
!13 = !{!"S", %"class.xalanc_1_10::XalanDOMString" zeroinitializer, i32 3, !14, !2, !3}
!14 = !{%"class.xalanc_1_10::XalanVector.0" zeroinitializer, i32 0}
!15 = !{!"S", %"class.xalanc_1_10::XalanVector.0" zeroinitializer, i32 4, !7, !8, !8, !16}
!16 = !{i16 0, i32 1}
!17 = !{!"S", %"class.xalanc_1_10::XalanDOMStringReusableAllocator" zeroinitializer, i32 1, !18}
!18 = !{%"class.xalanc_1_10::ReusableArenaAllocator" zeroinitializer, i32 0}
!19 = !{!"S", %"class.xalanc_1_10::ReusableArenaAllocator" zeroinitializer, i32 3, !20, !4, !21}
!20 = !{%"class.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 0}
!21 = !{!"A", i32 7, !4}
!22 = !{!"S", %"class.xalanc_1_10::ArenaAllocator" zeroinitializer, i32 3, !11, !23, !24}
!23 = !{i16 0, i32 0}
!24 = !{%"class.xalanc_1_10::XalanList" zeroinitializer, i32 0}
!25 = !{!"S", %"class.xalanc_1_10::XalanList" zeroinitializer, i32 3, !7, !26, !26}
!26 = !{%"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XalanDOMString> *>::Node" zeroinitializer, i32 1}
!27 = !{!"S", %"struct.xalanc_1_10::XalanList<xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XalanDOMString> *>::Node" zeroinitializer, i32 3, !28, !26, !26}
!28 = !{%"class.xalanc_1_10::ReusableArenaBlock" zeroinitializer, i32 1}
!29 = !{!"S", %"class.xalanc_1_10::ReusableArenaBlock" zeroinitializer, i32 4, !30, !23, !23, !3}
!30 = !{%"class.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 0}
!31 = !{!"S", %"class.xalanc_1_10::ArenaBlockBase" zeroinitializer, i32 4, !32, !23, !23, !33}
!32 = !{%"class.xalanc_1_10::XalanAllocator" zeroinitializer, i32 0}
!33 = !{%"class.xalanc_1_10::XalanDOMString" zeroinitializer, i32 1}
!34 = !{!"S", %"class.xalanc_1_10::XalanAllocator" zeroinitializer, i32 1, !7}
!35 = !{%__SOADT_EL_class.F zeroinitializer, i32 1}
!36 = distinct !{!37, !33}
!37 = !{%"class.xalanc_1_10::XalanDOMStringCache" zeroinitializer, i32 1}
