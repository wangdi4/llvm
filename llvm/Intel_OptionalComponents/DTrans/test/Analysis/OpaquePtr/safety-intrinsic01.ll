; This test verifies that argument of intrinsic call is not marked as
; Unhandled if argument is not value of interest. Makes sure
; "Unhandled use -- Value passed to intrinsic" is not set.

; REQUIRES: asserts

; RUN: opt -opaque-pointers -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -debug-only=dtrans-safetyanalyzer-verbose -disable-output %s 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK-NOT: dtrans-safety: Unhandled use -- Value passed to intrinsic
; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %class.XMLDeleter

%"class.XMLDeleter" = type { i32 }

define internal void @_ZN11xercesc_2_712DGXMLScanner12scanCharDataERNS_9XMLBufferE(ptr "intel_dtrans_func_index"="1" %0) !intel.dtrans.func.type !2 {
blk:
  %1 = load i32, ptr null, align 4
  br label %2

2:                                                ; preds = %0
  %3 = call i32 @llvm.umax.i32(i32 %1, i32 0)
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare i32 @llvm.umax.i32(i32, i32) #0

!intel.dtrans.types = !{!0}

!0 = !{!"S", %"class.XMLDeleter" zeroinitializer, i32 1, !1}
!1 = !{i32 0, i32 0}
!2 = !{!3}
!3 = !{%"class.XMLDeleter" zeroinitializer, i32 1}
