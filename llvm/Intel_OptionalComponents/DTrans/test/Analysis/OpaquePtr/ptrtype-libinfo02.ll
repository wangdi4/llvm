; REQUIRES: asserts

; RUN: opt -disable-output -whole-program-assume -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s
; RUN: opt -disable-output -whole-program-assume -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s

; Check that the DTrans PtrTypeAnalyzer does not set UNHANDLED on pointers used
; for intrinsic library functions that are declared without DTrans metadata
; attached to them because an internal lookup table can be used to resolve their
; types.

define void @test_intrinsic() {
  %p8.1 = alloca i8
; CHECK: %p8.1 = alloca i8
; CHECK:   LocalPointerInfo:
; CHECK-NOT: UNHANDLED

  %p8.2 = alloca i8
; CHECK: %p8.2 = alloca i8
; CHECK:   LocalPointerInfo:
; CHECK-NOT: UNHANDLED

  call void @llvm.lifetime.start.p0i8(i64 0, i8* %p8.1)
  call void @llvm.lifetime.end.p0i8(i64 0, i8* %p8.1)
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %p8.1, i8* %p8.2, i64 1, i1 false)
  call void @llvm.memmove.p0i8.p0i8.i64(i8* %p8.1, i8* %p8.2, i64 1, i1 false)
  call void @llvm.memset.p0i8.i64(i8* %p8.1, i8 0, i64 1, i1 false)

  ret void
}

; Intrinsic library functions declared without DTrans metadata.
declare void @llvm.lifetime.start.p0i8(i64, i8*)
declare void @llvm.lifetime.end.p0i8(i64, i8*)
declare void @llvm.memcpy.p0i8.p0i8.i64(i8*, i8*, i64, i1)
declare void @llvm.memmove.p0i8.p0i8.i64(i8*, i8*, i64, i1)
declare void @llvm.memset.p0i8.i64(i8*, i8, i64, i1)

!intel.dtrans.types = !{}
