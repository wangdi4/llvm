; REQUIRES: asserts

; RUN: opt -disable-output -whole-program-assume -dtrans-ptrtypeanalyzertest -debug-only=dtrans-pta-verbose < %s 2>&1 | FileCheck %s --check-prefix=CHECK
; RUN: opt -disable-output -whole-program-assume -passes=dtrans-ptrtypeanalyzertest -debug-only=dtrans-pta-verbose < %s 2>&1 | FileCheck %s --check-prefix=CHECK

; Test type recovery for function definitions with and without metadata

%struct.test01 = type { i64, i32, i32 }

; Check a function definition that returns a pointer type
define internal %struct.test01* @test01() !dtrans_type !3 {
  %mem = call i8* @malloc(i64 16)
  %st = bitcast i8* %mem to %struct.test01*
  ret %struct.test01* %st
}
; CHECK: @test01 - Added alias [DECL]: %struct.test01* ()*
; CHECK: @test01 - Added alias [USE]: %struct.test01* ()*


; Check a function definition that has a pointer type parameter
define internal void @test02(%struct.test01* %in) !dtrans_type !4 {
  ret void
}
; CHECK: @test02 - Added alias [DECL]: void (%struct.test01*)*
; CHECK: @test02 - Added alias [USE]: void (%struct.test01*)*


; Check a function definition that can created without the need for metadata.
define internal void @test03(i32 %in1, double %in2) {
  ret void
}
; CHECK: @test03 - Added alias [DECL]: void (i32, double)*
; CHECK: @test03 - Added alias [USE]: void (i32, double)*


declare i8* @malloc(i64)

!1 = !{!2, i32 1}  ; %struct.test01*
!2 = !{!"R", %struct.test01 zeroinitializer, i32 0}  ; %struct.test01
!3 = !{!"F", i1 false, i32 0, !1}  ; %struct.test01* ()
!4 = !{!"F", i1 false, i32 1, !5, !1}  ; void (%struct.test01*)
!5 = !{!"void", i32 0}  ; void
!6 = !{i64 0, i32 0}  ; i64
!7 = !{i32 0, i32 0}  ; i32
!8 = !{!"S", %struct.test01  zeroinitializer, i32 3, !6, !7, !7} ; { i64, i32, i32 }

!dtrans_types = !{!8}
