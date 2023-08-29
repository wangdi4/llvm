; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-lower-small-memset-memcpy,print<hir>" -hir-details-dims -disable-output < %s 2>&1 | FileCheck %s

; The test checks that HIR Lower Small Memset/Memcpy pass does not
; transform memset with strided mem ref. (%A)[0] has stride of 8 while
; element type is i32 (size 4), that implies strided access.

; HIR:
;            BEGIN REGION { }
;                  + DO i1 = 0, 2, 1   <DO_LOOP>
;                  |   @llvm.memset.p0.i64(&((ptr)(%A)[0]),  5,  12,  0);
;                  + END LOOP
;            END REGION

; HIR After optimization:
; CHECK:     BEGIN REGION { }
; CHECK:           + DO i1 = 0, 2, 1   <DO_LOOP>
; CHECK:           |   @llvm.memset.p0.i64(&((%A)[0:0:8(i32:0)]),  5,  12,  0);
; CHECK:           + END LOOP
; CHECK:     END REGION

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @MAIN__(ptr noalias %A) {
alloca:
  br label %bb17

bb17:                                             ; preds = %alloca, %bb17
  %iv = phi i64 [ 1, %alloca ], [ %add11, %bb17 ]
  %t1 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 8, ptr elementtype(i32) %A, i64 1)
  %bc = bitcast ptr %t1 to ptr
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 2 dereferenceable(6) %bc, i8 5, i64 12, i1 false)
  %add11 = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %add11, 4
  br i1 %exitcond, label %bb1, label %bb17

bb1:                                              ; preds = %bb17
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1

; Function Attrs: argmemonly mustprogress nocallback nofree nounwind willreturn writeonly
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #3

attributes #1 = { nounwind readnone speculatable }
attributes #3 = { argmemonly mustprogress nocallback nofree nounwind willreturn writeonly }

