; RUN: opt < %s -whole-program-assume  -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; This test verifies correct handling of load and store instructions by the
; DTransAnalysis and verifies that real legality checks are correctly
; identified while patterns which do not present any legality concerns are not
; incorrectly marked as unsafe.

; Store of pointer to struct cast as an pointer-sized integer.
%struct.test01.a = type { i32, i32 }
%struct.test01.b = type { i32, i32, %struct.test01.a* }
define void @test1( %struct.test01.a* %sa, %struct.test01.b* %sb ) {
  %a.as.i = ptrtoint %struct.test01.a* %sa to i64
  %pb.a = getelementptr %struct.test01.b, %struct.test01.b* %sb, i64 0, i32 2
  %pb.a.as.pi = bitcast %struct.test01.a** %pb.a to i64*
  store i64 %a.as.i, i64* %pb.a.as.pi
  ret void
}

; CHECK: LLVMType: %struct.test01.a = type { i32, i32 }
; CHECK: Safety data: No issues found
; CHECK: LLVMType: %struct.test01.b = type { i32, i32, %struct.test01.a* }
; CHECK: Safety data: No issues found

; Mismatched store of pointer to struct cast as an pointer-sized integer.
%struct.test02.a = type { i32, i32 }
%struct.test02.b = type { i32, i32, %struct.test02.a* }
define void @test2( %struct.test02.b* %sb ) {
  %b.as.i = ptrtoint %struct.test02.b* %sb to i64
  %pb.a = getelementptr %struct.test02.b, %struct.test02.b* %sb, i64 0, i32 2
  %pb.a.as.pi = bitcast %struct.test02.a** %pb.a to i64*
  store i64 %b.as.i, i64* %pb.a.as.pi
  ret void
}

; CHECK: LLVMType: %struct.test02.a = type { i32, i32 }
; CHECK: Safety data: Unsafe pointer store
; CHECK: LLVMType: %struct.test02.b = type { i32, i32, %struct.test02.a* }
; CHECK: Safety data: Unsafe pointer store

; Copy of pointer to struct from one aggregate to another via GEP, bitcast and
; load/store.
%struct.test03.a = type { i32, %struct.test03.a* }
%struct.test03.b = type { i32, i32, %struct.test03.a* }
define void @test3( %struct.test03.a* %sa, %struct.test03.b* %sb ) {
  %ppaa = getelementptr %struct.test03.a, %struct.test03.a* %sa, i64 0, i32 1
  %ppaa.as.pi = bitcast %struct.test03.a** %ppaa to i64*
  %paa.as.i = load i64, i64* %ppaa.as.pi
  %ppba = getelementptr %struct.test03.b, %struct.test03.b* %sb, i64 0, i32 2
  %ppba.as.pi = bitcast %struct.test03.a** %ppba to i64*
  store i64 %paa.as.i, i64* %ppba.as.pi
  ret void
}

; CHECK: LLVMType: %struct.test03.a = type { i32, %struct.test03.a* }
; CHECK: Safety data: No issues found
; CHECK: LLVMType: %struct.test03.b = type { i32, i32, %struct.test03.a* }
; CHECK: Safety data: No issues found

; Copy of mismatched pointer to struct from one aggregate to another via GEP,
; bitcast and load/store.
%struct.test04.a = type { i32, %struct.test04.b* }
%struct.test04.b = type { i32, i32, %struct.test04.a* }
define void @test4( %struct.test04.a* %sa, %struct.test04.b* %sb ) {
  %ppab = getelementptr %struct.test04.a, %struct.test04.a* %sa, i64 0, i32 1
  %ppab.as.pi = bitcast %struct.test04.b** %ppab to i64*
  %pab.as.i = load i64, i64* %ppab.as.pi
  %ppba = getelementptr %struct.test04.b, %struct.test04.b* %sb, i64 0, i32 2
  %ppba.as.pi = bitcast %struct.test04.a** %ppba to i64*
  store i64 %pab.as.i, i64* %ppba.as.pi
  ret void
}

