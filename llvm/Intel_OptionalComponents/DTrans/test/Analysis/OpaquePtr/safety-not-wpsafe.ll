; REQUIRES: asserts

; Test that checks the safety analysis is not run when the input is not
; marked as Whole Program Safe

; RUN: opt -dtrans-safetyanalyzer -debug-only=dtrans-safetyanalyzer -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -passes='require<dtrans-safetyanalyzer>' -debug-only=dtrans-safetyanalyzer -disable-output %s 2>&1 | FileCheck %s

%struct.testmember01 = type { i64, i64 }
%struct.test01 = type { %struct.testmember01* }
@var_test01 = internal global %struct.test01 zeroinitializer
define void @test01() {
  %mem_i8 = call i8* @malloc(i64 16)
  %mystruct = bitcast i8* %mem_i8 to %struct.testmember01*
  store %struct.testmember01* %mystruct, %struct.testmember01** getelementptr (%struct.test01, %struct.test01* @var_test01, i64 0, i32 0)
  ret void
}

declare i8* @malloc(i64)

!1 = !{i64 0, i32 0}  ; i64
!2 = !{!3, i32 1}  ; %struct.testmember01*
!3 = !{!"R", %struct.testmember01 zeroinitializer, i32 0}  ; %struct.testmember01
!4 = !{!"S", %struct.testmember01 zeroinitializer, i32 2, !1, !1} ; { i64, i64 }
!5 = !{!"S", %struct.test01 zeroinitializer, i32 1, !2} ; { %struct.testmember01* }

!dtrans_types = !{!4, !5}

; CHECK: DTransSafetyInfo::analyzeModule running
; CHECK: DTransSafetyInfo: Not Whole Program Safe
