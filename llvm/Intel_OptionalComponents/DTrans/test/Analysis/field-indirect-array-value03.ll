; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s  -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; Verify results copied to dtrans immutable analysis.
; RUN: opt < %s -whole-program-assume -dtransanalysis -dtrans-print-immutable-types -disable-output 2>&1 | FileCheck %s --check-prefix=IMMUTABLE
; RUN: opt < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-immutable-types -disable-output 2>&1 | FileCheck %s --check-prefix=IMMUTABLE

; Check the results of indirect array multiple value analysis.
; At this point, the analysis will always be marked as incomplete, as we
; are recognizing only a limited number of ways the indirect arrays can
; be assigned. This analysis is only needed for heuristics at this point.
; Test assignment of multiple values to elements of a field which points to
; an allocated array. Tolerate conversion of values returned from @malloc
; to integers and then back to pointers.

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.MYSTRUCTARRAY = type { i32, i32, i32*, i32* }
; CHECK: Name: struct.MYSTRUCTARRAY
; CHECK: Number of fields: 4
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info:
; CHECK: Frequency: 0
; CHECK: Single Value: i32 0
; CHECK: Multiple IA Value: [  ] <incomplete>
; CHECK: Bottom Alloc Function
; CHECK: 1)Field LLVM Type: i32
; CHECK: Field info:
; CHECK: Frequency: 0
; CHECK: Single Value: i32 0
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
; CHECK: Total Frequency: 4


; IMMUTABLE: StructType: %struct.MYSTRUCTARRAY = type { i32, i32, i32*, i32* }
; IMMUTABLE:   Field 0:
; IMMUTABLE:     Likely Values: 0
; IMMUTABLE:     Likely Indirect Array Values:
; IMMUTABLE:   Field 1:
; IMMUTABLE:     Likely Values: 0
; IMMUTABLE:     Likely Indirect Array Values:
; IMMUTABLE:   Field 2:
; IMMUTABLE:     Likely Values: null
; IMMUTABLE:     Likely Indirect Array Values: 1 3
; IMMUTABLE:   Field 3:
; IMMUTABLE:     Likely Values: null
; IMMUTABLE:     Likely Indirect Array Values: 5 7


%struct.MYSTRUCTARRAY = type { i32, i32, i32*, i32* }

@george = internal global %struct.MYSTRUCTARRAY zeroinitializer, align 8
@fred = internal global %struct.MYSTRUCTARRAY zeroinitializer, align 8

declare dso_local noalias i8* @malloc(i64) local_unnamed_addr #1

define dso_local i32 @main() {
  %1 = tail call noalias dereferenceable_or_null(80) i8* @malloc(i64 80) #2
  %t10 = ptrtoint i8* %1 to i64
  %t11 = inttoptr i64 %t10 to i8*
  %2 = bitcast i8* %t11 to i32*
  %t20 = getelementptr inbounds %struct.MYSTRUCTARRAY, %struct.MYSTRUCTARRAY* @george, i64 0, i32 2
  %t21 = bitcast i32** %t20 to i8**
  store i8* %t11, i8** %t21, align 8
  %3 = tail call noalias dereferenceable_or_null(80) i8* @malloc(i64 80) #2
  %t30 = ptrtoint i8* %3 to i64
  %t31 = inttoptr i64 %t30 to i8*
  %4 = bitcast i8* %t31 to i32*
  %t40 = getelementptr inbounds %struct.MYSTRUCTARRAY, %struct.MYSTRUCTARRAY* @george, i64 0, i32 3
  %t41 = bitcast i32** %t40 to i8**
  store i8* %t31, i8** %t41, align 8
  %5 = tail call noalias dereferenceable_or_null(160) i8* @malloc(i64 160) #2
  %t50 = ptrtoint i8* %3 to i64
  %t51 = inttoptr i64 %t50 to i8*
  %6 = bitcast i8* %t51 to i32*
  %t60 = getelementptr inbounds %struct.MYSTRUCTARRAY, %struct.MYSTRUCTARRAY* @fred, i64 0, i32 2
  %t61 = bitcast i32** %t60 to i8**
  store i8* %t51, i8** %t61, align 8
  %7 = tail call noalias dereferenceable_or_null(160) i8* @malloc(i64 160) #2
  %t70 = ptrtoint i8* %7 to i64
  %t71 = inttoptr i64 %t70 to i8*
  %8 = bitcast i8* %t71 to i32*
  %t80 = getelementptr inbounds %struct.MYSTRUCTARRAY, %struct.MYSTRUCTARRAY* @fred, i64 0, i32 3
  %t81 = bitcast i32** %t80 to i8**
  store i8* %t71, i8** %t81, align 8
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


