; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; Verify results get copied to the DTrans immutable analysis.

; RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-immutable-types -disable-output 2>&1 | FileCheck %s --check-prefix=IMMUTABLE

; Check that field value collection occurs, and the results of likely values are
; transferred to the DTrans immutable analysis results. This case collects
; multiple values for the scalar field, and multiple values for the memory
; referenced through the pointer field.

; Note: At this point, the "IA Value" (indirect array value) information will
; always be marked as incomplete, as we are recognizing only a limited number of
; ways the indirect arrays can be assigned. This analysis is only needed for
; heuristics at this point.

; This test is like field-likely-analysis01.ll, but only uses opaque pointers.

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.MYSTRUCTARRAY
; CHECK: Name: struct.MYSTRUCTARRAY
; CHECK: Number of fields: 4
; CHECK: 0)
; CHECK: DTrans Type: i32
; CHECK: Multiple Value: [ 0, 1 ] <complete>
; CHECK: Multiple IA Value: [  ] <incomplete>
; CHECK: 1)
; CHECK: DTrans Type: i32
; CHECK: Multiple Value: [ 0, 2 ] <complete>
; CHECK: Multiple IA Value: [  ] <incomplete>
; CHECK: 2)
; CHECK: DTrans Type: i32*
; CHECK: Multiple Value: [ null ] <incomplete>
; CHECK: Multiple IA Value: [ 1, 3 ] <incomplete>
; CHECK: 3)
; CHECK: DTrans Type: i32*
; CHECK: Multiple Value: [ null ] <incomplete>
; CHECK: Multiple IA Value: [ 5, 7 ] <incomplete>
; CHECK: End LLVMType: %struct.MYSTRUCTARRAY

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.YOURSTRUCTARRAY
; CHECK: Name: struct.YOURSTRUCTARRAY
; CHECK: Number of fields: 4
; CHECK: 0)
; CHECK: DTrans Type: i32
; CHECK: Multiple Value: [ 0, 11 ] <complete>
; CHECK: Multiple IA Value: [  ] <incomplete>
; CHECK: 1)
; CHECK: DTrans Type: i32
; CHECK: Multiple Value: [ 0, 21 ] <complete>
; CHECK: Multiple IA Value: [  ] <incomplete>
; CHECK: 2)
; CHECK: DTrans Type: i32*
; CHECK: Multiple Value: [ null ] <incomplete>
; CHECK: Multiple IA Value: [ 101, 103 ] <incomplete>
; CHECK: 3)
; CHECK: DTrans Type: i32*
; CHECK: Multiple Value: [ null ] <incomplete>
; CHECK: Multiple IA Value: [ 105, 107 ] <incomplete>
; CHECK: End LLVMType: %struct.YOURSTRUCTARRAY

; IMMUTABLE: StructType: %struct.MYSTRUCTARRAY
; IMMUTABLE:   Field 0:
; IMMUTABLE:     Likely Values: 0 1{{ *}}
; IMMUTABLE:     Likely Indirect Array Values:{{ *}}
; IMMUTABLE:   Field 1:
; IMMUTABLE:     Likely Values: 0 2{{ *}}
; IMMUTABLE:     Likely Indirect Array Values:{{ *}}
; IMMUTABLE:   Field 2:
; IMMUTABLE:     Likely Values: null
; IMMUTABLE:     Likely Indirect Array Values: 1 3{{ *}}
; IMMUTABLE:   Field 3:
; IMMUTABLE:     Likely Values: null
; IMMUTABLE:     Likely Indirect Array Values: 5 7{{ *}}

; IMMUTABLE: StructType: %struct.YOURSTRUCTARRAY
; IMMUTABLE:   Field 0:
; IMMUTABLE:     Likely Values: 0 11{{ *}}
; IMMUTABLE:     Likely Indirect Array Values:{{ *}}
; IMMUTABLE:   Field 1:
; IMMUTABLE:     Likely Values: 0 21{{ *}}
; IMMUTABLE:     Likely Indirect Array Values:{{ *}}
; IMMUTABLE:   Field 2:
; IMMUTABLE:     Likely Values: null
; IMMUTABLE:     Likely Indirect Array Values: 101 103{{ *}}
; IMMUTABLE:   Field 3:
; IMMUTABLE:     Likely Values: null
; IMMUTABLE:     Likely Indirect Array Values: 105 107{{ *}}
%struct.YOURSTRUCTARRAY = type { i32, i32, ptr, ptr }
%struct.MYSTRUCTARRAY = type { i32, i32, ptr, ptr }

@ethel = internal global %struct.YOURSTRUCTARRAY zeroinitializer, align 8
@martha = internal global %struct.YOURSTRUCTARRAY zeroinitializer, align 8

@george = internal global %struct.MYSTRUCTARRAY zeroinitializer, align 8
@fred = internal global %struct.MYSTRUCTARRAY zeroinitializer, align 8

