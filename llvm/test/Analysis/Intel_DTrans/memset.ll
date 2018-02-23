; RUN: opt < %s -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s

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
  tail call void @llvm.memset.p0i8.i64(i8* %p, i8 0, i64 8, i32 4, i1 false)
  ret void
}
; CHECK: LLVMType: %struct.test01 = type { i32, i16, i8 }
; CHECK: Safety data: No issues found


; This test checks when multiple of the structure size is used, such as
; for an array of structures.
; This is a safe use.
%struct.test02 = type { i32, i16, i8 }
define void @test02(%struct.test02* %a) {
  %p = bitcast %struct.test02* %a to i8*
  tail call void @llvm.memset.p0i8.i64(i8* %p, i8 0, i64 32, i32 4, i1 false)
  ret void
}
; CHECK: LLVMType: %struct.test02 = type { i32, i16, i8 }
; CHECK: Safety data: No issues found


; This test checks when a structure is composed of structures. Each of the
; the structure fields should be marked as written.
; This is a safe use.
%struct.test03.a = type { i32, i16, [2 x i32] }
%struct.test03.b = type { i32, i16, i8 }
%struct.test03.c = type { %struct.test03.a, %struct.test03.b }
define void @test03(%struct.test03.c* %c) {
  %c0 = bitcast %struct.test03.c* %c to i8*
  tail call void @llvm.memset.p0i8.i64(i8* %c0, i8 0, i64 24, i32 4, i1 false)
  ret void
}
; CHECK: LLVMType: %struct.test03.a = type { i32, i16, [2 x i32] }
; CHECK: Field LLVM Type: i32
; CHECK-NEXT: Field info: Written
; CHECK: LLVMType: %struct.test03.b = type { i32, i16, i8 }
; CHECK: Field LLVM Type: i16
; CHECK-NEXT: Field info: Written
; CHECK: LLVMType: %struct.test03.c = type { %struct.test03.a, %struct.test03.b }
; CHECK: Safety data: No issues found


; This test checks when a structure is composed of pointers to structures.
; This is a safe use, and only members of struct.test04.c should be marked
; with 'Written'.
%struct.test04.a = type { i32, i16, [2 x i32] }
%struct.test04.b = type { i32, i16, i8 }
%struct.test04.c = type { %struct.test04.a*, %struct.test04.b* }
define void @test04(%struct.test04.c* %c) {
  %c0 = bitcast %struct.test04.c* %c to i8*
  tail call void @llvm.memset.p0i8.i64(i8* %c0, i8 0, i64 16, i32 4, i1 false)
  ret void
}
; CHECK: LLVMType: %struct.test04.a = type { i32, i16, [2 x i32] }
; CHECK: Field LLVM Type: i32
; CHECK-NEXT: Field info:
; CHECK: LLVMType: %struct.test04.b = type { i32, i16, i8 }
; CHECK: Field LLVM Type: i16
; CHECK-NEXT: Field info:
; CHECK: LLVMType: %struct.test04.c = type { %struct.test04.a*, %struct.test04.b* }
; CHECK: Number of fields: 2
; CHECK: Field LLVM Type: %struct.test04.a*
; CHECK: Field info: Written
; CHECK: Field LLVM Type: %struct.test04.b*
; CHECK: Field info: Written
; CHECK: Safety data: No issues found


; This test checks when memset only clears part of the structure.
; This is an unsafe use.
%struct.test05 = type { i32, i16, i16, i8 }
define void @unsafe1(%struct.test05* %b) {
  %p = bitcast %struct.test05* %b to i8*
  tail call void @llvm.memset.p0i8.i64(i8* %p, i8 0, i64 6, i32 4, i1 false)
  ret void
}
; CHECK: LLVMType: %struct.test05 = type { i32, i16, i16, i8 }
; CHECK: Safety data: Bad memfunc size


; This test checks when memset clears the structure based on a summation
; of the field types, ignoring tail padding of the structure.
; Currently, this is marked as unhandled, but the code could be extended
; to handle in the future.
%struct.test06 = type { i32, i16, i16, i8 }
define void @unsafe2(%struct.test06* %b) {
  %p = bitcast %struct.test06* %b to i8*
  tail call void @llvm.memset.p0i8.i64(i8* %p, i8 0, i64 9, i32 4, i1 false)
  ret void
}
; CHECK: LLVMType: %struct.test06 = type { i32, i16, i16, i8 }
; CHECK: Safety data: Bad memfunc size


