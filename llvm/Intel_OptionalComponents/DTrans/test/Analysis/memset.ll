; RUN: opt < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s

target datalayout = "e-m:w-i64:64-f80:128-n8:16:32:64-S128"

; This test verifies correct identification and processing of the memset
; intrinsic by the DTransAnalysis and verifies that real legality checks
; are correctly identified while patterns which do not present any legality
; concerns are not incorrectly marked as unsafe.

; This test checks that the entire structure is filled by the memset.
; This is a safe use.
%struct.test01 = type { i32, i16, i8 }
define void @test01(%struct.test01* %a) {
  %p = bitcast %struct.test01* %a to i8*
  tail call void @llvm.memset.p0i8.i64(i8* %p, i8 0, i64 8, i1 false)
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test01 = type { i32, i16, i8 }
; CHECK: Safety data: No issues found


; This test checks when multiple of the structure size is used, such as
; for an array of structures.
; This is a safe use.
%struct.test02 = type { i32, i16, i8 }
define void @test02(%struct.test02* %a) {
  %p = bitcast %struct.test02* %a to i8*
  tail call void @llvm.memset.p0i8.i64(i8* %p, i8 0, i64 32, i1 false)
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test02 = type { i32, i16, i8 }
; CHECK: Safety data: No issues found


; This test checks when a multiple of the structure size is used, such as
; for an array of structures, but the size is not a compile time constant.
; This is a safe use.
%struct.test02b = type { i32, i16, i8 }
define void @test02b(%struct.test02b* %a, i32 %n) {
  %p = bitcast %struct.test02b* %a to i8*
  %conv = sext i32 %n to i64
  %mul = mul i64 %conv, 8
  tail call void @llvm.memset.p0i8.i64(i8* %p, i8 0, i64 %mul, i1 false)
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test02b = type { i32, i16, i8 }
; CHECK: Safety data: No issues found


; This test checks when a structure is composed of structures. Each of the
; the structure fields should be marked as written.
; This is a safe use.
%struct.test03.a = type { i32, i16, [2 x i32] }
%struct.test03.b = type { i32, i16, i8 }
%struct.test03.c = type { %struct.test03.a, %struct.test03.b }
define void @test03(%struct.test03.c* %c) {
  %c0 = bitcast %struct.test03.c* %c to i8*
  tail call void @llvm.memset.p0i8.i64(i8* %c0, i8 0, i64 24, i1 false)
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test03.a = type { i32, i16, [2 x i32] }
; CHECK: Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK-LABEL: LLVMType: %struct.test03.b = type { i32, i16, i8 }
; CHECK: Field LLVM Type: i16
; CHECK: Field info: Written
; CHECK-LABEL: LLVMType: %struct.test03.c = type { %struct.test03.a, %struct.test03.b }
; CHECK: Safety data: Contains nested structure


; This test checks when a structure is composed of pointers to structures.
; This is a safe use, and only members of struct.test04.c should be marked
; with 'Written'.
%struct.test04.a = type { i32, i16, [2 x i32] }
%struct.test04.b = type { i32, i16, i8 }
%struct.test04.c = type { %struct.test04.a*, %struct.test04.b* }
define void @test04(%struct.test04.c* %c) {
  %c0 = bitcast %struct.test04.c* %c to i8*
  tail call void @llvm.memset.p0i8.i64(i8* %c0, i8 0, i64 16, i1 false)
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test04.a = type { i32, i16, [2 x i32] }
; CHECK: Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK-LABEL: LLVMType: %struct.test04.b = type { i32, i16, i8 }
; CHECK: Field LLVM Type: i16
; CHECK: Field info:{{ *$}}
; CHECK-LABEL: LLVMType: %struct.test04.c = type { %struct.test04.a*, %struct.test04.b* }
; CHECK: Number of fields: 2
; CHECK: Field LLVM Type: %struct.test04.a*
; CHECK: Field info: Written
; CHECK: Field LLVM Type: %struct.test04.b*
; CHECK: Field info: Written
; CHECK: Safety data: No issues found


; This test checks when memset only clears part of the structure.
; This is a safe use, but should mark the 'Memfunc partial write' safety.
%struct.test05 = type { i32, i16, i16, i8 }
define void @unsafe1(%struct.test05* %b) {
  %p = bitcast %struct.test05* %b to i8*
  tail call void @llvm.memset.p0i8.i64(i8* %p, i8 0, i64 6, i1 false)
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test05 = type { i32, i16, i16, i8 }
; CHECK: Safety data: Memfunc partial write


; This test checks when memset clears the structure based on a summation
; of the field types, ignoring tail padding of the structure.
; This is a safe use.
%struct.test06 = type { i32, i16, i16, i8 }
define void @unsafe2(%struct.test06* %b) {
  %p = bitcast %struct.test06* %b to i8*
  tail call void @llvm.memset.p0i8.i64(i8* %p, i8 0, i64 9, i1 false)
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test06 = type { i32, i16, i16, i8 }
; CHECK: Safety data: No issues found


; This test checks a pattern that is produced in special cases by the FE that
; reduces the structure to an i8*.
%struct.test07 = type { [200 x i8], [200 x i8], i64 }
@test07var = internal unnamed_addr global %struct.test07 zeroinitializer, align 8
define void @test07(%struct.test07* %a) {
  %p = bitcast %struct.test07* %a to i8*
  tail call void @llvm.memset.p0i8.i64(i8* getelementptr inbounds (%struct.test07, %struct.test07* @test07var, i64 0, i32 0, i64 0), i8 0, i64 408, i1 false)
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test07 = type { [200 x i8], [200 x i8], i64 }
; CHECK: Safety data: Global instance


; This test checks using memset with a pointer-to-pointer parameter. In this
; case, only pointers to the struct are being set, so the elements should not
; be marked as being written.
%struct.test08.a = type { i32, i32, i32, i32 }
%struct.test08.b = type { i32, %struct.test08.a** }
define void @test08(%struct.test08.b* %b) {
  %buf = tail call noalias i8* @malloc(i64 128)
  %b_pp = getelementptr inbounds %struct.test08.b, %struct.test08.b* %b, i64 0, i32 1
  %addr_pp = bitcast %struct.test08.a*** %b_pp to i8**
  store i8* %buf, i8** %addr_pp, align 8
  tail call void @llvm.memset.p0i8.i64(i8* %buf, i8 0, i64 128, i1 false)
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test08.a = type { i32, i32, i32, i32 }
; CHECK: Number of fields: 4
; CHECK: Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: Safety data: No issues found


; This test checks using memset with a pointer to a structure that is a
; structure member field of a parent structure.
; This is a safe use.
%struct.test09.a = type { i32, i32, i32, i32, i32 }
%struct.test09.b = type { i32, %struct.test09.a }
define void @test09(%struct.test09.b* %b) {
  %a = getelementptr inbounds %struct.test09.b, %struct.test09.b* %b, i64 0, i32 1
  %a0 = bitcast %struct.test09.a* %a to i8*
  tail call void @llvm.memset.p0i8.i64(i8* %a0, i8 0, i64 20, i1 false)
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test09.a = type { i32, i32, i32, i32, i32 }
; CHECK: Safety data: Nested structure
; CHECK-LABEL: LLVMType: %struct.test09.b = type { i32, %struct.test09.a }
; CHECK: Safety data: Memfunc partial write | Contains nested structure


; This test checks using memset with the address of a pointer to struct member
; field within a nested stucture. In this case, just the pointer field being is
; being written.
; This is a safe use.
%struct.test10.a = type { i32, i32, i32, i32, i32 }
%struct.test10.b = type { i32, %struct.test10.a* }
define void @test10(%struct.test10.b* %b) {
  %a = getelementptr inbounds %struct.test10.b, %struct.test10.b* %b, i64 0, i32 1
  %a0 = bitcast %struct.test10.a** %a to i8*
  tail call void @llvm.memset.p0i8.i64(i8* %a0, i8 0, i64 8, i1 false)
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test10.a = type { i32, i32, i32, i32, i32 }
; CHECK: Safety data: No issues found
; CHECK-LABEL: LLVMType: %struct.test10.b = type { i32, %struct.test10.a* }
; CHECK: Safety data: Memfunc partial write


; This test checks using memset with a pointer to a structure that is a
; structure member field of a parent stucture, but the write covers the
; structure field and the following field. This should be detected as a partial
; write on the parent structure, and a write on the child structure.
%struct.test11.a = type { i32, i32, i32, i32, i32 }
%struct.test11.b = type { i32, %struct.test11.a, i32 }
define void @test11(%struct.test11.b* %b) {
  %a = getelementptr inbounds %struct.test11.b, %struct.test11.b* %b, i64 0, i32 1
  %a0 = bitcast %struct.test11.a* %a to i8*
  tail call void @llvm.memset.p0i8.i64(i8* %a0, i8 0, i64 24, i1 false)
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test11.a = type { i32, i32, i32, i32, i32 }
; CHECK: Number of fields: 5
; CHECK: Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: Safety data: Nested structure
; CHECK-LABEL: LLVMType: %struct.test11.b = type { i32, %struct.test11.a, i32 }
; CHECK: Number of fields: 3
; CHECK: Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: Field LLVM Type: %struct.test11.a = type { i32, i32, i32, i32, i32 }
; CHECK: Field info: Written
; CHECK: Field LLVM Type: i32
; CHECK: Field info: Written

; CHECK: Safety data: Memfunc partial write | Contains nested structure


; This test checks using memset with address of a pointer to struct member
; field within a nested stucture. In this case, it writes the field and
; the adjacent field.
; This should detect a partial write usage.
%struct.test12.a = type { i32, i32, i32, i32, i32 }
%struct.test12.b = type { i32, %struct.test12.a*, i64 }
define void @test12(%struct.test12.b* %b) {
  %a = getelementptr inbounds %struct.test12.b, %struct.test12.b* %b, i64 0, i32 1
  %a0 = bitcast %struct.test12.a** %a to i8*
  tail call void @llvm.memset.p0i8.i64(i8* %a0, i8 0, i64 16, i1 false)
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test12.a = type { i32, i32, i32, i32, i32 }
; CHECK: Safety data: No issues found
; CHECK-LABEL: LLVMType: %struct.test12.b = type { i32, %struct.test12.a*, i64 }
; CHECK: Safety data: Memfunc partial write


; This test checks using memset with a pointer to a structure that is an array
; of structures field of a parent structure. Sets all 10 elements of array of
; structure. The structure passed into the memset should be considered safe.
%struct.test13.b = type { i32, [10 x %struct.test13.a] }
%struct.test13.a = type { i32, i32, i32, i32, i32 }
define void @test13(%struct.test13.b* %b) {
  %a = getelementptr inbounds %struct.test13.b, %struct.test13.b* %b, i64 0, i32 1
  %t0 = bitcast [10 x %struct.test13.a]* %a to i8*
  tail call void @llvm.memset.p0i8.i64(i8* %t0, i8 0, i64 200, i1 false)
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test13.a = type { i32, i32, i32, i32, i32 }
; CHECK: Number of fields: 5
; CHECK: Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: Safety data: Nested structure
; CHECK-LABEL: LLVMType: %struct.test13.b = type { i32, [10 x %struct.test13.a] }
; CHECK: Number of fields: 2
; CHECK: Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: Field LLVM Type: [10 x %struct.test13.a]
; CHECK: Field info: Written
; CHECK: Safety data: Memfunc partial write | Contains nested structure


; This test checks using memset with a pointer to a structure that is an array
; of structures field of a parent structure. However, this version only sets 5
; elements of array of structure.
; The structure passed into the memset should be considered safe, but the
; structure that contains it should be marked with bad memfunc size because
; we currently require the entire aggregate to be set.
%struct.test14.b = type { i32, [10 x %struct.test14.a] }
%struct.test14.a = type { i32, i32, i32, i32, i32 }
define void @test14(%struct.test14.b* %b) {
  %a = getelementptr inbounds %struct.test14.b, %struct.test14.b* %b, i64 0, i32 1
  %t0 = bitcast [10 x %struct.test14.a]* %a to i8*
  tail call void @llvm.memset.p0i8.i64(i8* %t0, i8 0, i64 100, i1 false)
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test14.a = type { i32, i32, i32, i32, i32 }
; CHECK: Multiple Value
; CHECK: Multiple Value
; CHECK: Multiple Value
; CHECK: Multiple Value
; CHECK: Multiple Value
; CHECK: Safety data: Bad memfunc size | Nested structure
; CHECK-LABEL: LLVMType: %struct.test14.b = type { i32, [10 x %struct.test14.a] }
; CHECK: Safety data: Bad memfunc size | Contains nested structure


; This test checks using memset with the address of a scalar member of the
; structure.
; This is an unsafe use because we do not track type information for scalar
; pointers, and therefore are unable to resolve the pointer target.
%struct.test15.a = type { i32, i32, i32, i32, i32 }
define void @test15(%struct.test15.a* %a) {
  %c = getelementptr inbounds %struct.test15.a, %struct.test15.a* %a, i64 0, i32 2
  %t0 = bitcast i32* %c to i8*
  tail call void @llvm.memset.p0i8.i64(i8* %t0, i8 0, i64 12, i1 false)
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test15.a = type { i32, i32, i32, i32, i32 }
; CHECK: Safety data: Memfunc partial write


; This test checks using memset with a zero for the size parameter.
; This is a safe use, no matter what the pointer parameter points to.
%struct.test16.a = type { i32, i32, i32, i32, i32 }
define void @test16(%struct.test16.a* %a) {
  %c = getelementptr inbounds %struct.test16.a, %struct.test16.a* %a, i64 0, i32 2
  %t0 = bitcast i32* %c to i8*
  tail call void @llvm.memset.p0i8.i64(i8* %t0, i8 0, i64 0, i1 false)
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test16.a = type { i32, i32, i32, i32, i32 }
; CHECK: Safety data: No issues found

; This test checks using memset to write a subset of fields in the middle of a
; structure such as may have been created by the memcpy optimizer.
%struct.test17 = type { %struct.test17*, %struct.test17*, %struct.test17*, %struct.test17*, %struct.test17* }
define void @test17(%struct.test17* %a) {
  %a1 = getelementptr inbounds %struct.test17, %struct.test17* %a, i64 0, i32 1
  %1 = bitcast %struct.test17** %a1 to i8*
  call void @llvm.memset.p0i8.i64(i8* %1, i8 0, i64 24, i1 false)
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test17 = type { %struct.test17*, %struct.test17*, %struct.test17*, %struct.test17*, %struct.test17* }
; CHECK: Field LLVM Type: %struct.test17*
; CHECK: Field info:{{ *$}}
; CHECK: Field LLVM Type: %struct.test17*
; CHECK: Field info: Written
; CHECK: Field LLVM Type: %struct.test17*
; CHECK: Field info: Written
; CHECK: Field LLVM Type: %struct.test17*
; CHECK: Field info: Written
; CHECK: Field LLVM Type: %struct.test17*
; CHECK: Field info:{{ *$}}
; CHECK: Safety data: Memfunc partial write


; This test checks using memset to write a subset of fields ending with the
; last field of the structure.
%struct.test18 = type { %struct.test18*, %struct.test18*, %struct.test18*, %struct.test18*, %struct.test18* }
define void @test18(%struct.test18* %a) {
  %a1 = getelementptr inbounds %struct.test18, %struct.test18* %a, i64 0, i32 2
  %1 = bitcast %struct.test18** %a1 to i8*
  call void @llvm.memset.p0i8.i64(i8* %1, i8 0, i64 24, i1 false)
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test18 = type { %struct.test18*, %struct.test18*, %struct.test18*, %struct.test18*, %struct.test18* }
; CHECK: Field LLVM Type: %struct.test18*
; CHECK: Field info:{{ *$}}
; CHECK: Field LLVM Type: %struct.test18*
; CHECK: Field info:{{ *$}}
; CHECK: Field LLVM Type: %struct.test18*
; CHECK: Field info: Written
; CHECK: Field LLVM Type: %struct.test18*
; CHECK: Field info: Written
; CHECK: Field LLVM Type: %struct.test18*
; CHECK: Field info: Written
; CHECK: Safety data: Memfunc partial write


; This test checks using memset starting with a pointer to member that writes
; the entire structure
%struct.test19 = type { %struct.test19*, %struct.test19*, %struct.test19*, %struct.test19*, %struct.test19* }
define void @test19(%struct.test19* %a) {
  %a1 = getelementptr inbounds %struct.test19, %struct.test19* %a, i64 0, i32 0
  %1 = bitcast %struct.test19** %a1 to i8*
  call void @llvm.memset.p0i8.i64(i8* %1, i8 0, i64 40, i1 false)
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test19 = type { %struct.test19*, %struct.test19*, %struct.test19*, %struct.test19*, %struct.test19* }
; CHECK: Field LLVM Type: %struct.test19*
; CHECK: Field info: Written
; CHECK: Field LLVM Type: %struct.test19*
; CHECK: Field info: Written
; CHECK: Field LLVM Type: %struct.test19*
; CHECK: Field info: Written
; CHECK: Field LLVM Type: %struct.test19*
; CHECK: Field info: Written
; CHECK: Field LLVM Type: %struct.test19*
; CHECK: Field info: Written
; CHECK: Safety data: No issues found


; This test checks using memset to write a subset of fields, but not including
; the padding bytes after the last field.
%struct.test20 = type { i16, %struct.test20*, i16, %struct.test20* }
define void @test20(%struct.test20* %a) {
  %a1 = getelementptr inbounds %struct.test20, %struct.test20* %a, i64 0, i32 1
  %1 = bitcast %struct.test20** %a1 to i8*
  call void @llvm.memset.p0i8.i64(i8* %1, i8 0, i64 10, i1 false)
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test20 = type { i16, %struct.test20*, i16, %struct.test20* }
; CHECK: Field LLVM Type: i16
; CHECK: Field info:{{ *$}}
; CHECK: Field LLVM Type: %struct.test20*
; CHECK: Field info: Written
; CHECK: Field LLVM Type: i16
; CHECK: Field info: Written
; CHECK: Field LLVM Type: %struct.test20*
; CHECK: Field info:{{ *$}}
; CHECK: Safety data: Memfunc partial write


; This test checks using memset to write a subset of fields, but with
; including the; padding bytes after the last field.
%struct.test21 = type { i16, %struct.test21*, i16, %struct.test21* }
define void @test21(%struct.test21* %a) {
  %a1 = getelementptr inbounds %struct.test21, %struct.test21* %a, i64 0, i32 1
  %1 = bitcast %struct.test21** %a1 to i8*
  call void @llvm.memset.p0i8.i64(i8* %1, i8 0, i64 16, i1 false)
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test21 = type { i16, %struct.test21*, i16, %struct.test21* }
; CHECK: Field LLVM Type: i16
; CHECK: Field info:{{ *$}}
; CHECK: Field LLVM Type: %struct.test21*
; CHECK: Field info: Written
; CHECK: Field LLVM Type: i16
; CHECK: Field info: Written
; CHECK: Field LLVM Type: %struct.test21*
; CHECK: Field info:{{ *$}}
; CHECK: Safety data: Memfunc partial write


; This test checks using memset to write a subset of fields, but not including
; the padding bytes after the last field, which is also the last field of the
; structure.
%struct.test22 = type { i16, %struct.test22*, i16, %struct.test22*, i16 }
define void @test22(%struct.test22* %a) {
  %a1 = getelementptr inbounds %struct.test22, %struct.test22* %a, i64 0, i32 3
  %1 = bitcast %struct.test22** %a1 to i8*
  call void @llvm.memset.p0i8.i64(i8* %1, i8 0, i64 10, i1 false)
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test22 = type { i16, %struct.test22*, i16, %struct.test22*, i16 }
; CHECK: Field LLVM Type: i16
; CHECK: Field info:{{ *$}}
; CHECK: Field LLVM Type: %struct.test22*
; CHECK: Field info:{{ *$}}
; CHECK: Field LLVM Type: i16
; CHECK: Field info:{{ *$}}
; CHECK: Field LLVM Type: %struct.test22*
; CHECK: Field info: Written
; CHECK: Field LLVM Type: i16
; CHECK: Field info: Written
; CHECK: Safety data: Memfunc partial write


; This test checks using memset to write a subset of fields, but with including
; the padding bytes after the last field, which is also the last field of the
; structure.
%struct.test23 = type { i16, %struct.test23*, i16, %struct.test23*, i16 }
define void @test23(%struct.test23* %a) {
  %a1 = getelementptr inbounds %struct.test23, %struct.test23* %a, i64 0, i32 3
  %1 = bitcast %struct.test23** %a1 to i8*
  call void @llvm.memset.p0i8.i64(i8* %1, i8 0, i64 16, i1 false)
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test23 = type { i16, %struct.test23*, i16, %struct.test23*, i16 }
; CHECK: Field LLVM Type: i16
; CHECK: Field info:{{ *$}}
; CHECK: Field LLVM Type: %struct.test23*
; CHECK: Field info:{{ *$}}
; CHECK: Field LLVM Type: i16
; CHECK: Field info:{{ *$}}
; CHECK: Field LLVM Type: %struct.test23*
; CHECK: Field info: Written
; CHECK: Field LLVM Type: i16
; CHECK: Field info: Written
; CHECK: Safety data: Memfunc partial write


; This test checks using memset to write a subset of fields, but writes beyond
; the last field
%struct.test24 = type { i16, %struct.test24*, i16, %struct.test24*, i16 }
define void @test24(%struct.test24* %a) {
  %a1 = getelementptr inbounds %struct.test24, %struct.test24* %a, i64 0, i32 3
  %1 = bitcast %struct.test24** %a1 to i8*
  call void @llvm.memset.p0i8.i64(i8* %1, i8 0, i64 32, i1 false)
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test24 = type { i16, %struct.test24*, i16, %struct.test24*, i16 }
; CHECK: Multiple Value
; CHECK: Multiple Value
; CHECK: Multiple Value
; CHECK: Multiple Value
; CHECK: Multiple Value
; CHECK: Safety data: Bad memfunc size


; This test checks using a pointer to member of a scalar element of an array.
; This case is currently not handled.
%array.test25 = type [ 25 x i32 ]
define void @test25(%array.test25* %a) {
  %arr_mem = getelementptr inbounds %array.test25, %array.test25* %a, i64 0, i32 2
  %addr = bitcast i32* %arr_mem to i8*
  tail call void @llvm.memset.p0i8.i64(i8* %addr, i8 0, i64 4, i1 false)
  ret void
}


; This test checks using a pointer to member of a structure element of an
; array.
; This case is currently not handled.
%struct.test26 = type { i32, i32 }
%array.test26 = type [26 x %struct.test26]
define void @test26(%array.test26* %a) {
  %arr_mem = getelementptr inbounds %array.test26, %array.test26* %a, i64 0, i32 2
  %addr = bitcast %struct.test26* %arr_mem to i8*
  tail call void @llvm.memset.p0i8.i64(i8* %addr, i8 0, i64 8, i1 false)
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test26 = type { i32, i32 }
; CHECK: Safety data: Unhandled use


; Array types get printed last so these checks aren't with their IR.

; CHECK: DTRANS_ArrayInfo:
; CHECK-LABEL: LLVMType: [25 x i32]
; CHECK: Safety data: Unhandled use

; CHECK: DTRANS_ArrayInfo:
; CHECK-LABEL: LLVMType: [26 x %struct.test26]
; CHECK: Safety data: Unhandled use

declare void @llvm.memset.p0i8.i64(i8*, i8, i64, i1)
declare noalias i8* @malloc(i64)
