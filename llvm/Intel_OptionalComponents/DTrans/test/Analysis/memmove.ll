; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s


; Call memmove with matched struct pointers.
; This is an safe use.
%struct.test01 = type { i32, i32 }
define void @test01(%struct.test01* %s1, %struct.test01* %s2) {
  %p1 = bitcast %struct.test01* %s1 to i8*
  %p2 = bitcast %struct.test01* %s2 to i8*
  call void @llvm.memmove.p0i8.p0i8.i64(i8* %p1, i8* %p2, i64 8, i1 false)
  ret void
}
; CHECK: LLVMType: %struct.test01 = type { i32, i32 }
; CHECK: Safety data: No issues found

; Call memmove with mismatched struct pointers.
; This is an unsafe use.
%struct.test02.a = type { i32, i32 }
%struct.test02.b = type { i16, i16, i32 }
define void @test02(%struct.test02.a* %sa, %struct.test02.b* %sb) {
  %pa = bitcast %struct.test02.a* %sa to i8*
  %pb = bitcast %struct.test02.b* %sb to i8*
  call void @llvm.memmove.p0i8.p0i8.i64(i8* %pa, i8* %pb, i64 8, i1 false)
  ret void
}
; CHECK: LLVMType: %struct.test02.a = type { i32, i32 }
; CHECK: Safety data: Bad memfunc manipulation
; CHECK: LLVMType: %struct.test02.b = type { i16, i16, i32 }
; CHECK: Safety data: Bad memfunc manipulation


; Call memmove with matched struct pointers, but only part of the
; structure is copied.
; This is should set the memfunc partial write safety bit.
%struct.test03 = type { i32, i32, i32, i32 }
define void @test03(%struct.test03* %s1, %struct.test03* %s2) {
  %p1 = bitcast %struct.test03* %s1 to i8*
  %p2 = bitcast %struct.test03* %s2 to i8*
  call void @llvm.memmove.p0i8.p0i8.i64(i8* %p1, i8* %p2, i64 8, i1 false)
  ret void
}
; CHECK: LLVMType: %struct.test03 = type { i32, i32, i32, i32 }
; CHECK: Safety data: Memfunc partial write


declare void @llvm.memmove.p0i8.p0i8.i64(i8* , i8*, i64, i1)
