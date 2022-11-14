; This test verifies that AddressTaken is not set for a struct when destructor
; returns the struct pointer as i8*.

; REQUIRES: asserts

; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed  -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -dtrans-usecrulecompat -disable-output < %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test = type { ptr }
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test

%"struct.test" = type { ptr }

define hidden "intel_dtrans_func_index"="1" ptr @foo(i8* "intel_dtrans_func_index"="2" %obj) #1  !intel.dtrans.func.type !2 {
  call void @free(i8* %obj)
  ret ptr %obj
}

; Function Attrs: allockind("free")
declare !intel.dtrans.func.type !2 void @free(i8* "intel_dtrans_func_index"="1") #0

attributes #0 = { allockind("free") "alloc-family"="malloc" }
attributes #1 = { "intel-mempool-destructor" }

!intel.dtrans.types = !{!3}

!0 = distinct !{!1}
!1 = !{i8 0, i32 1}
!2 = distinct !{!1, !4}
!3 = !{!"S", %"struct.test" zeroinitializer, i32 1, !1}
!4 = !{%"struct.test" zeroinitializer, i32 1}