; CHECK: LLVMType: %struct.test04.a = type { i32, %struct.test04.b* }
; CHECK: Safety data: Unsafe pointer store
; CHECK: LLVMType: %struct.test04.b = type { i32, i32, %struct.test04.a* }
; CHECK: Safety data: Unsafe pointer store

; Cast of non-instruction to struct pointer.
; This use actually meets the pointer copy idiom and so is safe.
%struct.test05 = type { i32, %struct.test05* }
@p.test05 = internal unnamed_addr global %struct.test05 zeroinitializer
define void @test5(%struct.test05* %pa.arg) {
  %ppa.as.pi = bitcast %struct.test05** getelementptr(
                                          %struct.test05,
                                          %struct.test05* @p.test05,
                                          i64 0, i32 1) to i64*
  %pa.as.i = load i64, i64* %ppa.as.pi
  %ppa.arg = getelementptr %struct.test05, %struct.test05* %pa.arg, i64 0, i32 1
  %ppa.arg.as.pi = bitcast %struct.test05** %ppa.arg to i64*
  store i64 %pa.as.i, i64* %ppa.arg.as.pi
  ret void
}

; CHECK: LLVMType: %struct.test05 = type { i32, %struct.test05* }
; CHECK: Safety data: Global instance

; Load from aliased pointer passed across PHI and Select
%struct.test06 = type { i32, i32 }
define void @test6(%struct.test06* %p1, i32* %p2) {
entry:
  %t1 = bitcast %struct.test06* %p1 to i8*
  br label %loop

loop:
  %t2 = phi i8* [%t1, %entry], [%t4, %back]
  br i1 undef, label %end, label %back

back:
  %t3 = bitcast i32* %p2 to i8*
  %t4 = select i1 undef, i8* %t2, i8* %t3
  br label %loop

end:
  %t5 = bitcast i8* %t2 to i32*
  %t6 = load i32, i32* %t5
  ret void
}

; Note: Although the load can be proven safe in this case, the %t2 phi node
;       creates an unsafe pointer because we cannot reasonably prove that
;       the resulting value won't be used as if it were a pointer to
;       a %struct.test06 object, while it may refer to the %p2 argument.
; CHECK: LLVMType: %struct.test06 = type { i32, i32 }
; CHECK: Safety data: Unsafe pointer merge

; Load from a GEP-based pointer with the correct type
%struct.test07 = type { i32, i32 }
define void @test7(%struct.test07* %p) {
  %py = getelementptr %struct.test07, %struct.test07* %p, i64 0, i32 1
  %y = load i32, i32* %py
  ret void
}

; CHECK: LLVMType: %struct.test07 = type { i32, i32 }
; CHECK: Safety data: No issues found

; Load from a GEP-based pointer with the incorrect type
%struct.test08 = type { i32, i32 }
define void @test8(%struct.test08* %p) {
  %py = getelementptr %struct.test08, %struct.test08* %p, i64 0, i32 1
  %p2 = bitcast i32* %py to i16*
  %y2 = load i16, i16* %p2
  ret void
}

; CHECK: LLVMType: %struct.test08 = type { i32, i32 }
; CHECK: Safety data: Bad casting | Mismatched element access

; Load from a byte-flattened GEP-based pointer with the correct type
%struct.test09 = type { i32, i32 }
define void @test9(%struct.test09* %p) {
  %p2 = bitcast %struct.test09* %p to i8*
  %py8 = getelementptr i8, i8* %p2, i64 4
  %py = bitcast i8* %py8 to i32*
  %y = load i32, i32* %py
  ret void
}

; CHECK: LLVMType: %struct.test09 = type { i32, i32 }
; CHECK: Safety data: No issues found

; Load from a byte-flattened GEP-based pointer with the incorrect type
%struct.test10 = type { i32, i32 }
define void @test10(%struct.test10* %p) {
  %p2 = bitcast %struct.test10* %p to i8*
  %py8 = getelementptr i8, i8* %p2, i64 4
  %y = load i8, i8* %py8
  ret void
}

; CHECK: LLVMType: %struct.test10 = type { i32, i32 }
; CHECK: Safety data: Mismatched element access

