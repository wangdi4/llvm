; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test initializing an array of structure pointers with an incompatible pointer type.

%struct.test01a = type { i32, i32 }

@var01b = internal global i32 zeroinitializer
@var01a = internal global [2 x ptr] [ptr @var01b, ptr @var01b], !intel_dtrans_type !0

; CHECK-LABEL: LLVMType: %struct.test01a = type { i32, i32 }
; CHECK: Safety data: Unsafe pointer store | Global pointer{{ *$}}
; CHECK: End LLVMType: %struct.test01a 

!0 = !{!"A", i32 2, !1} ; [2 x %struct.test01a*]
!1 = !{%struct.test01a zeroinitializer, i32 1} ; %struct.test01a*
!2 = !{!"S", %struct.test01a zeroinitializer, i32 2, !3, !3}
!3 = !{i32 0, i32 0} ; i32

!intel.dtrans.types = !{!2}
