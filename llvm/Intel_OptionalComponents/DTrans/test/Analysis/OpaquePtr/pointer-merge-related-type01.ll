; REQUIRES: asserts

; RUN: opt  < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -disable-output -debug-only=dtrans-safetyanalyzer-verbose -dtrans-print-types 2>&1 | FileCheck %s

; This test case checks that "Unsafe pointer merge (related types) -- Merging
; pointers with related types" was set for the PHI node instruction in @foo.
; The PHI node is merging two pointers and then it is used as a related type.
; Also, this test case checks that Unsafe pointer merge (related types) was
; set for %class.MainClass and %class.MainClass.base.

; CHECK: dtrans-safety: Unsafe pointer merge (related types) -- Merging pointers with related types
; CHECK:  [foo]   %phi = phi ptr [ %0, %entry ], [ %1, %br.loop ]

; CHECK:  LLVMType: %class.MainClass = type { [4 x i32], i32, [4 x i8] }
; CHECK:  Safety data: Ambiguous GEP | Structure may have ABI padding | Unsafe pointer merge (related types){{ *$}}

; CHECK:  LLVMType: %class.MainClass.base = type { [4 x i32], i32 }
; CHECK:  Safety data: Ambiguous GEP | Nested structure | Structure could be base for ABI padding | Unsafe pointer merge (related types){{ *$}}


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.MainClass = type { [4 x i32], i32, [4 x i8]}
%class.MainClass.base = type { [4 x i32], i32}

%class.TestClass.Inner = type { %class.MainClass.base, [4 x i8] }

%class.TestClass.Outer = type { ptr, [4 x i8] }

define dso_local void @foo(ptr "intel_dtrans_func_index"="1" %ptr, i1 %var) !intel.dtrans.func.type !12 {
entry:
  %0 = load ptr, ptr %ptr
  br label %br.loop

br.loop:
  %phi = phi ptr [ %0, %entry ], [ %1, %br.loop ]
  %1 = getelementptr inbounds %class.MainClass, ptr %phi, i64 0, i32 0
  br i1 %var, label %br.exit, label %br.loop

br.exit:
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

!8 = !{!"S", %class.TestClass.Inner zeroinitializer, i32 2, !7, !4} ; %class.TestClass.Inner = type { %class.MainClass.base, [4 x i8] }
!9 = !{%class.TestClass.Inner zeroinitializer, i32 1} ; %class.TestClass.Inner

!10 = !{!"S", %class.TestClass.Outer zeroinitializer, i32 2, !9, !4} ;  %class.TestClass.Outer = type { %class.TestClass.Inner*, [4 x i8] }
!11 = !{%class.TestClass.Outer zeroinitializer, i32 1} ; %class.TestClass.Outer*

!12 = distinct !{!11}