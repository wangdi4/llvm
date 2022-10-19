; REQUIRES: asserts

; RUN: opt  < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -disable-output -dtrans-print-types -debug-only=dtrans-safetyanalyzer-verbose 2>&1 | FileCheck %s
; RUN: opt  < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -disable-output -dtrans-print-types -debug-only=dtrans-safetyanalyzer-verbose 2>&1 | FileCheck %s


; This test case checks that "Unsafe pointer store (related types) -- Type for
; field load/store contains related types" was set for the store instruction in
; @foo, but the structure %class.MainClass was set to "Unsafe pointer store"
; due to write in the padded field in @bar. The main goal of this test case is
; to check that "Unsafe pointer store (related types)" was reverted correctly
; into "Unsafe pointer store".

; CHECK: dtrans-safety: Unsafe pointer store (related types) -- Type for field load/store contains related types
; CHECK:   [foo]   store ptr %2, ptr %0, align 8

; CHECK: LLVMType: %class.MainClass = type { [4 x i32], i32, [4 x i8] }
; CHECK: Safety data: Bad casting | Ambiguous GEP | Mismatched element access | Unsafe pointer store{{.*}}

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.MainClass = type { [4 x i32], i32, [4 x i8]}
%class.MainClass.base = type { [4 x i32], i32}

%class.TestClass = type { %class.MainClass.base, [4 x i8] }
%class.TestClass.Outer = type { ptr, i32 }

define dso_local void @foo(ptr "intel_dtrans_func_index"="1" %ptr, ptr "intel_dtrans_func_index"="2" %ptr1) !intel.dtrans.func.type !12 {
entry:
  %0 = getelementptr inbounds %class.TestClass.Outer, ptr %ptr, i64 0, i32 0
  %1 = getelementptr inbounds %class.TestClass.Outer, ptr %ptr1, i64 0, i32 0
  %2 = load ptr, ptr %1
  store ptr %2, ptr %0
  %3 = getelementptr inbounds %class.MainClass, ptr %2, i64 0, i32 0
  %4 = load i32, ptr %3
  ret void
}

define dso_local void @bar(ptr "intel_dtrans_func_index"="1" %ptr) !intel.dtrans.func.type !13 {
entry:
  %0 = getelementptr inbounds %class.TestClass.Outer, ptr %ptr, i64 0, i32 0
  %1 = load ptr, ptr %0
  %2 = getelementptr inbounds %class.TestClass, ptr %1, i64 0, i32 0
  %3 = getelementptr inbounds %class.MainClass, ptr %2, i64 0, i32 2
  %4 = load i32, ptr %3
  ret void
}

!intel.dtrans.types = !{!5, !6, !8, !10}

!0 = !{i32 0, i32 0} ; i32
!1 = !{i8 0, i32 0}  ; i8
!2 = !{i8 0, i32 1}  ; i8*
!3 = !{!"A", i32 4, !0} ; [4 x i32]
!4 = !{!"A", i32 4, !1} ; [4 x i8]

!5 = !{!"S", %class.MainClass.base zeroinitializer, i32 2, !3, !0} ; %class.MainClass.base = type { [4 x i32], i32}
!6 = !{!"S", %class.MainClass zeroinitializer, i32 3, !3, !0, !4} ; %class.MainClass = type { [4 x i32], i32, [4 x i8]}
!7 = !{%class.MainClass.base zeroinitializer, i32 0} ; %class.MainClass.base

!8 = !{!"S", %class.TestClass zeroinitializer, i32 2, !7, !4} ; %class.TestClass = type { %class.MainClass.base, [4 x i8] }
!9 = !{%class.TestClass zeroinitializer, i32 1} ; %class.TestClass*

!10 = !{!"S", %class.TestClass.Outer zeroinitializer, i32 2, !9, !0} ;  %class.TestClass.Outer = type { %class.TestClass.inner*, i32 }
!11 = !{%class.TestClass.Outer zeroinitializer, i32 1} ; %class.TestClass.Outer*

!12 = distinct !{!11, !11}
!13 = distinct !{!11}

