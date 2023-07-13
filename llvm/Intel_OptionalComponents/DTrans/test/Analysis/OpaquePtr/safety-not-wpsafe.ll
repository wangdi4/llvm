; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; Test that checks the safety analysis is not run when the input is not
; marked as Whole Program Safe

; RUN: opt -passes='require<dtrans-safetyanalyzer>' -debug-only=dtrans-safetyanalyzer -disable-output %s 2>&1 | FileCheck %s

%struct.testmember01 = type { i64, i64 }
%struct.test01 = type { ptr }
@var_test01 = internal global %struct.test01 zeroinitializer
define void @test01() {
  %mem_i8 = call ptr @malloc(i64 16)
  store ptr %mem_i8, ptr getelementptr (%struct.test01, ptr @var_test01, i64 0, i32 0)
  ret void
}

declare !intel.dtrans.func.type !4 "intel_dtrans_func_index"="1" ptr @malloc(i64)

; CHECK: DTransSafetyInfo::analyzeModule running
; CHECK: DTransSafetyInfo: Not Whole Program Safe
!1 = !{i64 0, i32 0}  ; i64
!2 = !{%struct.testmember01 zeroinitializer, i32 1}  ; %struct.testmember01*
!3 = !{i8 0, i32 1}  ; i8*
!4 = distinct !{!3}
!5 = !{!"S", %struct.testmember01 zeroinitializer, i32 2, !1, !1} ; { i64, i64 }
!6 = !{!"S", %struct.test01 zeroinitializer, i32 1, !2} ; { %struct.testmember01* }

!intel.dtrans.types = !{!5, !6}
