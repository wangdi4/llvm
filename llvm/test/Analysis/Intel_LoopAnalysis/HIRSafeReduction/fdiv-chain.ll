; RUN: opt < %s -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir-safe-reduction-analysis>" -disable-output 2>&1 | FileCheck %s

; This test case checks that safe reduction analysis was applied when there
; is a chain of divisions. It was created from the following source code:

;       subroutine sub (a,b,s,n,u,v)
;         real a(1000), b(1000), s, v, t
;         s = v
;         t = u
;         !dir$ vector always assert
;         do i=1, n
;           t = s / a(i)
;           s  =  t / b(i)
;         enddo
;       end subroutine sub

; HIR representation

; BEGIN REGION { }
;       + DO i1 = 0, zext.i32.i64(%"sub_$N_fetch.2") + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647> <vectorize>
;       |   %1 = (%"sub_$B")[i1]  *  (%"sub_$A")[i1];
;       |   %div.26 = %div.26  /  %1;
;       + END LOOP
; END REGION

; Safe reduction analysis print

; CHECK:  + DO i1 = 0, zext.i32.i64(%"sub_$N_fetch.2") + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:  |   <Safe Reduction> Red Op: fdiv <Has Unsafe Algebra- No> <Conditional- No>
; CHECK:  |   %div.26 = %div.26  /  %1; <Safe Reduction>
; CHECK:  + END LOOP

; ModuleID = 'simple2.f90'
source_filename = "simple2.f90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @sub_(ptr noalias nocapture readonly dereferenceable(4) %"sub_$A", ptr noalias nocapture readonly dereferenceable(4) %"sub_$B", ptr noalias nocapture dereferenceable(4) %"sub_$S", ptr noalias nocapture readonly dereferenceable(4) %"sub_$N", ptr noalias nocapture readnone dereferenceable(4) %"sub_$U", ptr noalias nocapture readonly dereferenceable(4) %"sub_$V") {
alloca_0:
  %"sub_$V_fetch.1" = load float, ptr %"sub_$V"
  store float %"sub_$V_fetch.1", ptr %"sub_$S"
  %"sub_$N_fetch.2" = load i32, ptr %"sub_$N"
  %rel.1 = icmp slt i32 %"sub_$N_fetch.2", 1
  br i1 %rel.1, label %bb3, label %bb2.preheader

bb2.preheader:                                    ; preds = %alloca_0
  %0 = add nuw nsw i32 %"sub_$N_fetch.2", 1
  %wide.trip.count = zext i32 %0 to i64
  br label %bb2

bb2:                                              ; preds = %bb2.preheader, %bb2
  %indvars.iv = phi i64 [ 1, %bb2.preheader ], [ %indvars.iv.next, %bb2 ]
  %div.26 = phi float [ %"sub_$V_fetch.1", %bb2.preheader ], [ %div.2, %bb2 ]
  %"sub_$A[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"sub_$A", i64 %indvars.iv)
  %"sub_$A[]_fetch.6" = load float, ptr %"sub_$A[]"
  %"sub_$B[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"sub_$B", i64 %indvars.iv)
  %"sub_$B[]_fetch.9" = load float, ptr %"sub_$B[]"
  %1 = fmul reassoc ninf nsz arcp contract afn float %"sub_$B[]_fetch.9", %"sub_$A[]_fetch.6"
  %div.2 = fdiv reassoc ninf nsz arcp contract afn float %div.26, %1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %bb3.loopexit, label %bb2

bb3.loopexit:                                     ; preds = %bb2
  %div.2.lcssa = phi float [ %div.2, %bb2 ]
  store float %div.2.lcssa, ptr %"sub_$S"
  br label %bb3

bb3:                                              ; preds = %bb3.loopexit, %alloca_0
  ret void
}

declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64)
