; INTEL_FEATURE_SW_DTRANS

; Test that DTrans type metadata is preserved on a global variable during
; extraction.
;
; Verify metadata is kept for declarations when llvm-extract is using textual IR
; RUN: llvm-extract -keep-dtranstypemetadata -func test -S < %s | FileCheck %s
; RUN: llvm-extract -opaque-pointers -keep-dtranstypemetadata -func test -S < %s | FileCheck %s

; Verify metadata is kept for declarations when llvm-extract is using bitcode. This
; requires that all functions get materialized by llvm-extract before the extraction
; begins.
; RUN: llvm-as -o - < %s | llvm-extract -keep-dtranstypemetadata -func test -S | FileCheck %s
; RUN: llvm-as -opaque-pointers -o - < %s | llvm-extract -opaque-pointers -keep-dtranstypemetadata -func test -S | FileCheck %s

%struct.test01 = type { i64, i64 }
%struct.test02 = type { i64, i64 }

@gVar0 = internal global %struct.test01 zeroinitializer
@gVar1 = internal global %struct.test01* @gVar0, !intel_dtrans_type !2
@gVar2 = internal global %struct.test02* zeroinitializer, !intel_dtrans_type !3
; CHECK: @gVar1 =
; CHECK-SAME: !intel_dtrans_type ![[MD_TEST01:[0-9]+]]
; CHECK: ![[MD_TEST01]] = !{%struct.test01 zeroinitializer, i32 1}

define void @test() {
  %x = load %struct.test01*, %struct.test01** @gVar1
  ret void
}

!1 = !{i64 0, i32 0}  ; i64
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!4 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i64, i64 }
!5 = !{!"S", %struct.test02 zeroinitializer, i32 2, !1, !1} ; { i64, i64 }

!intel.dtrans.types = !{!4, !5}

; end INTEL_FEATURE_SW_DTRANS
