; REQUIRES: asserts
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-outofboundsok=false -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; This test verifies that BadCasting is not set for parent class
; (i.e ValueStore) and other fields recursively (i.e FieldValueMap) even
; though expected type (%IdentityConstraint*) and used type (%IC_KeyRef*)
; of %i1 don't match. The parent class is not involved in any BadCasting
; in this test as it is used only to get address of a field. Only either %i
; or %i1 is involved in BadCasting. Skip BadCasting for parent struct only
; when expected type and used type are related types. (Special case)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%ValueStore = type { i8, i32, ptr, %FieldValueMap }
%FieldValueMap = type { i32, i32 }
%IC_KeyRef = type { %IdentityConstraint.base, ptr }
%IdentityConstraint.base = type <{ %XSerializable, ptr, i32 }>
%XSerializable = type { i32 }
%IdentityConstraint = type <{ %XSerializable, ptr, i32, [4 x i8] }>

define void @_ZN11xercesc_2_710ValueStore20endDcocumentFragmentEPNS_15ValueStoreCacheE(ptr "intel_dtrans_func_index"="1" %arg) !intel.dtrans.func.type !14 {
bb:
  %i = getelementptr inbounds %ValueStore, ptr %arg, i64 0, i32 2
  %i1 = load ptr, ptr %i, align 8
  %i2 = getelementptr inbounds %IC_KeyRef, ptr %i1, i64 0, i32 1
  ret void
}

; CHECK: LLVMType: %FieldValueMap = type { i32, i32 }
; CHECK:   Safety data: Nested structure{{ *$}}

; CHECK:   LLVMType: %IC_KeyRef = type { %IdentityConstraint.base, ptr }
; CHECK:  Safety data: Bad casting | Ambiguous GEP | Contains nested structure{{ *$}}

; CHECK:  LLVMType: %IdentityConstraint = type <{ %XSerializable, ptr, i32, [4 x i8] }>
; CHECK: Safety data: Bad casting | Ambiguous GEP | Mismatched element access | Contains nested structure | Structure may have ABI padding{{ *$}}

; CHECK: LLVMType: %IdentityConstraint.base = type <{ %XSerializable, ptr, i32 }>
; CHECK: Safety data: Bad casting | Ambiguous GEP | Nested structure | Contains nested structure | Structure could be base for ABI padding{{ *$}}

; CHECK: LLVMType: %ValueStore = type { i8, i32, ptr, %FieldValueMap }
; CHECK:  Safety data: Mismatched element access | Contains nested structure{{ *$}}

!intel.dtrans.types = !{!0, !2, !7, !10, !11, !12}

!0 = !{!"S", %XSerializable zeroinitializer, i32 1, !1}
!1 = !{i32 0, i32 0}
!2 = !{!"S", %IdentityConstraint zeroinitializer, i32 4, !3, !4, !1, !5}
!3 = !{%XSerializable zeroinitializer, i32 0}
!4 = !{i16 0, i32 1}
!5 = !{!"A", i32 4, !6}
!6 = !{i8 0, i32 0}
!7 = !{!"S", %ValueStore zeroinitializer, i32 4, !6, !1, !8, !9}
!8 = !{%IdentityConstraint zeroinitializer, i32 1}
!9 = !{%FieldValueMap zeroinitializer, i32 0}
!10 = !{!"S", %FieldValueMap zeroinitializer, i32 2, !1, !1}
!11 = !{!"S", %IdentityConstraint.base zeroinitializer, i32 3, !3, !4, !1}
!12 = !{!"S", %IC_KeyRef zeroinitializer, i32 2, !13, !8}
!13 = !{%IdentityConstraint.base zeroinitializer, i32 0}
!14 = distinct !{!15}
!15 = !{%ValueStore zeroinitializer, i32 1}
