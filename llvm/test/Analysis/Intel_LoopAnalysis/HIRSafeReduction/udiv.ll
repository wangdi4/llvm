; RUN: opt < %s -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir-safe-reduction-analysis>" -disable-output 2>&1 | FileCheck %s

; This test case checks that safe reduction analysis was applied when there
; is a division in the form of x = x / y, and the variables are unsigned
; integers. It was created from the following source code:

;       subroutine sub (a,s,n,v)
;         integer a(1000), s
;         s = v
;         !dir$ vector always assert
;         do i=1, n
;           s  =  s / a(i)
;         enddo
;       end subroutine sub

; NOTE: Fortran doesn't support unsigned natively. The test case will produce
; a signed interger division (sdiv). This test case was modified to use
; unsigned integer division (udiv).

; HIR representation

;   BEGIN REGION { }
;         + DO i1 = 0, zext.i32.i64(%"sub_$N_fetch.2") + -1, 1   <DO_LOOP>
;         |   %div.14 = %div.14  /u  (%"sub_$A")[i1];
;         + END LOOP
;   END REGION

; Safe reduction analysis print

; CHECK:            + DO i1 = 0, zext.i32.i64(%"sub_$N_fetch.2") + -1, 1   <DO_LOOP>
; CHECK:            |   <Safe Reduction> Red Op: udiv <Has Unsafe Algebra- No> <Conditional- No>
; CHECK:            |   %div.14 = %div.14  /u  (%"sub_$A")[i1]; <Safe Reduction>
; CHECK:            + END LOOP


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: argmemonly nofree nosync nounwind uwtable
define void @sub_(ptr noalias nocapture readonly dereferenceable(4) %"sub_$A", ptr noalias nocapture dereferenceable(4) %"sub_$S", ptr noalias nocapture readonly dereferenceable(4) %"sub_$N", ptr noalias nocapture readonly dereferenceable(4) %"sub_$V") local_unnamed_addr #0 {
alloca_0:
  %"sub_$V_fetch.1" = load i32, ptr %"sub_$V", align 1
  store i32 %"sub_$V_fetch.1", ptr %"sub_$S", align 1
  %"sub_$N_fetch.2" = load i32, ptr %"sub_$N", align 1
  %rel.1 = icmp ult i32 %"sub_$N_fetch.2", 1
  br i1 %rel.1, label %bb3, label %bb2.preheader

bb2.preheader:                                    ; preds = %alloca_0
  %0 = add nuw nsw i32 %"sub_$N_fetch.2", 1
  %wide.trip.count = zext i32 %0 to i64
  br label %bb2

bb2:                                              ; preds = %bb2.preheader, %bb2
  %indvars.iv = phi i64 [ 1, %bb2.preheader ], [ %indvars.iv.next, %bb2 ]
  %div.14 = phi i32 [ %"sub_$V_fetch.1", %bb2.preheader ], [ %div.1, %bb2 ]
  %"sub_$A_entry[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(i32) %"sub_$A", i64 %indvars.iv)
  %"sub_$A_entry[]_fetch.6" = load i32, ptr %"sub_$A_entry[]", align 1
  %div.1 = udiv i32 %div.14, %"sub_$A_entry[]_fetch.6"
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %bb3.loopexit, label %bb2

bb3.loopexit:                                     ; preds = %bb2
  %div.1.lcssa = phi i32 [ %div.1, %bb2 ]
  store i32 %div.1.lcssa, ptr %"sub_$S", align 1
  br label %bb3

bb3:                                              ; preds = %bb3.loopexit, %alloca_0
  ret void
}

; Function Attrs: nofree nosync nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1

attributes #0 = { argmemonly nofree nosync nounwind uwtable "denormal-fp-math"="preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nofree nosync nounwind readnone speculatable }

