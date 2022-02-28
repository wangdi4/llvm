; INTEL_FEATURE_SW_DTRANS

; This test is to verify that !intel.dtrans.func.type information is preserved
; on functions that are converted from 'define' to 'declare' during extraction.
;
; @caller - should be removed
; @test   - should be kept with metadata attachments
; @callee - should be converted to 'declare' with metadata attachments

; Verify metadata is kept for declarations when llvm-extract is using textual IR
; RUN: llvm-extract -keep-dtranstypemetadata -func test -S < %s | FileCheck %s
; RUN: llvm-extract -opaque-pointers -keep-dtranstypemetadata -func test -S < %s | FileCheck %s

; Verify metadata is kept for declarations when llvm-extract is using bitcode. This
; requires that all functions get materialized by llvm-extract before the extraction
; begins.
; RUN: llvm-as -o - < %s | llvm-extract -keep-dtranstypemetadata -func test -S | FileCheck %s
; RUN: llvm-as -opaque-pointers -o - < %s | llvm-extract -opaque-pointers -keep-dtranstypemetadata -func test -S | FileCheck %s

%struct.test01 = type { i64, i64 }

define void @caller(%struct.test01* "intel_dtrans_func_index"="1" %arg) !intel.dtrans.func.type !3 {
  call void @test(%struct.test01* %arg)
  ret void
}
; CHECK-NOT: @caller

define void @test(%struct.test01* "intel_dtrans_func_index"="1" %arg) !intel.dtrans.func.type !4 {
  call void @callee(%struct.test01* %arg)
  ret void
}
; CHECK: define void @test({{.*}} "intel_dtrans_func_index"="1" %arg) !intel.dtrans.func.type !{{[0-9]+}}

define void @callee(%struct.test01* "intel_dtrans_func_index"="1" %arg) !intel.dtrans.func.type !5 {
  ret void
}
; CHECK: declare !intel.dtrans.func.type !{{[0-9]+}} void @callee({{.*}} "intel_dtrans_func_index"="1")

!intel.dtrans.types = !{!6}

!1 = !{i64 0, i32 0}  ; i64
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = distinct !{!2}
!4 = distinct !{!2}
!5 = distinct !{!2}
!6 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i64, i64 }

; end INTEL_FEATURE_SW_DTRANS
