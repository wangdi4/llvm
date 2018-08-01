; RUN: opt < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -passes="require<dtransanalysis>" -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; memcpy: check that the first field is marked as ComplexUse and Memfunc partial write issue is set.
%struct.test01 = type { i32, i32 }
define void @test01(%struct.test01* %s1, %struct.test01* %s2) {
  %p1 = bitcast %struct.test01* %s1 to i8*
  %p2 = bitcast %struct.test01* %s2 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %p1, i8* %p2, i64 4, i1 false)
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test01 = type { i32, i32 }
; CHECK: Field info
; CHECK-SAME: ComplexUse
; CHECK: Field info
; CHECK-NOT: ComplexUse
; CHECK: Safety data: Memfunc partial write

; memset: check that the first field is marked as ComplexUse and Memfunc partial write issue is set.
%struct.test02 = type { i32, i32 }
define void @test02(%struct.test02* %s1) {
  %p1 = bitcast %struct.test02* %s1 to i8*
  call void @llvm.memset.p0i8.i64(i8* %p1, i8 0, i64 4, i1 false)
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test02 = type { i32, i32 }
; CHECK: Field info
; CHECK-SAME: ComplexUse
; CHECK: Field info
; CHECK-NOT: ComplexUse
; CHECK: Safety data: Memfunc partial write

; memcpy: check that the second field is marked as ComplexUse and Memfunc partial write issue is set.
%struct.test03 = type { i32, i32 }
define void @test03(%struct.test03* %s1, %struct.test03* %s2) {
  %p01 = getelementptr %struct.test03, %struct.test03* %s1, i64 0, i32 1
  %p02 = getelementptr %struct.test03, %struct.test03* %s2, i64 0, i32 1

  %p1 = bitcast i32* %p01 to i8*
  %p2 = bitcast i32* %p02 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %p1, i8* %p2, i64 4, i1 false)
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test03 = type { i32, i32 }
; CHECK: Field info
; CHECK-NOT: ComplexUse
; CHECK: Field info
; CHECK-SAME: ComplexUse
; CHECK: Safety data: Memfunc partial write

; memset: check that the second field is marked as ComplexUse and Memfunc partial write issue is set.
%struct.test04 = type { i32, i32 }
define void @test04(%struct.test04* %s1) {
  %p01 = getelementptr %struct.test04, %struct.test04* %s1, i64 0, i32 1
  %p1 = bitcast i32* %p01 to i8*
  call void @llvm.memset.p0i8.i64(i8* %p1, i8 0, i64 4, i1 false)
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test04 = type { i32, i32 }
; CHECK: Field info
; CHECK-NOT: ComplexUse
; CHECK: Field info
; CHECK-SAME: ComplexUse
; CHECK: Safety data: Memfunc partial write

declare void @llvm.memcpy.p0i8.p0i8.i64(i8*, i8*, i64, i1)
declare void @llvm.memset.p0i8.i64(i8*, i8, i64, i1)

