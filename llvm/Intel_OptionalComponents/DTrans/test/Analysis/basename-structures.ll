; REQUIRES: asserts

; RUN: opt < %s -whole-program-assume -dtransanalysis -dtrans-print-types -S 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -S 2>&1 | FileCheck %s

; This test case checks that the type-info for %class.simple was generated
; even if there is no use for it in the module.

%struct.simple = type { %struct.test }
%struct.simple.1 = type { %struct.test }

%struct.test = type { i32, i64, i32 }

declare void @foo(%struct.test* %0)

define void @bar(%struct.simple.1* %tmp0) {
  %tmp1 = bitcast %struct.simple.1* %tmp0 to %struct.test*
  call void @foo(%struct.test* %tmp1)
  ret void
}

; Check that the DTrans type info for %class.simple was generated

; CHECK: LLVMType: %struct.simple = type { %struct.test }
; CHECK: Safety data: Contains nested structure

; CHECK: LLVMType: %struct.simple.1 = type { %struct.test }
; CHECK: Safety data: Contains nested structure | Field address taken call

; CHECK: LLVMType: %struct.test = type { i32, i64, i32 }
; CHECK: Safety data: Address taken | Nested structure | Field address taken call

; Check that the type %class.simple was removed from the IR

; CHECK-NOT: %struct.simple = type { %struct.test }
