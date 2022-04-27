; INTEL_FEATURE_SW_DTRANS

; This test is verify that all DTrans type metadata is removed during extraction.

; RUN: llvm-extract -keep-dtranstypemetadata=false -func test -S < %s | FileCheck %s
; RUN: llvm-extract -opaque-pointers -keep-dtranstypemetadata=false -func test -S < %s | FileCheck %s

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
; CHECK: define void @test({{.*}} %arg)
; CHECK-NOT: !intel.dtrans.func.type

define void @callee(%struct.test01* "intel_dtrans_func_index"="1" %arg) !intel.dtrans.func.type !5 {
  ret void
}
; CHECK: declare void @callee({{.*}})

!intel.dtrans.types = !{!6}
; CHECK-NOT: !intel.dtrans.types

!1 = !{i64 0, i32 0}  ; i64
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = distinct !{!2}
!4 = distinct !{!2}
!5 = distinct !{!2}
!6 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i64, i64 }

; end INTEL_FEATURE_SW_DTRANS
