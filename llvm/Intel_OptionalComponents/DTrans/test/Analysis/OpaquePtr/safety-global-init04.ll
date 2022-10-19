; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test initializing an array of structure pointers with an incompatible pointer type.

%struct.test01b = type { i64 }
%struct.test01a = type { i32, i32 }

@var01b = internal global %struct.test01b zeroinitializer
@var01a = internal global [2 x %struct.test01a*] [%struct.test01a* bitcast (%struct.test01b* @var01b to %struct.test01a*),
                                                  %struct.test01a* bitcast (%struct.test01b* @var01b to %struct.test01a*)], !intel_dtrans_type !0

; CHECK-LABEL: LLVMType: %struct.test01a = type { i32, i32 }
; CHECK: Safety data: Unsafe pointer store | Global pointer{{ *$}}
; CHECK: End LLVMType: %struct.test01a

; CHECK-LABEL: LLVMType: %struct.test01b = type { i64 }
; CHECK: Safety data: Bad casting | Global instance{{ *$}}
; CHECK: End LLVMType: %struct.test01b

!0 = !{!"A", i32 2, !1} ; [2 x %struct.test01a*]
!1 = !{%struct.test01a zeroinitializer, i32 1} ; %struct.test01a*
!2 = !{!"S", %struct.test01a zeroinitializer, i32 2, !3, !3}
!3 = !{i32 0, i32 0} ; i32
!4 = !{!"S", %struct.test01b zeroinitializer, i32 1, !5}
!5 = !{i64 0, i32 0} ; i64

!intel.dtrans.types = !{!2, !4}
