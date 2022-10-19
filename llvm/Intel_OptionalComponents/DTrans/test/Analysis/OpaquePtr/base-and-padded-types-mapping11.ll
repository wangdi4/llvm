; REQUIRES: asserts

; RUN: opt  < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt  < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; This test verifies that %struct.test.a and %struct.test.a.base are NOT
; mapped together since there is a memcpy that could write in the field
; that is reserved for padding.

; CHECK-LABEL: LLVMType: %struct.test.a = type { i32, i32, [4 x i8] }
; CHECK-NOT: Related base structure: struct.test.a.base
; CHECK: 2)Field LLVM Type: [4 x i8]
; CHECK-NOT: Field info: PaddedField
; CHECK: Bottom Alloc Function
; CHECK-NOT: Safety data: {{.*}}Structure may have ABI padding{{.*}}

; CHECK-LABEL: LLVMType: %struct.test.a.base = type { i32, i32 }
; CHECK-NOT: Related padded structure: struct.test.a
; CHECK-NOT: Safety data: {{.*}}Structure could be base for ABI padding{{.*}}

; ModuleID = 'simple2.cpp'
source_filename = "simple2.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.test.a = type { i32, i32, [4 x i8] }
%struct.test.a.base = type { i32, i32 }

define void @test01(ptr "intel_dtrans_func_index"="1" %ptr, ptr "intel_dtrans_func_index"="2" %ptr2) !intel.dtrans.func.type !13 {
  call void @llvm.memcpy.p0i8.p0i8.i64(ptr %ptr, ptr %ptr2, i64 10, i1 false)
  ret void
}

declare !intel.dtrans.func.type !12 void @llvm.memcpy.p0i8.p0i8.i64(ptr "intel_dtrans_func_index"="1", ptr "intel_dtrans_func_index"="2", i64, i1)

!llvm.module.flags = !{!0, !1, !2, !3}
!intel.dtrans.types = !{!4, !10}

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
!13 = distinct !{!14, !15}
!14 = !{%struct.test.a zeroinitializer, i32 1}
!15 = !{%struct.test.a.base zeroinitializer, i32 1}

