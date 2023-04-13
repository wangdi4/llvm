; This test verifies that "@llvm.memcpy.p0.p0.i64" call is NOT removed by
; instcombine using Andersens's points-to analysis.
; Andersens's points-to analysis shouldn't treat mmap as allocation call
; when mmap has definition.

; RUN: opt < %s -passes='require<anders-aa>,instcombine' -S 2>&1 | FileCheck %s

; CHECK: define internal fastcc void @mmap
; CHECK: call void @llvm.memcpy.p0.p0.i64(ptr

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.ptunit_result = type { ptr, ptr, ptr, ptr, ptr, ptr, ptr }

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg)

define dso_local i32 @main() {
  %tmp1 = alloca %struct.ptunit_result, align 8
  %tmp2 = alloca %struct.ptunit_result, align 8
  call fastcc void @mmap(ptr %tmp2, ptr %tmp1)
  ret i32 1
}

define internal fastcc void @mmap(ptr %agg.result, ptr %src) {
entry:
  call void @llvm.memcpy.p0.p0.i64(ptr %agg.result, ptr %src, i64 56, i1 false)
  ret void
}

