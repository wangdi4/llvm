; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -debug-only=dtrans-pta-verbose < %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -debug-only=dtrans-pta-verbose < %s 2>&1 | FileCheck %s

; Test type recovery for function definitions with and without metadata

%struct.test01 = type { i64, i32, i32 }

; Check a function definition that returns a pointer type
define internal "intel_dtrans_func_index"="1" %struct.test01* @test01() !intel.dtrans.func.type !4 {
  %mem = call i8* @malloc(i64 16)
  %st = bitcast i8* %mem to %struct.test01*
  ret %struct.test01* %st
}
; CHECK: Added alias [DECL]: @test01 -- %struct.test01* ()*
; CHECK: Added alias [USE]: @test01 -- %struct.test01* ()*


; Check a function definition that has a pointer type parameter
define internal void @test02(%struct.test01* "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !5 {
  ret void
}
; CHECK: Added alias [DECL]: @test02 -- void (%struct.test01*)*
; CHECK: Added alias [USE]: @test02 -- void (%struct.test01*)*


; Check a function definition that can created without the need for metadata.
define internal void @test03(i32 %in1, double %in2) {
  ret void
}
; CHECK: Added alias [DECL]: @test03 -- void (i32, double)*
; CHECK: Added alias [USE]: @test03 -- void (i32, double)*


declare void @llvm.directive.region.exit(token)
; CHECK: Added alias [DECL]: @llvm.directive.region.exit -- void (token)*
; CHECK: Added alias [USE]: @llvm.directive.region.exit -- void (token)*


declare !intel.dtrans.func.type !7 "intel_dtrans_func_index"="1" i8* @malloc(i64)

!1 = !{i64 0, i32 0}  ; i64
!2 = !{i32 0, i32 0}  ; i32
!3 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!4 = distinct !{!3}
!5 = distinct !{!3}
!6 = !{i8 0, i32 1}  ; i8*
!7 = distinct !{!6}
!8 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !2, !2} ; { i64, i32, i32 }

!intel.dtrans.types = !{!8}
