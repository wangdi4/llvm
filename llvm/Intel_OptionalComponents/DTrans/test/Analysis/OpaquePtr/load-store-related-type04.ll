; REQUIRES: asserts

; RUN: opt  < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -disable-output -debug-only=dtrans-safetyanalyzer-verbose 2>&1 | FileCheck %s --check-prefixes=CHECK-BC,CHECK-UPS,CHECK-MEA
; RUN: opt  < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -disable-output -debug-only=dtrans-safetyanalyzer-verbose 2>&1 | FileCheck %s --check-prefixes=CHECK-BC,CHECK-UPS,CHECK-MEA

; This test case checks that "Bad casting (related types) -- Pointer type
; for field load/store contains related types", "Unsafe pointer store
; (related types) -- Type for field load/store contains related types", and
; "Unsafe pointer store (related types) -- Type for field load/store contains
; related types" were not set for the store instruction in @foo. The reason is
; because the allocated space in %0 is expected to be used as a pointer to
; %class.TestClass, but is it used in %2 as %class.SimpleClass, which is not a
; zero element nor a related type of %class.TestClass.

; CHECK-BC-NOT: Bad casting (related types) -- Pointer type for field load/store contains related types
; CHECK-MEA-NOT: Mismatched element access (related types) -- Type for field load/store contains related types
; CHECK-UPS-NOT: Bad casting (related types) -- Pointer type for field load/store contains related types

; CHECK-BC: dtrans-safety: Bad casting -- Incompatible pointer type for field load/store
; CHECK-BC:   [foo]   store ptr %phi, ptr %1, align 8
; CHECK-BC: dtrans-safety: Bad casting -- Incompatible pointer type for field load/store
; CHECK-BC:   [foo]   store ptr %phi, ptr %1, align 8
; CHECK-BC: dtrans-safety: Bad casting -- Incompatible pointer type for field load/store
; CHECK-BC:   [foo]   store ptr %phi, ptr %1, align 8
; CHECK-BC: dtrans-safety: Bad casting -- Incompatible pointer type for field load/store
; CHECK-BC:   [foo]   store ptr %phi, ptr %1, align 8

; CHECK-UPS:dtrans-safety: Unsafe pointer store -- Incompatible type for field load/store
; CHECK-UPS:  [foo]   store ptr %phi, ptr %1, align 8
; CHECK-UPS:dtrans-safety: Unsafe pointer store -- Incompatible type for field load/store
; CHECK-UPS:  [foo]   store ptr %phi, ptr %1, align 8
; CHECK-UPS:dtrans-safety: Unsafe pointer store -- Incompatible type for field load/store
; CHECK-UPS:  [foo]   store ptr %phi, ptr %1, align 8
; CHECK-UPS:dtrans-safety: Unsafe pointer store -- Incompatible type for field load/store
; CHECK-UPS:  [foo]   store ptr %phi, ptr %1, align 8

; CHECK-MEA: dtrans-safety: Mismatched element access -- Incompatible type for field load/store
; CHECK-MEA:   [foo]   store ptr %phi, ptr %1, align 8

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.MainClass = type { [4 x i32], i32, [4 x i8]}
%class.MainClass.base = type { [4 x i32], i32}

%class.TestClass = type { %class.MainClass.base, [4 x i8] }
%class.TestClass.Outer = type { ptr, i32 }

%class.SimpleClass = type { [4 x i32], i32, [4 x i8]}

define dso_local void @foo(ptr "intel_dtrans_func_index"="1" %ptr, i1 %var) !intel.dtrans.func.type !12 {
entry:
  br i1 %var, label %br.true, label %br.continue

br.true:
  %0 = tail call noalias noundef nonnull ptr @_Znwm(i64 24)
  br label %br.continue

br.continue:
  %phi = phi ptr [ null, %entry ], [ %0, %br.true ]
  %1 = getelementptr inbounds %class.TestClass.Outer, ptr %ptr, i64 0, i32 0
  store ptr %phi, ptr %1
  %2 = getelementptr inbounds %class.SimpleClass, ptr %phi, i64 0, i32 0
  %3 = load i32, ptr %2
  ret void
}

declare !intel.dtrans.func.type !13 "intel_dtrans_func_index"="1" ptr @_Znwm(i64)
declare i32 @__gxx_personality_v0(...)

!intel.dtrans.types = !{!5, !6, !8, !10, !14}

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

!12 = distinct !{!11}
!13 = distinct !{!2}

!14 = !{!"S", %class.SimpleClass zeroinitializer, i32 3, !3, !0, !4} ; %class.SimpleClass = type { [4 x i32], i32, [4 x i8]}