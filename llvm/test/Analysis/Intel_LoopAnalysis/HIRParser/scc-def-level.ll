; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-framework -hir-framework-debug=parser -hir-details | FileCheck %s
; RUN: opt < %s -passes=hir-ssa-deconstruction | opt -passes="print<hir-framework>" -hir-framework-debug=parser -hir-details 2>&1 | FileCheck %s

; Verify that the def level of the use of %add41.lcssa78 in inner loop is marked
; as non-linear. It was getting marked as def@1 because we formed this SCC:
; (%add41.lcssa -> %add41 -> %add41.lcssa78) and then used the definition level
; of SCC root %add41.lcssa78 which is the outer loop header phi.

; CHECK: + DO i32 i1 = 0, %div + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>
; CHECK: |   + DO i64 i2 = 0, 9, 1   <DO_LOOP>
; CHECK: |   |   %ld1 = (%ptr1)[2 * i2 + 2];
; CHECK: |   |   %ld2 = (%ptr2)[2 * i2 + 2];
; CHECK: |   |   %mul39 = %ld1  *  %ld2;
; CHECK: |   |   %add41.lcssa78 = %add41.lcssa78  +  %mul39;
; CHECK:         <RVAL-REG> NON-LINEAR double %add41.lcssa78

; CHECK: |   |   (%ptr1)[2 * i2 + 2] = %add41.lcssa78;
; CHECK: |   + END LOOP
; CHECK: |
; CHECK: |   %add41.lcssa78.out = %add41.lcssa78;
; CHECK: + END LOOP


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind readnone speculatable
declare double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8, i64, i64, double*, i64) #0

define void @foo(i32 %div, double* noalias nocapture %ptr1, double* noalias nocapture readonly %ptr2) {
alloca_2:
  %rel = icmp slt i32 %div, 1
  br i1 %rel, label %bb95, label %bb105.preheader

bb105.preheader:                                  ; preds = %alloca_2
  %0 = add nuw nsw i32 %div, 1
  br label %bb105

bb105:                                            ; preds = %bb134, %bb105.preheader
  %add41.lcssa78 = phi double [ %add41.lcssa, %bb134 ], [ 0.000000e+00, %bb105.preheader ]
  %j = phi i32 [ %add61, %bb134 ], [ 1, %bb105.preheader ]
  br label %bb109

bb109:                                            ; preds = %bb109, %bb105
  %indvars.iv = phi i64 [ %indvars.iv.next, %bb109 ], [ 1, %bb105 ]
  %indvars.iv.next = add nsw i64 %indvars.iv, 2
  %1 = add nsw i64 %indvars.iv, 1
  %gep1 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull %ptr1, i64 %indvars.iv.next)
  %ld1 = load double, double* %gep1, align 1
  %gep2 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* %ptr2, i64 %indvars.iv.next)
  %ld2 = load double, double* %gep2, align 1
  %mul39 = fmul double %ld1, %ld2
  %add41 = fadd double %add41.lcssa78, %mul39
  store double %add41, double* %gep1, align 1
  %exitcond = icmp eq i64 %indvars.iv.next, 21
  br i1 %exitcond, label %bb134, label %bb109

bb134:                                            ; preds = %bb109
  %add41.lcssa = phi double [ %add41, %bb109 ]
  %add61 = add nuw nsw i32 %j, 1
  %exitcond80 = icmp eq i32 %add61, %0
  br i1 %exitcond80, label %bb95.loopexit, label %bb105

bb95.loopexit:                                    ; preds = %bb134
  %add41.lcssa.lcssa = phi double [ %add41.lcssa, %bb134 ]
  br label %bb95

bb95:                                             ; preds = %bb95.loopexit, %alloca_2
  ret void
}

