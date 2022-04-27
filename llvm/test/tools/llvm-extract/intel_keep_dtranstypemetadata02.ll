; INTEL_FEATURE_SW_DTRANS

; This test is to verify that DTrans metadata for structures that are
; not needed following extraction is removed, allowing for structure
; type definitions that are not needed to also be removed.

; RUN: llvm-extract -keep-dtranstypemetadata -func test -S < %s | FileCheck %s
; RUN: llvm-extract -opaque-pointers -keep-dtranstypemetadata -func test -S < %s | FileCheck %s

%struct.test01 = type { i64, i64 }
%struct.test02 = type { i64, i64 }
; CHECK: %struct.test01 = type { i64, i64 }
; CHECK-NOT: %struct.test02 = type { i64, i64 }

define void @caller(%struct.test01* "intel_dtrans_func_index"="1" %arg) !intel.dtrans.func.type !3 {
  %tmp = alloca %struct.test02
  %gep = getelementptr %struct.test02, %struct.test02* %tmp, i64 0, i32 0
  store i64 0, i64* %gep
  call void @test(%struct.test01* %arg)
  ret void
}

define void @test(%struct.test01* "intel_dtrans_func_index"="1" %arg) !intel.dtrans.func.type !4 {
  %gep = getelementptr %struct.test01, %struct.test01* %arg, i64 0, i32 0
  store i64 0, i64* %gep
  ret void
}

!intel.dtrans.types = !{!5, !6}

; CHECK: !intel.dtrans.types = !{![[MD_TEST01:[0-9]+]]}
; CHECK: ![[MD_TEST01]] = !{!"S", %struct.test01 zeroinitializer, {{.*}}
; CHECK-NOT: !"S", %struct.test02 zeroinitializer

!1 = !{i64 0, i32 0}  ; i64
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = distinct !{!2}
!4 = distinct !{!2}
!5 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i64, i64 }
!6 = !{!"S", %struct.test02 zeroinitializer, i32 2, !1, !1} ; { i64, i64 }

; end INTEL_FEATURE_SW_DTRANS
