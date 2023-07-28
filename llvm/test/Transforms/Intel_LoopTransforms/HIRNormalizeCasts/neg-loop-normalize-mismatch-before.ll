; RUN: opt -passes="hir-ssa-deconstruction,print<hir>,hir-normalize-casts,print<hir>" -debug-only=hir-normalize-casts %s -disable-output 2>&1 | FileCheck %s

; This test case checks that the zext casting in the loop upperbound was not
; converted into sext. The reason is that the loop uses %n as sext and zext.
; In this case we found the zext memref before we collect the sext memref.
; It was created from the following C++ code, but the casting was added in
; the IR:

; unsigned foo(unsigned *a, int n) {
;   unsigned res = 0;
;   for (int i = 0; i < n; i++) {
;     res += a[n + i] + a[n + i];
;   }
;   return res;
; }

; HIR before transformation

; CHECK: BEGIN REGION { }
; CHECK:       + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:       |   %4 = (%a)[i1 + zext.i32.i64(%n)];
; CHECK:       |   %5 = (%a)[i1 + sext.i32.i64(%n)];
; CHECK:       |   %res.09 = %4 + %5  +  %res.09;
; CHECK:       + END LOOP
; CHECK: END REGION


; HIR after transformation

; CHECK: BEGIN REGION { }
; CHECK:       + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:       |   %4 = (%a)[i1 + zext.i32.i64(%n)];
; CHECK:       |   %5 = (%a)[i1 + sext.i32.i64(%n)];
; CHECK:       |   %res.09 = %4 + %5  +  %res.09;
; CHECK:       + END LOOP
; CHECK: END REGION


;Module Before HIR
; ModuleID = 'simple.cpp'
source_filename = "simple.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: argmemonly mustprogress nofree norecurse nosync nounwind readonly willreturn uwtable
define dso_local noundef i32 @_Z3fooPji(ptr nocapture noundef readonly %a, i32 noundef %n) local_unnamed_addr {
entry:
  %cmp8 = icmp sgt i32 %n, 0
  br i1 %cmp8, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %0 = sext i32 %n to i64
  %1 = zext i32 %n to i64
  %ub = zext i32 %n to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  %add1.lcssa = phi i32 [ %add1, %for.body ]
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  %res.0.lcssa = phi i32 [ 0, %entry ], [ %add1.lcssa, %for.cond.cleanup.loopexit ]
  ret i32 %res.0.lcssa

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %res.09 = phi i32 [ 0, %for.body.preheader ], [ %add1, %for.body ]
  %2 = add nuw nsw i64 %indvars.iv, %1
  %arrayidx = getelementptr inbounds i32, ptr %a, i64 %2
  %3 = add nuw nsw i64 %indvars.iv, %0
  %arrayidx2 = getelementptr inbounds i32, ptr %a, i64 %3
  %4 = load i32, ptr %arrayidx, align 4
  %5 = load i32, ptr %arrayidx2, align 4
  %tadd = add i32 %5, %4
  %add1 = add i32 %tadd, %res.09
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %ub
  br i1 %exitcond.not, label %for.cond.cleanup.loopexit, label %for.body
}
