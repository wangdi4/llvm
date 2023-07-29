; REQUIRES: asserts
; RUN: opt -passes="hir-ssa-deconstruction,print<hir>,hir-normalize-casts,print<hir>" -debug-only=hir-normalize-casts %s -disable-output 2>&1 | FileCheck %s

; This test case checks that the zext casting in the loop upperbound wasn't
; converted into sext. The reason is that the legal max trip count for the
; upperbound is larger than INT_MAX for i16. It was created from the following
; C++ code, but the casting was added in the IR:

; unsigned foo(unsigned *a, int n, int n2) {
;   unsigned res = 0;
;   for (int i = 0; i < n - 2; i++) {
;     res += a[n + i];
;   }
;   return res;
; }

; HIR before transformation

; CHECK:  BEGIN REGION { }
; CHECK:        + DO i1 = 0, zext.i16.i32(%n) + -3, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>  <LEGAL_MAX_TC = 4294967295>
; CHECK:        |   %2 = (%a)[i1 + sext.i16.i64(%n)];
; CHECK:        |   %res.09 = %2  +  %res.09;
; CHECK:        + END LOOP
; CHECK:  END REGION

; HIR after transformation

; CHECK:  BEGIN REGION { }
; CHECK:        + DO i1 = 0, zext.i16.i32(%n) + -3, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>  <LEGAL_MAX_TC = 4294967295>
; CHECK:        |   %2 = (%a)[i1 + sext.i16.i64(%n)];
; CHECK:        |   %res.09 = %2  +  %res.09;
; CHECK:        + END LOOP
; CHECK:  END REGION


;Module Before HIR
; ModuleID = 'simple.cpp'
source_filename = "simple.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: argmemonly mustprogress nofree norecurse nosync nounwind readonly willreturn uwtable
define dso_local noundef i32 @_Z3fooPji(ptr nocapture noundef readonly %a, i16 noundef %n) local_unnamed_addr {
entry:
  %zn = zext i16 %n to i32
  %ub = sub i32 %zn, 2
  %cmp8 = icmp sgt i32 %ub, 0
  br i1 %cmp8, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %0 = sext i16 %n to i32
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  %add1.lcssa = phi i32 [ %add1, %for.body ]
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  %res.0.lcssa = phi i32 [ 0, %entry ], [ %add1.lcssa, %for.cond.cleanup.loopexit ]
  ret i32 %res.0.lcssa

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i32 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %res.09 = phi i32 [ 0, %for.body.preheader ], [ %add1, %for.body ]
  %1 = add nuw nsw i32 %indvars.iv, %0
  %arrayidx = getelementptr inbounds i32, ptr %a, i32 %1
  %2 = load i32, ptr %arrayidx
  %add1 = add i32 %2, %res.09
  %indvars.iv.next = add nuw nsw i32 %indvars.iv, 1
  %exitcond.not = icmp eq i32 %indvars.iv.next, %ub
  br i1 %exitcond.not, label %for.cond.cleanup.loopexit, label %for.body
}
