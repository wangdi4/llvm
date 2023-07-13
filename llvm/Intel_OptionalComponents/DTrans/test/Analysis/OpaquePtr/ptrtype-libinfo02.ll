; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s

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

  %r = call i32 @llvm.eh.typeid.for(ptr @_ZTIi)
  call void (...) @llvm.icall.branch.funnel(ptr %p8.3, ptr getelementptr (i8, ptr @g, i64 0))
  call void @llvm.lifetime.start.p0(i64 0, ptr %p8.1)
  call void @llvm.lifetime.end.p0(i64 0, ptr %p8.1)
  call void @llvm.memcpy.p0.p0.i64(ptr %p8.1, ptr %p8.2, i64 1, i1 false)
  call void @llvm.memmove.p0.p0.i64(ptr %p8.1, ptr %p8.2, i64 1, i1 false)
  call void @llvm.memset.p0.i64(ptr %p8.1, i8 0, i64 1, i1 false)

  call void @llvm.prefetch.p0(ptr %p8.1, i32 0, i32 3, i32 1)

  %ss = call ptr @llvm.stacksave()
  call void @llvm.stackrestore(ptr %ss)

  call void @llvm.va_start(ptr %p8.2)
  call void @llvm.va_copy(ptr %p8.2, ptr %p8.1)
  call void @llvm.va_end(ptr %p8.2)

  ret void
}

; Intrinsic library functions declared without DTrans metadata.
declare i32 @llvm.eh.typeid.for(ptr)
declare void @llvm.icall.branch.funnel(...)
declare void @llvm.lifetime.start.p0(i64, ptr)
declare void @llvm.lifetime.end.p0(i64, ptr)
declare void @llvm.memcpy.p0.p0.i64(ptr, ptr, i64, i1)
declare void @llvm.memmove.p0.p0.i64(ptr, ptr, i64, i1)
declare void @llvm.memset.p0.i64(ptr, i8, i64, i1)
declare void @llvm.prefetch.p0(ptr, i32, i32, i32)
declare void @llvm.stackrestore(ptr)
declare ptr @llvm.stacksave()
declare void @llvm.va_copy(ptr, ptr)
declare void @llvm.va_end(ptr)
declare void @llvm.va_start(ptr)

!intel.dtrans.types = !{}
