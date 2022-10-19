; REQUIRES: asserts

; RUN: opt  < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -disable-output -debug-only=dtrans-safetyanalyzer-verbose 2>&1 | FileCheck %s --check-prefixes=CHECK-BC,CHECK-UPS,CHECK-MEA
; RUN: opt  < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -disable-output -debug-only=dtrans-safetyanalyzer-verbose 2>&1 | FileCheck %s --check-prefixes=CHECK-BC,CHECK-UPS,CHECK-MEA

; This test case checks that "Bad casting (related types) -- Pointer type
; for field load/store contains related types" and "Mismatched element access
; (related types) -- Type for field load/store contains related types" were not
; set for the load and store instructions in @foo. The reason is because field
; 0 in %class.TestClass.Outer is a pointer to %class.TestClass, but then the
; pointer is used as %class.SimpleClass, which is not a zero element nor a
; related type of %class.TestClass. Also, it checks that "Unsafe pointer
; store (related types) -- Type for field load/store contains related types"
; was not set for the store instruction.

; CHECK-BC-NOT: Bad casting (related types) -- Pointer type for field load/store contains related types
; CHECK-MEA-NOT: Mismatched element access (related types) -- Type for field load/store contains related types
; CHECK-UPS-NOT: Bad casting (related types) -- Pointer type for field load/store contains related types

; CHECK-BC: dtrans-safety: Bad casting -- Incompatible pointer type for field load/store
; CHECK-BC:   [foo]   %2 = load ptr, ptr %1, align 8
; CHECK-BC: dtrans-safety: Bad casting -- Incompatible pointer type for field load/store
; CHECK-BC:   [foo]   %2 = load ptr, ptr %1, align 8
; CHECK-BC: dtrans-safety: Bad casting -- Incompatible pointer type for field load/store
; CHECK-BC:   [foo]   %2 = load ptr, ptr %1, align 8

; CHECK-MEA: dtrans-safety: Mismatched element access -- Incompatible type for field load/store
; CHECK-MEA:   [foo]   %2 = load ptr, ptr %1, align 8

; CHECK-BC: dtrans-safety: Bad casting -- Incompatible pointer type for field load/store
; CHECK-BC:   [foo]   store ptr %2, ptr %0, align 8
; CHECK-BC: dtrans-safety: Bad casting -- Incompatible pointer type for field load/store
; CHECK-BC:   [foo]   store ptr %2, ptr %0, align 8
; CHECK-BC: dtrans-safety: Bad casting -- Incompatible pointer type for field load/store
; CHECK-BC:   [foo]   store ptr %2, ptr %0, align 8

; CHECK-UPS: dtrans-safety: Unsafe pointer store -- Incompatible type for field load/store
; CHECK-UPS:   [foo]   store ptr %2, ptr %0, align 8
; CHECK-UPS: dtrans-safety: Unsafe pointer store -- Incompatible type for field load/store
; CHECK-UPS:   [foo]   store ptr %2, ptr %0, align 8
; CHECK-UPS: dtrans-safety: Unsafe pointer store -- Incompatible type for field load/store
; CHECK-UPS:   [foo]   store ptr %2, ptr %0, align 8

; CHECK-MEA: dtrans-safety: Mismatched element access -- Incompatible type for field load/store
; CHECK-MEA:   [foo]   store ptr %2, ptr %0, align 8

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.MainClass = type { [4 x i32], i32, [4 x i8]}
%class.MainClass.base = type { [4 x i32], i32}

%class.TestClass = type { %class.MainClass.base, [4 x i8] }
%class.TestClass.Outer = type { ptr, i32 }

%class.SimpleClass = type { [4 x i32], i32, [4 x i8]}

define dso_local void @foo(ptr "intel_dtrans_func_index"="1" %ptr, ptr "intel_dtrans_func_index"="2" %ptr1) !intel.dtrans.func.type !12 {
entry:
  %0 = getelementptr inbounds %class.TestClass.Outer, ptr %ptr, i64 0, i32 0
  %1 = getelementptr inbounds %class.TestClass.Outer, ptr %ptr1, i64 0, i32 0
  %2 = load ptr, ptr %1
  store ptr %2, ptr %0
  %3 = getelementptr inbounds %class.SimpleClass, ptr %2, i64 0, i32 0
  %4 = load i32, ptr %3
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

!8 = !{!"S", %class.TestClass zeroinitializer, i32 2, !7, !4} ; %class.TestClass = type { %class.MainClass.base, [4 x i8] }
!9 = !{%class.TestClass zeroinitializer, i32 1} ; %class.TestClass*

!10 = !{!"S", %class.TestClass.Outer zeroinitializer, i32 2, !9, !0} ;  %class.TestClass.Outer = type { %class.TestClass.inner*, i32 }
!11 = !{%class.TestClass.Outer zeroinitializer, i32 1} ; %class.TestClass.Outer*

!12 = distinct !{!11, !11}

!13 = !{!"S", %class.SimpleClass zeroinitializer, i32 3, !3, !0, !4} ; %class.SimpleClass = type { [4 x i32], i32, [4 x i8]}