; This test checks a pattern that is produced in special cases by the FE that
; reduces the structure to an i8*.
%struct.test07 = type { [200 x i8], [200 x i8], i64 }
@test07var = internal dso_local global %struct.test07 zeroinitializer, align 8
define void @test07(%struct.test07* %a) {
  %p = bitcast %struct.test07* %a to i8*
  tail call void @llvm.memset.p0i8.i64(i8* getelementptr inbounds (%struct.test07, %struct.test07* @test07var, i64 0, i32 0, i64 0), i8 0, i64 408, i32 4, i1 false)
  ret void
}
; FIXME: This should be resolve to "No issues found" once the getelemntptr processing is extended.
; CHECK: LLVMType: %struct.test07 = type { [200 x i8], [200 x i8], i64 }
; CHECK: Safety data: Unhandled use


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
  tail call void @llvm.memset.p0i8.i64(i8* %buf, i8 0, i64 128, i32 8, i1 false)
  ret void
}
; CHECK: LLVMType: %struct.test08.a = type { i32, i32, i32, i32 }
; CHECK: Number of fields: 4
; CHECK: Field LLVM Type: i32
; CHECK-NEXT: Field info:
; CHECK: Safety data: No issues found


; This test checks using memset with a pointer to a structure that is a structure
; member field of a parent stucture.
; This is a safe use.
%struct.test09.a = type { i32, i32, i32, i32, i32 }
%struct.test09.b = type { i32, %struct.test09.a }
define void @test09(%struct.test09.b* %b) {
  %a = getelementptr inbounds %struct.test09.b, %struct.test09.b* %b, i64 0, i32 1
  %a0 = bitcast %struct.test09.a* %a to i8*
  tail call void @llvm.memset.p0i8.i64(i8* nonnull %a0, i8 0, i64 20, i32 4, i1 false)
  ret void
}
; CHECK: LLVMType: %struct.test09.a = type { i32, i32, i32, i32, i32 }
; CHECK: Safety data: No issues found
; CHECK: LLVMType: %struct.test09.b = type { i32, %struct.test09.a }
; CHECK: Safety data: No issues found


; This test checks using memset with the address of a pointer to struct member
; field within a nested stucture. In this case, just the pointer field being is
; being written.
; This is a safe use.
%struct.test10.a = type { i32, i32, i32, i32, i32 }
%struct.test10.b = type { i32, %struct.test10.a* }
define void @test10(%struct.test10.b* %b) {
  %a = getelementptr inbounds %struct.test10.b, %struct.test10.b* %b, i64 0, i32 1
  %a0 = bitcast %struct.test10.a** %a to i8*
  tail call void @llvm.memset.p0i8.i64(i8* nonnull %a0, i8 0, i64 8, i32 4, i1 false)
  ret void
}
; CHECK: LLVMType: %struct.test10.a = type { i32, i32, i32, i32, i32 }
; CHECK: Safety data: No issues found
; CHECK: LLVMType: %struct.test10.b = type { i32, %struct.test10.a* }
; CHECK: Safety data: No issues found


; This test checks using memset with a pointer to a structure that is a structure
; member field of a parent stucture, but writes beyond the end of the structure.
; This is an unsafe use, and bad memfunc size should be propagated to both structures
; involved.
%struct.test11.a = type { i32, i32, i32, i32, i32 }
%struct.test11.b = type { i32, %struct.test11.a, i32 }
define void @test11(%struct.test11.b* %b) {
  %a = getelementptr inbounds %struct.test11.b, %struct.test11.b* %b, i64 0, i32 1
  %a0 = bitcast %struct.test11.a* %a to i8*
  tail call void @llvm.memset.p0i8.i64(i8* nonnull %a0, i8 0, i64 24, i32 4, i1 false)
  ret void
}
; CHECK: LLVMType: %struct.test11.a = type { i32, i32, i32, i32, i32 }
; CHECK: Safety data: Bad memfunc size
; CHECK: LLVMType: %struct.test11.b = type { i32, %struct.test11.a, i32 }
; CHECK: Safety data: Bad memfunc size


