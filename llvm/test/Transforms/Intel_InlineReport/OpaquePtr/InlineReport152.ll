; RUN: opt -passes='cgscc(inline),dse,inlinereportmakecurrent' -inline-report=0xe807 < %s -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-CL
; RUN: opt -passes='inlinereportsetup,cgscc(inline),dse,inlinereportemitter' -inline-report=0xe886 < %s -S 2>&1 | FileCheck %s

; Check that the classic inlining report notes that a malloc and memset were
; replaced by calloc, while the metadata inlining report misses the calloc
; because it was added after inlining was performed.

; CHECK-LABEL: COMPILE FUNC: AcquireQuantizeInfo
; CHECK: DELETE: malloc
; CHECK: DELETE: llvm.memset.p0.i64
; CHECK-CL: EXTERN: calloc

%struct._ZTS14_ExceptionInfo._ExceptionInfo = type { i32, i32, ptr, ptr, ptr, i32, ptr, i64 }

declare dso_local noalias noundef ptr @malloc(i64 noundef)

declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg)

define fastcc ptr @AcquireQuantizeInfo(ptr noundef readonly %0) unnamed_addr {
  %2 = alloca %struct._ZTS14_ExceptionInfo._ExceptionInfo, align 8
  %3 = tail call dereferenceable_or_null(48) ptr @malloc(i64 noundef 48)
  tail call void @llvm.memset.p0.i64(ptr noundef nonnull align 1 dereferenceable(48) %3, i8 0, i64 48, i1 false)
  ret ptr %3
}