; Load element zero from a pointer that aliases as a pointer to an aggregate
; type.
%struct.test11 = type { i8, i32 }
define void @test11(%struct.test11* %p) {
  %p2 = bitcast %struct.test11* %p to i8*
  %x = load i8, i8* %p2
  ret void
}

; CHECK: LLVMType: %struct.test11 = type { i8, i32 }
; CHECK: Safety data: No issues found

; Load something other than element zero from a pointer that aliases as
; a pointer to an aggregate type.
%struct.test12 = type { i32, i32 }
define void @test12(%struct.test12* %p) {
  %p2 = bitcast %struct.test12* %p to i8*
  %x = load i8, i8* %p2
  ret void
}

; CHECK: LLVMType: %struct.test12 = type { i32, i32 }
; CHECK: Safety data: Mismatched element access

; Load from an ambiguous pointer.
%struct.test13.a = type { i32, i32 }
%struct.test13.b = type { i64, i64 }
define void @test13(%struct.test13.a** %p1, %struct.test13.b** %p2) {
  %p1n = bitcast %struct.test13.a** %p1 to i64*
  %p2n = bitcast %struct.test13.b** %p2 to i64*
  %p3n = select i1 undef, i64* %p1n, i64* %p2n
  %p = load i64, i64* %p3n
  ret void
}

; CHECK: LLVMType: %struct.test13.a = type { i32, i32 }
; CHECK: Safety data: Unsafe pointer merge
; CHECK: LLVMType: %struct.test13.b = type { i64, i64 }
; CHECK: Safety data: Unsafe pointer merge

; Store of non-aggregate to an aggregate pointer location.
%struct.test14 = type { i32, i32 }
define void @test14(%struct.test14** %ppS, i8* %pUnknown) {
  %tmp = bitcast %struct.test14** %ppS to i64*
  %unknown = ptrtoint i8* %pUnknown to i64
  store i64 %unknown, i64* %tmp
  ret void
}

; CHECK: LLVMType: %struct.test14 = type { i32, i32 }
; CHECK: Safety data: Unsafe pointer store

; Store of aggregate to unknown location.
%struct.test15 = type { i32, i32 }
define void @test15(%struct.test15* %pS, i64* %pUnknown) {
  %tmp = ptrtoint %struct.test15* %pS to i64
  store i64 %tmp, i64* %pUnknown
  ret void
}

; CHECK: LLVMType: %struct.test15 = type { i32, i32 }
; CHECK: Safety data: Address taken

; Store of an element within an aggregate.
%struct.test16 = type { i32, i32 }
define void @test16(%struct.test16* %pS, i32 %val) {
  %pb = getelementptr %struct.test16, %struct.test16* %pS, i64 0, i32 1
  store i32 %val, i32* %pb
  ret void
}

; CHECK: LLVMType: %struct.test16 = type { i32, i32 }
; CHECK: Safety data: No issues found

; Mismatched store of an element within an aggregate.
%struct.test17 = type { i32, i32 }
define void @test17(%struct.test17* %pS, i8 %val) {
  %pb = getelementptr %struct.test17, %struct.test17* %pS, i64 0, i32 1
  %tmp = bitcast i32* %pb to i8*
  store i8 %val, i8* %tmp
  ret void
}

; CHECK: LLVMType: %struct.test17 = type { i32, i32 }
; CHECK: Safety data: Mismatched element access

; Store a pointer to a field in arbitrary memory.
%struct.test18 = type { i32, i32 }
define void @test18(%struct.test18* %pS, i64* %pUnknown) {
  %pb = getelementptr %struct.test18, %struct.test18* %pS, i64 0, i32 1
  %tmp = ptrtoint i32* %pb to i64
  store i64 %tmp, i64* %pUnknown
  ret void
}

; CHECK: LLVMType: %struct.test18 = type { i32, i32 }
; CHECK: Safety data: Field address taken

; Load of nested element zero.
%struct.test19.a = type { i32, i32 }
%struct.test19.b = type { %struct.test19.a, i32 }
define void @test19(%struct.test19.b* %p) {
  %pNested = bitcast %struct.test19.b* %p to i32*
  %t = load i32, i32* %pNested
  ret void
}