define void @test() {
 ; Set field values for the structures.
  store i32 11, ptr getelementptr inbounds (%struct.YOURSTRUCTARRAY, ptr @ethel, i64 0, i32 0), align 8
  store i32 21, ptr getelementptr inbounds (%struct.YOURSTRUCTARRAY, ptr @ethel, i64 0, i32 1), align 4
  %alloc1 = tail call ptr @malloc(i64 80)
  store ptr %alloc1, ptr getelementptr inbounds (%struct.YOURSTRUCTARRAY, ptr @ethel, i64 0, i32 2), align 8
  %alloc2 = tail call ptr @malloc(i64 80)
  store ptr %alloc2, ptr getelementptr inbounds (%struct.YOURSTRUCTARRAY, ptr @ethel, i64 0, i32 3), align 8
  %alloc3 = tail call ptr @malloc(i64 160)
  store ptr %alloc3, ptr getelementptr inbounds (%struct.YOURSTRUCTARRAY, ptr @martha, i64 0, i32 2), align 8
  %alloc4 = tail call ptr @malloc(i64 160)
  store ptr %alloc4, ptr getelementptr inbounds (%struct.YOURSTRUCTARRAY, ptr @martha, i64 0, i32 3), align 8

  ; Set values for the allocated arrays of the structures.
  store i32 101, ptr %alloc1, align 4
  store i32 105, ptr %alloc2, align 4
  %ar1.2 = getelementptr inbounds i32, ptr %alloc1, i64 2
  store i32 101, ptr %ar1.2, align 4
  %ar2.2 = getelementptr inbounds i32, ptr %alloc2, i64 2
  store i32 105, ptr %ar2.2, align 4
  %ar3.1 = getelementptr inbounds i32, ptr %alloc3, i64 1
  store i32 103, ptr %ar3.1, align 4
  %ar4.1 = getelementptr inbounds i32, ptr %alloc4, i64 1
  store i32 107, ptr %ar4.1, align 4
  %ar3.3 = getelementptr inbounds i32, ptr %alloc3, i64 3
  store i32 103, ptr %ar3.3, align 4
  %ar4.3 = getelementptr inbounds i32, ptr %alloc4, i64 3
  store i32 107, ptr %ar4.3, align 4
  ret void
}

define i32 @main() {
 ; Set field values for the structures.
  store i32 1, ptr getelementptr inbounds (%struct.MYSTRUCTARRAY, ptr @george, i64 0, i32 0), align 8
  store i32 2, ptr getelementptr inbounds (%struct.MYSTRUCTARRAY, ptr @george, i64 0, i32 1), align 4
  %alloc1 = tail call ptr @malloc(i64 80)
  store ptr %alloc1, ptr getelementptr inbounds (%struct.MYSTRUCTARRAY, ptr @george, i64 0, i32 2), align 8
  %alloc2 = tail call ptr @malloc(i64 80)
  store ptr %alloc2, ptr getelementptr inbounds (%struct.MYSTRUCTARRAY, ptr @george, i64 0, i32 3), align 8
  %alloc3 = tail call ptr @malloc(i64 160)
  store ptr %alloc3, ptr getelementptr inbounds (%struct.MYSTRUCTARRAY, ptr @fred, i64 0, i32 2), align 8
  %alloc4 = tail call ptr @malloc(i64 160)
  store ptr %alloc4, ptr getelementptr inbounds (%struct.MYSTRUCTARRAY, ptr @fred, i64 0, i32 3), align 8

  ; Set values for the allocated arrays of the structures.
  store i32 1, ptr %alloc1, align 4
  store i32 5, ptr %alloc2, align 4
  %ar1.2 = getelementptr inbounds i32, ptr %alloc1, i64 2
  store i32 1, ptr %ar1.2, align 4
  %ar2.2 = getelementptr inbounds i32, ptr %alloc2, i64 2
  store i32 5, ptr %ar2.2, align 4
  %ar3.1 = getelementptr inbounds i32, ptr %alloc3, i64 1
  store i32 3, ptr %ar3.1, align 4
  %ar4.1 = getelementptr inbounds i32, ptr %alloc4, i64 1
  store i32 7, ptr %ar4.1, align 4
  %ar3.3 = getelementptr inbounds i32, ptr %alloc3, i64 3
  store i32 3, ptr %ar3.3, align 4
  %ar4.3 = getelementptr inbounds i32, ptr %alloc4, i64 3
  store i32 7, ptr %ar4.3, align 4
  ret i32 8
}

declare !intel.dtrans.func.type !4 "intel_dtrans_func_index"="1" ptr @malloc(i64)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i32 0, i32 1}  ; i32*
!3 = !{i8 0, i32 1}  ; i8*
!4 = distinct !{!3}
!5 = !{!"S", %struct.MYSTRUCTARRAY zeroinitializer, i32 4, !1, !1, !2, !2} ; { i32, i32, i32*, i32* }
!6 = !{!"S", %struct.YOURSTRUCTARRAY zeroinitializer, i32 4, !1, !1, !2, !2} ; { i32, i32, i32*, i32* }

!intel.dtrans.types = !{!5, !6}
