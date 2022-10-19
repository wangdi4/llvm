; REQUIRES: asserts

; RUN: opt  < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -debug-only=dtrans-safetyanalyzer-verbose -disable-output 2>&1 | FileCheck %s
; RUN: opt  < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -debug-only=dtrans-safetyanalyzer-verbose -disable-output 2>&1 | FileCheck %s

; This test verifies that %struct.test.a and %struct.test.a.base are
; mapped together since there is a memcpy that won't write into the
; field that is reserved for padding. The goal of this test case is
; to check when a parent structure (%struct.test.b) encapsulates a
; base structure (%struct.test.a.base) and we use memcopy to copy
; data into the padded structure (%struct.test.a).

; CHECK: dtrans-safety: Bad memfunc manipulation (related types) -- memcpy/memmove (related types) - copying data from base to padded structure (or vice-versa)

; CHECK-LABEL: LLVMType: %struct.test.a = type { i32, i32, [4 x i8] }
; CHECK: Related base structure: struct.test.a.base
; CHECK: 2)Field LLVM Type: [4 x i8]
; CHECK: Field info: Written PaddedField
; CHECK: Bottom Alloc Function
; CHECK: Safety data: Structure may have ABI padding | Bad memfunc manipulation (related types){{ *$}}

; CHECK-LABEL: LLVMType: %struct.test.a.base = type { i32, i32 }
; CHECK: Related padded structure: struct.test.a
; CHECK: Safety data: Nested structure | Structure could be base for ABI padding | Bad memfunc manipulation (related types){{ *$}}

; CHECK-LABEL: %struct.test.b = type { %struct.test.a.base, [4 x i8] }
; CHECK: Safety data: Contains nested structure | Bad memfunc manipulation (related types){{ *$}}

; ModuleID = 'simple2.cpp'
source_filename = "simple2.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.test.a = type { i32, i32, [ 4 x i8] }
%struct.test.a.base = type { i32, i32 }

%struct.test.b = type { %struct.test.a.base, [ 4 x i8 ]}

define void @test01(ptr "intel_dtrans_func_index"="1" %ptr, ptr "intel_dtrans_func_index"="2" %ptr2) !intel.dtrans.func.type !13 {
  call void @llvm.memcpy.p0i8.p0i8.i64(ptr %ptr, ptr %ptr2, i64 8, i1 false)
  ret void
}

declare !intel.dtrans.func.type !12 void @llvm.memcpy.p0i8.p0i8.i64(ptr "intel_dtrans_func_index"="1", ptr "intel_dtrans_func_index"="2", i64, i1)


!llvm.module.flags = !{!0, !1, !2, !3}
!intel.dtrans.types = !{!4, !10, !17}

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
!12 = distinct !{!11, !11}
!13 = distinct !{!14, !16}
!14 = !{%struct.test.a zeroinitializer, i32 1}
!15 = !{%struct.test.a.base zeroinitializer, i32 1}
!16 = !{%struct.test.b zeroinitializer, i32 1}
!17 = !{!"S", %struct.test.b zeroinitializer, i32 2, !10, !7}

