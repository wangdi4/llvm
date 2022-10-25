; REQUIRES: asserts

; RUN: opt  < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -debug-only=dtrans-safetyanalyzer-verbose -disable-output 2>&1 | FileCheck %s

; This test verifies that %class.TestClass.Outer, %struct.test.a and
; %struct.test.a.base are set to "Bad memfunc manipulation (related types)"
; since we can't identify the dominant type for the input pointer
; to the memset's call, but we can identify that %class.TestClass.Outer
; encapsulates all the declaration and use aliases. Also, this test case
; checks that the safety violations were set correctly because the size
; used for memset is the same as the zero element of %struct.test.a.

; CHECK: dtrans-safety: Bad memfunc manipulation (related types) -- memset used in a structure with related type
; CHECK:   [test01]   call void @llvm.memset.p0.i64(ptr %ptr, i8 1, i64 4, i1 false)

; CHECK-LABEL: LLVMType: %class.TestClass.Outer = type { %struct.test.a.base, [4 x i8] }
; CHECK:   Safety data: Ambiguous GEP | Contains nested structure | Bad memfunc manipulation (related types){{ *$}}

; CHECK-LABEL: LLVMType: %struct.test.a = type { i32, i32, [4 x i8] }
; CHECK: Related base structure: struct.test.a.base
; CHECK: 2)Field LLVM Type: [4 x i8]
; CHECK: Field info: PaddedField
; CHECK: Top Alloc Function
; CHECK: Safety data: Ambiguous GEP | Structure may have ABI padding | Bad memfunc manipulation (related types){{ *$}}

; CHECK-LABEL: LLVMType: %struct.test.a.base = type { i32, i32 }
; CHECK: Related padded structure: struct.test.a
; CHECK: Safety data: Ambiguous GEP | Nested structure | Structure could be base for ABI padding | Bad memfunc manipulation (related types){{ *$}}

; ModuleID = 'simple2.cpp'
source_filename = "simple2.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.test.a = type { i32, i32, [4 x i8] }
%struct.test.a.base = type { i32, i32 }

%class.TestClass.Outer = type { %struct.test.a.base, [4 x i8] }

define void @test01(ptr "intel_dtrans_func_index"="1" %ptr) !intel.dtrans.func.type !13 {
entry:
  %0 = getelementptr inbounds %struct.test.a, ptr %ptr, i64 0, i32 0
  %1 = load i32, ptr %0
  call void @llvm.memset.p0i8.i64(ptr %ptr, i8 1, i64 4, i1 false)
  ret void
}

declare !intel.dtrans.func.type !12 void @llvm.memset.p0i8.i64(ptr "intel_dtrans_func_index"="1", i8, i64, i1)


!llvm.module.flags = !{!0, !1, !2, !3}
!intel.dtrans.types = !{!4, !10, !16}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{i32 1, !"ThinLTO", i32 0}
!3 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!4 = !{!"S", %struct.test.a zeroinitializer, i32 3, !6, !6, !7}
!5 = !{%struct.test.a zeroinitializer, i32 0}
!6 = !{i32 0, i32 0}
!7 = !{!"A", i32 4, !8}
!8 = !{i8 0, i32 0}
!9 = !{%struct.test.a.base zeroinitializer, i32 0}
!10 = !{!"S", %struct.test.a.base zeroinitializer, i32 2, !6, !6}
!11 = !{i8 0, i32 1}  ; i8*
!12 = distinct !{!11}
!13 = distinct !{!17}
!14 = !{%struct.test.a zeroinitializer, i32 1}
!15 = !{%struct.test.a.base zeroinitializer, i32 1}
!16 = !{!"S", %class.TestClass.Outer zeroinitializer, i32 2, !9, !7}
!17 = !{%class.TestClass.Outer zeroinitializer, i32 1}
