; UNSUPPORTED: enable-opaque-pointers

; This test verifies that alignment of "bar" is NOT increased to 64.
; This test is same as code_align_01.ll except annotation for
; SOAToAOS transformation is missing.

; RUN: opt < %s -dtrans-codealign -whole-program-assume -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -S 2>&1 | FileCheck %s
; RUN: opt < %s -passes=dtrans-codealign -whole-program-assume -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -S 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: define dso_local void @foo
; CHECK: align 2
; CHECK-NOT: align 64

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
%"struct.xalanc_1_10::ReusableArenaBlock<xalanc_1_10::XalanDOMString>::NextBlock" = type { i16, i32 }

define dso_local void @foo() {
  call i1 @bar(%"class.xalanc_1_10::XalanDOMStringCache"* null, %"class.xalanc_1_10::XalanDOMString"* null)
  call i1 @bar(%"class.xalanc_1_10::XalanDOMStringCache"* null, %"class.xalanc_1_10::XalanDOMString"* null)
  ret void
}

; Function Attrs: uwtable
define dso_local zeroext i1 @bar(%"class.xalanc_1_10::XalanDOMStringCache"* %this, %"class.xalanc_1_10::XalanDOMString"* %theString) align 2 {
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
