; negative test: check that mm+index idiom analyzer bailouts in non-idiom cases.
; In this test we have non linear (invariant) value assigned for the index part
; REQUIRES: asserts
; RUN: opt -passes='hir-ssa-deconstruction,hir-temp-cleanup,hir-vec-dir-insert' -enable-mmindex=1 -disable-nonlinear-mmindex=1 -debug-only=parvec-analysis -S < %s 2>&1 | FileCheck %s
; TODO: add RUN with enabled non-linear indexes after support is implemented.

; Input HIR. The statement <7> contains the invariant value (2) assigned to the
; index part of the min/max+index idiom.
;
; <0>          BEGIN REGION { }
; <14>               + DO i1 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>
; <4>                |   %1 = (%a)[i1];
; <7>                |   %ndx.016 = (%1 > %max.015) ? 2 : %ndx.016;
; <6>                |   %max.015 = (%1 > %max.015) ? %1 : %max.015;
; <14>               + END LOOP
; <0>          END REGION
;
;CHECK: [MinMax+Index] Looking at candidate
;CHECK-NEXT: [MinMax+Index] Depends on
;CHECK-NEXT: [MinMax+Index] Skipped: nonlinear rhs disabled (invariant)

; Function Attrs: norecurse nounwind readonly willreturn mustprogress
define dso_local i32 @_Z3fooPii(ptr nocapture readonly %a, i32 %n) local_unnamed_addr {
entry:
  %0 = load i32, ptr %a, align 4
  %cmp14 = icmp sgt i32 %n, 0
  br i1 %cmp14, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  %spec.select.lcssa = phi i32 [ %spec.select, %for.body ]
  %spec.select13.lcssa = phi i32 [ %spec.select13, %for.body ]
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  %max.0.lcssa = phi i32 [ %0, %entry ], [ %spec.select.lcssa, %for.cond.cleanup.loopexit ]
  %ndx.0.lcssa = phi i32 [ 1, %entry ], [ %spec.select13.lcssa, %for.cond.cleanup.loopexit ]
  %add = add nsw i32 %ndx.0.lcssa, %max.0.lcssa
  ret i32 %add

for.body:                                         ; preds = %for.body.preheader, %for.body
  %i.017 = phi i32 [ %inc, %for.body ], [ 0, %for.body.preheader ]
  %ndx.016 = phi i32 [ %spec.select13, %for.body ], [ 1, %for.body.preheader ]
  %max.015 = phi i32 [ %spec.select, %for.body ], [ %0, %for.body.preheader ]
  %ptridx1 = getelementptr inbounds i32, ptr %a, i32 %i.017
  %1 = load i32, ptr %ptridx1, align 4
  %cmp2 = icmp sgt i32 %1, %max.015
  %spec.select = select i1 %cmp2, i32 %1, i32 %max.015
  %spec.select13 = select i1 %cmp2, i32 2, i32 %ndx.016
  %inc = add nuw nsw i32 %i.017, 1
  %exitcond.not = icmp eq i32 %inc, %n
  br i1 %exitcond.not, label %for.cond.cleanup.loopexit, label %for.body
}


