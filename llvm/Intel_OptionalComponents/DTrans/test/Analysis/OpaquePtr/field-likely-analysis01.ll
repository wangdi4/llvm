; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; Verify results get copied to the DTrans immutable analysis.

; RUN: opt < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-immutable-types -disable-output 2>&1 | FileCheck %s --check-prefix=IMMUTABLE
; RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-immutable-types -disable-output 2>&1 | FileCheck %s --check-prefix=IMMUTABLE

; Check that field value collection occurs, and the results of likely values are
; transferred to the DTrans immutable analysis results. This case collects
; multiple values for the scalar field, and multiple values for the memory
; referenced through the pointer field.

; Note: At this point, the "IA Value" (indirect array value) information will
; always be marked as incomplete, as we are recognizing only a limited number of
; ways the indirect arrays can be assigned. This analysis is only needed for
; heuristics at this point.

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

%struct.MYSTRUCTARRAY = type { i32, i32, i32*, i32* }

@george = internal global %struct.MYSTRUCTARRAY zeroinitializer, align 8
@fred = internal global %struct.MYSTRUCTARRAY zeroinitializer, align 8

define i32 @main() {
 ; Set field values for the structures.
  store i32 1, i32* getelementptr inbounds (%struct.MYSTRUCTARRAY, %struct.MYSTRUCTARRAY* @george, i64 0, i32 0), align 8
  store i32 2, i32* getelementptr inbounds (%struct.MYSTRUCTARRAY, %struct.MYSTRUCTARRAY* @george, i64 0, i32 1), align 4
  %alloc1 = tail call i8* @malloc(i64 80)
  %alloc1.ptr = bitcast i8* %alloc1 to i32*
  store i32* %alloc1.ptr, i32** getelementptr inbounds (%struct.MYSTRUCTARRAY, %struct.MYSTRUCTARRAY* @george, i64 0, i32 2), align 8
  %alloc2 = tail call i8* @malloc(i64 80)
  %alloc2.ptr = bitcast i8* %alloc2 to i32*
  store i32* %alloc2.ptr, i32** getelementptr inbounds (%struct.MYSTRUCTARRAY, %struct.MYSTRUCTARRAY* @george, i64 0, i32 3), align 8
  %alloc3 = tail call i8* @malloc(i64 160)
  %alloc3.ptr = bitcast i8* %alloc3 to i32*
  store i32* %alloc3.ptr, i32** getelementptr inbounds (%struct.MYSTRUCTARRAY, %struct.MYSTRUCTARRAY* @fred, i64 0, i32 2), align 8
  %alloc4 = tail call i8* @malloc(i64 160)
  %alloc4.ptr = bitcast i8* %alloc4 to i32*
  store i32* %alloc4.ptr, i32** getelementptr inbounds (%struct.MYSTRUCTARRAY, %struct.MYSTRUCTARRAY* @fred, i64 0, i32 3), align 8

  ; Set values for the allocated arrays of the structures.
  store i32 1, i32* %alloc1.ptr, align 4
  store i32 5, i32* %alloc2.ptr, align 4
  %ar1.2 = getelementptr inbounds i32, i32* %alloc1.ptr, i64 2
  store i32 1, i32* %ar1.2, align 4
  %ar2.2 = getelementptr inbounds i32, i32* %alloc2.ptr, i64 2
  store i32 5, i32* %ar2.2, align 4
  %ar3.1 = getelementptr inbounds i32, i32* %alloc3.ptr, i64 1
  store i32 3, i32* %ar3.1, align 4
  %ar4.1 = getelementptr inbounds i32, i32* %alloc4.ptr, i64 1
  store i32 7, i32* %ar4.1, align 4
  %ar3.3 = getelementptr inbounds i32, i32* %alloc3.ptr, i64 3
  store i32 3, i32* %ar3.3, align 4
  %ar4.3 = getelementptr inbounds i32, i32* %alloc4.ptr, i64 3
  store i32 7, i32* %ar4.3, align 4
  ret i32 8
}

declare !intel.dtrans.func.type !4 "intel_dtrans_func_index"="1" i8* @malloc(i64)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i32 0, i32 1}  ; i32*
!3 = !{i8 0, i32 1}  ; i8*
!4 = distinct !{!3}
!5 = !{!"S", %struct.MYSTRUCTARRAY zeroinitializer, i32 4, !1, !1, !2, !2} ; { i32, i32, i32*, i32* }

!intel.dtrans.types = !{!5}
