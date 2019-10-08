; RUN: opt < %s  -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; Check the results of indirect array multiple value analysis.
; At this point, the analysis will always be marked as incomplete, as we
; are recognizing only a limited number of ways the indirect arrays can
; be assigned. This analysis is only needed for heuristics at this point.
; Test assignment of multiple values to elements of a field which points to
; an allocated array.

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.MYSTRUCTARRAY = type { i32, i32, i32*, i32* }
; CHECK: Name: struct.MYSTRUCTARRAY
; CHECK: Number of fields: 4
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: Frequency: 1
; CHECK: Multiple Value: [ 0, 1 ] <complete>
; CHECK: Multiple IA Value: [  ] <incomplete>
; CHECK: Bottom Alloc Function
; CHECK: 1)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: Frequency: 1
; CHECK: Multiple Value: [ 0, 2 ] <complete>
; CHECK: Multiple IA Value: [  ] <incomplete>
; CHECK: Bottom Alloc Function
; CHECK: 2)Field LLVM Type: i32*
; CHECK: Field info: Written
; CHECK: Frequency: 2
; CHECK: Multiple Value: [ null ] <incomplete>
; CHECK: Multiple IA Value: [ 1, 3 ] <incomplete>
; CHECK: Bottom Alloc Function
; CHECK: 3)Field LLVM Type: i32*
; CHECK: Field info: Written
; CHECK: Frequency: 2
; CHECK: Multiple Value: [ null ] <incomplete>
; CHECK: Multiple IA Value: [ 5, 7 ] <incomplete>
; CHECK: Bottom Alloc Function
; CHECK: Total Frequency: 6

%struct.MYSTRUCTARRAY = type { i32, i32, i32*, i32* }

@george = internal global %struct.MYSTRUCTARRAY zeroinitializer, align 8
@fred = internal global %struct.MYSTRUCTARRAY zeroinitializer, align 8

declare dso_local noalias i8* @malloc(i64) local_unnamed_addr #1

define dso_local i32 @main() {
  store i32 1, i32* getelementptr inbounds (%struct.MYSTRUCTARRAY, %struct.MYSTRUCTARRAY* @george, i64 0, i32 0), align 8
  store i32 2, i32* getelementptr inbounds (%struct.MYSTRUCTARRAY, %struct.MYSTRUCTARRAY* @george, i64 0, i32 1), align 4
  %1 = tail call noalias dereferenceable_or_null(80) i8* @malloc(i64 80) #2
  %2 = bitcast i8* %1 to i32*
  store i32* %2, i32** getelementptr inbounds (%struct.MYSTRUCTARRAY, %struct.MYSTRUCTARRAY* @george, i64 0, i32 2), align 8
  %3 = tail call noalias dereferenceable_or_null(80) i8* @malloc(i64 80) #2
  %4 = bitcast i8* %3 to i32*
  store i32* %4, i32** getelementptr inbounds (%struct.MYSTRUCTARRAY, %struct.MYSTRUCTARRAY* @george, i64 0, i32 3), align 8
  %5 = tail call noalias dereferenceable_or_null(160) i8* @malloc(i64 160) #2
  %6 = bitcast i8* %5 to i32*
  store i32* %6, i32** getelementptr inbounds (%struct.MYSTRUCTARRAY, %struct.MYSTRUCTARRAY* @fred, i64 0, i32 2), align 8
  %7 = tail call noalias dereferenceable_or_null(160) i8* @malloc(i64 160) #2
  %8 = bitcast i8* %7 to i32*
  store i32* %8, i32** getelementptr inbounds (%struct.MYSTRUCTARRAY, %struct.MYSTRUCTARRAY* @fred, i64 0, i32 3), align 8
  store i32 1, i32* %2, align 4
  store i32 5, i32* %4, align 4
  %9 = getelementptr inbounds i32, i32* %2, i64 2
  store i32 1, i32* %9, align 4
  %10 = getelementptr inbounds i32, i32* %4, i64 2
  store i32 5, i32* %10, align 4
  %11 = getelementptr inbounds i32, i32* %6, i64 1
  store i32 3, i32* %11, align 4
  %12 = getelementptr inbounds i32, i32* %8, i64 1
  store i32 7, i32* %12, align 4
  %13 = getelementptr inbounds i32, i32* %6, i64 3
  store i32 3, i32* %13, align 4
  %14 = getelementptr inbounds i32, i32* %8, i64 3
  store i32 7, i32* %14, align 4
  ret i32 8
}


