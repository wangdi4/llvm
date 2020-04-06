; REQUIRES: asserts

; RUN: opt -disable-output -whole-program-assume -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-CUR
; RUN: opt -disable-output -whole-program-assume -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-CUR

; Test pointer type recovery for realloc library calls based on
; inference from uses of the value created by the call instruction.

; Lines marked with CHECK-CUR are tests for the current form of IR.
; Lines marked with CHECK-FUT are placeholders for check lines that will
;   changed when the future opaque pointer form of IR is used.
; Lines marked with CHECK should remain the same when changing to use opaque
;   pointers.


; This case should detect the realloc result gets used as a
; %struct.test01*.
%struct.test01 = type { i64, i64 }
define internal %struct.test01* @test01(%struct.test01* %in) !dtrans_type !1 {
  %in_i8 = bitcast %struct.test01* %in to i8*
  %out_i8 = call i8* @realloc(i8* %in_i8, i64 256)
  %out = bitcast i8* %out_i8 to %struct.test01*
  ret %struct.test01* %out
}
; CHECK-CUR-LABEL: define internal %struct.test01* @test01
; CHECK-FUT-LABEL: define internal p0 @test01
; CHECK-CUR:   %out_i8 = call i8* @realloc(i8* %in_i8, i64 256)
; CHECK-FUT:   %out_i8 = call p0 @realloc(p0 %in_i8, i64 256)
; CHECK:    LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK-NEXT:    %struct.test01*{{ *$}}
; CHECK-NEXT:   i8*{{ *$}}
; CHECK-NEXT: No element pointees.


; In this case, the result of the realloc gets used as a different type
; than the original pointer type into the realloc. The pointer type
; analyzer only collects the type that the pointer is directly used as.
; This means the original input type will not be tracked as a result type
; for the realloc call. The safety analyzer should handle tracking whether the
; types are compatible.
%struct.test02 = type { %struct.test02*, %struct.test02b* }
%struct.test02b = type { %struct.test02b* }
@testvar02 = global %struct.test02 zeroinitializer
define internal void @test02() {
  %f0 = getelementptr %struct.test02, %struct.test02* @testvar02, i64 0, i32 0
  %v0 = load %struct.test02*, %struct.test02** %f0
  %v0_i8 = bitcast %struct.test02* %v0 to i8*
  %more = call i8* @realloc(i8* %v0_i8, i64 512)
  %more_i8 = bitcast i8* %more to %struct.test02b*
  %f1 = getelementptr %struct.test02, %struct.test02* @testvar02, i64 0, i32 1
  store %struct.test02b* %more_i8, %struct.test02b** %f1
  ret void
}
; CHECK-LABEL: define internal void @test02
; CHECK-CUR:  %more = call i8* @realloc(i8* %v0_i8, i64 512)
; CHECK-FUT:  %more = call p0 @realloc(p0 %v0_i8, i64 512)
; CHECK:    LocalPointerInfo:
; CHECK:      Aliased types:
; CHECK-NEXT:   %struct.test02b*{{ *$}}
; CHECK-NEXT:   i8*{{ *$}}
; CHECK-NEXT: No element pointees.

declare i8* @realloc(i8*, i64)

!1 = !{!"F", i1 false, i32 1, !2, !2}  ; %struct.test01* (%struct.test01*)
!2 = !{!3, i32 1}  ; %struct.test01*
!3 = !{!"R", %struct.test01 zeroinitializer, i32 0}  ; %struct.test01
!4 = !{i64 0, i32 0}  ; i64
!5 = !{!6, i32 1}  ; %struct.test02*
!6 = !{!"R", %struct.test02 zeroinitializer, i32 0}  ; %struct.test02
!7 = !{!8, i32 1}  ; %struct.test02b*
!8 = !{!"R", %struct.test02b zeroinitializer, i32 0}  ; %struct.test02b
!9 = !{!"S", %struct.test01 zeroinitializer, i32 2, !4, !4} ; { i64, i64 }
!10 = !{!"S", %struct.test02 zeroinitializer, i32 2, !5, !7} ; { %struct.test02*, %struct.test02b* }
!11 = !{!"S", %struct.test02b zeroinitializer, i32 1, !7} ; { %struct.test02b* }

!dtrans_types = !{!9, !10, !11}