; CHECK: LLVMType: %struct.test19.a = type { i32, i32 }
; CHECK: Safety data: Nested structure
; CHECK: LLVMType: %struct.test19.b = type { %struct.test19.a, i32 }
; CHECK: Safety data: Contains nested structure

; Load of nested array element zero.
%struct.test20 = type { [16 x i32], i32 }
define void @test20(%struct.test20* %p) {
  %pNested = bitcast %struct.test20* %p to i32*
  %t = load i32, i32* %pNested
  ret void
}

; CHECK: LLVMType: %struct.test20 = type { [16 x i32], i32 }
; CHECK: Safety data: No issues found

; Load of small structure as integer (unsafe for dtrans).
%struct.test21 = type { i32, i32 }
define void @test21(%struct.test21* %p) {
  %pWhole = bitcast %struct.test21* %p to i64*
  %t = load i64, i64* %pWhole
  ret void
}

; CHECK: LLVMType: %struct.test21 = type { i32, i32 }
; CHECK: Safety data: Bad casting | Mismatched element access

; Direct load of a structure as a structure.
%struct.test22 = type { i32, i32 }
define void @test22(%struct.test22* %p) {
  %t = load %struct.test22, %struct.test22* %p
  ret void
}

; CHECK: LLVMType: %struct.test22 = type { i32, i32 }
; CHECK: Safety data: Whole structure reference

; Direct store of a structure as a structure.
%struct.test23 = type { i32, i32 }
define void @test23(%struct.test23 %s, %struct.test23* %p) {
  store %struct.test23 %s, %struct.test23* %p
  ret void
}

; CHECK: LLVMType: %struct.test23 = type { i32, i32 }
; CHECK: Safety data: Whole structure reference

; Load an entire structure through a nested element access.
%struct.test24.a = type { i32, i32 }
%struct.test24.b = type { %struct.test24.a, i32 }
define void @test24(%struct.test24.b* %pb) {
  %pa = bitcast %struct.test24.b* %pb to %struct.test24.a*
  %t = load %struct.test24.a, %struct.test24.a* %pa
  ret void
}

; CHECK: LLVMType: %struct.test24.a = type { i32, i32 }
; CHECK: Safety data: Whole structure reference | Nested structure
; CHECK: LLVMType: %struct.test24.b = type { %struct.test24.a, i32 }
; CHECK: Safety data: Contains nested structure

; Store an allocated pointer directly to memory.
%struct.test25 = type { i32, i32 }
define void @test25(%struct.test25** %pIn) {
  %dest = bitcast %struct.test25** %pIn to i8**
  %p = call i8* @malloc(i64 8)
  store i8* %p, i8** %dest
  ret void
}

; CHECK: LLVMType: %struct.test25 = type { i32, i32 }
; CHECK: Safety data: No issues found

; Store an allocated pointer-to-pointer directly to memory.
%struct.test26 = type { i32, i32 }
define void @test26(%struct.test26*** %pIn) {
  %dest = bitcast %struct.test26*** %pIn to i8**
  %p = call i8* @malloc(i64 8)
  store i8* %p, i8** %dest
  ret void
}

; CHECK: LLVMType: %struct.test26 = type { i32, i32 }
; CHECK: Safety data: No issues found

; Store a pointer to struct to an array of pointers to that type of struct.
%struct.test27 = type { i32, [50 x %struct.test27*] }
define void @test27( %struct.test27* %p) {
  %p8 = bitcast %struct.test27* %p to i8*
  %pArr = getelementptr %struct.test27, %struct.test27* %p, i64 0, i32 1
  %pp = bitcast [50 x %struct.test27*]* %pArr to i8**
  store i8* %p8, i8** %pp
  ret void
}

; CHECK: LLVMType: %struct.test27 = type { i32, [50 x %struct.test27*] }
; CHECK: Safety data: No issues found
; This is here to make sure the test above finds the right safety data.
; CHECK-LABEL: DTRANS_ArrayInfo

declare noalias i8* @malloc(i64)
