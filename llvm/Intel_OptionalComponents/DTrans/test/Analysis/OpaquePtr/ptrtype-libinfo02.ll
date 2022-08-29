; REQUIRES: asserts

; RUN: opt -disable-output -whole-program-assume -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s
; RUN: opt -disable-output -whole-program-assume -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s

; Check that the DTrans PtrTypeAnalyzer does not set UNHANDLED on pointers used
; for intrinsic library functions that are declared without DTrans metadata
; attached to them because an internal lookup table can be used to resolve their
; types.

@g = external global i8
; CHECK:   LocalPointerInfo:
; CHECK-NOT: UNHANDLED

@_ZTIi = external dso_local constant i8
; CHECK:   LocalPointerInfo:
; CHECK-NOT: UNHANDLED

define void @test_intrinsic() {
  %p8.1 = alloca i8
; CHECK: %p8.1 = alloca i8
; CHECK:   LocalPointerInfo:
; CHECK-NOT: UNHANDLED

  %p8.2 = alloca i8
; CHECK: %p8.2 = alloca i8
; CHECK:   LocalPointerInfo:
; CHECK-NOT: UNHANDLED

  %p8.3 = alloca i8
; CHECK: %p8.3 = alloca i8
; CHECK:   LocalPointerInfo:
; CHECK-NOT: UNHANDLED

  %r = call i32 @llvm.eh.typeid.for(i8* @_ZTIi)
  call void (...) @llvm.icall.branch.funnel(i8* %p8.3, i8* getelementptr (i8, i8* @g, i64 0))
  call void @llvm.lifetime.start.p0i8(i64 0, i8* %p8.1)
  call void @llvm.lifetime.end.p0i8(i64 0, i8* %p8.1)
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %p8.1, i8* %p8.2, i64 1, i1 false)
  call void @llvm.memmove.p0i8.p0i8.i64(i8* %p8.1, i8* %p8.2, i64 1, i1 false)
  call void @llvm.memset.p0i8.i64(i8* %p8.1, i8 0, i64 1, i1 false)

  call void @llvm.prefetch.p0i8(i8* %p8.1, i32 0, i32 3, i32 1)

  %ss = call i8* @llvm.stacksave()
  call void @llvm.stackrestore(i8* %ss)

  call void @llvm.va_start(i8* %p8.2)
  call void @llvm.va_copy(i8* %p8.2, i8* %p8.1)
  call void @llvm.va_end(i8* %p8.2)

  ret void
}

; Intrinsic library functions declared without DTrans metadata.
declare i32 @llvm.eh.typeid.for(i8*)
declare void @llvm.icall.branch.funnel(...)
declare void @llvm.lifetime.start.p0i8(i64, i8*)
declare void @llvm.lifetime.end.p0i8(i64, i8*)
declare void @llvm.memcpy.p0i8.p0i8.i64(i8*, i8*, i64, i1)
declare void @llvm.memmove.p0i8.p0i8.i64(i8*, i8*, i64, i1)
declare void @llvm.memset.p0i8.i64(i8*, i8, i64, i1)
declare void @llvm.prefetch.p0i8(i8*, i32, i32, i32)
declare void @llvm.stackrestore(i8*)
declare i8* @llvm.stacksave()
declare void @llvm.va_copy(i8*, i8*)
declare void @llvm.va_end(i8*)
declare void @llvm.va_start(i8*)

!intel.dtrans.types = !{}
