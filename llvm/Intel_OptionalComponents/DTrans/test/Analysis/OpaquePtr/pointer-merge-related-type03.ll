; REQUIRES: asserts

; RUN: opt  < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -disable-output -debug-only=dtrans-safetyanalyzer-verbose 2>&1 | FileCheck %s
; RUN: opt  < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -disable-output -debug-only=dtrans-safetyanalyzer-verbose 2>&1 | FileCheck %s

; This test case checks that "Unsafe pointer merge (related types) -- Merging
; pointers with related types" was not set for the PHI node instruction in
; @foo. One of the incoming values of the PHI node (%0) is used as a type
; that is not in the zero element chain of %class.TestClass.Outer, nor
; as related type.

; CHECK-NOT: dtrans-safety: Unsafe pointer merge (related types) -- Merging pointers with related types
; CHECK: dtrans-safety: Unsafe pointer merge -- Merge of conflicting pointer types
; CHECK:  [foo]   %phi = phi ptr [ %0, %entry ], [ %2, %br.loop ]

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.MainClass = type { [4 x i32], i32, [4 x i8]}
%class.MainClass.base = type { [4 x i32], i32}

%class.TestClass.Inner = type { %class.MainClass.base, [4 x i8] }
%class.TestClass.Outer = type { ptr, [4 x i8] }

%class.SimpleClass = type { [4 x i32], i32, [4 x i8]}

define dso_local void @foo(ptr "intel_dtrans_func_index"="1" %ptr, i1 %var) !intel.dtrans.func.type !12 {
entry:
  %0 = load ptr, ptr %ptr
  %1 = getelementptr inbounds %class.SimpleClass, ptr %0, i64 0, i32 0
  br label %br.loop

br.loop:
  %phi = phi ptr [ %0, %entry ], [ %2, %br.loop ]
  %2 = getelementptr inbounds %class.MainClass, ptr %phi, i64 0, i32 0
  br i1 %var, label %br.exit, label %br.loop

br.exit:
  ret void
}

!intel.dtrans.types = !{!5, !6, !8, !10, !13}

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

!13 = !{!"S", %class.SimpleClass zeroinitializer, i32 3, !3, !0, !4} ; %class.SimpleClass = type { [4 x i32], i32, [4 x i8]}