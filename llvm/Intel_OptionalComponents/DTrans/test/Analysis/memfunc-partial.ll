; RUN: opt < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s

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
; CHECK: Safety data: Memfunc partial write

; memcpy: check that the first field is marked as ComplexUse and Memfunc partial write issue is set.
%struct.test02 = type { i32, i32 }
define void @test02(%struct.test02* %s1) {
  %p1 = bitcast %struct.test02* %s1 to i8*
  call void @llvm.memset.p0i8.i64(i8* %p1, i8 0, i64 4, i1 false)
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test02 = type { i32, i32 }
; CHECK: Field info
; CHECK-SAME: ComplexUse
; CHECK: Safety data: Memfunc partial write

declare void @llvm.memcpy.p0i8.p0i8.i64(i8*, i8*, i64, i1)
declare void @llvm.memset.p0i8.i64(i8*, i8, i64, i1)