; This test checks using memset with address of a pointer to struct member
; field within a nested stucture. In this case, it writes beyond the end of
; of the field.
; This is a unsafe use.
%struct.test12.a = type { i32, i32, i32, i32, i32 }
%struct.test12.b = type { i32, %struct.test12.a*, i64 }
define void @test12(%struct.test12.b* %b) {
  %a = getelementptr inbounds %struct.test12.b, %struct.test12.b* %b, i64 0, i32 1
  %a0 = bitcast %struct.test12.a** %a to i8*
  tail call void @llvm.memset.p0i8.i64(i8* nonnull %a0, i8 0, i64 16, i32 4, i1 false)
  ret void
}
; CHECK: LLVMType: %struct.test12.a = type { i32, i32, i32, i32, i32 }
; CHECK: Safety data: No issues found
; CHECK: LLVMType: %struct.test12.b = type { i32, %struct.test12.a*, i64 }
; CHECK: Safety data: Bad memfunc size


; This test checks using memset with a pointer to a structure that is an array
; of structures field of a parent structure. Sets all 10 elements of array of structure.
; The structure passed into the memset should be considered safe.
%struct.test13.b = type { i32, [10 x %struct.test13.a] }
%struct.test13.a = type { i32, i32, i32, i32, i32 }
define void @test13(%struct.test13.b* %b) {
  %a = getelementptr inbounds %struct.test13.b, %struct.test13.b* %b, i64 0, i32 1
  %t0 = bitcast [10 x %struct.test13.a]* %a to i8*
  tail call void @llvm.memset.p0i8.i64(i8* nonnull %t0, i8 0, i64 200, i32 4, i1 false)
  ret void
}
; CHECK: LLVMType: %struct.test13.a = type { i32, i32, i32, i32, i32 }
; CHECK: Number of fields: 5
; CHECK: Field LLVM Type: i32
; CHECK-NEXT: Field info: Written
; CHECK: Safety data: No issues found
; CHECK: LLVMType: %struct.test13.b = type { i32, [10 x %struct.test13.a] }
; CHECK: Safety data: No issues found


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
  tail call void @llvm.memset.p0i8.i64(i8* nonnull %t0, i8 0, i64 100, i32 4, i1 false)
  ret void
}
; CHECK: LLVMType: %struct.test14.a = type { i32, i32, i32, i32, i32 }
; CHECK: Safety data: Bad memfunc size
; CHECK: LLVMType: %struct.test14.b = type { i32, [10 x %struct.test14.a] }
; CHECK: Safety data: Bad memfunc size


; This test checks using memset with the address of a scalar member of the
; structure.
; This is an unsafe use because we do not track type information for scalar
; pointers, and therefore are unable to resolve the pointer target.
%struct.test15.a = type { i32, i32, i32, i32, i32 }
define void @test15(%struct.test15.a* %a) {
  %c = getelementptr inbounds %struct.test15.a, %struct.test15.a* %a, i64 0, i32 2
  %t0 = bitcast i32* %c to i8*
  tail call void @llvm.memset.p0i8.i64(i8* nonnull %t0, i8 0, i64 12, i32 4, i1 false)
  ret void
}
; CHECK: LLVMType: %struct.test15.a = type { i32, i32, i32, i32, i32 }
; CHECK: Safety data: Bad memfunc manipulation


; This test checks using memset with a zero for the size parameter.
; This is a safe use, no matter what the pointer parameter points to.
%struct.test16.a = type { i32, i32, i32, i32, i32 }
define void @test16(%struct.test16.a* %a) {
  %c = getelementptr inbounds %struct.test16.a, %struct.test16.a* %a, i64 0, i32 2
  %t0 = bitcast i32* %c to i8*
  tail call void @llvm.memset.p0i8.i64(i8* nonnull %t0, i8 0, i64 0, i32 4, i1 false)
  ret void
}
; CHECK: LLVMType: %struct.test16.a = type { i32, i32, i32, i32, i32 }
; CHECK: Safety data: No issues found


declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i32, i1)
declare noalias i8* @malloc(i64)
