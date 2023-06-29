; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-lmm,print<hir>" -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-lmm" -print-changed -disable-output < %s 2>&1 | FileCheck %s --check-prefix=CHECK-CHANGED

; This test case checks that the invariant bitcast instruction is hoisted
; out of the loop after applying LMM.

; HIR before transformation

; BEGIN REGION { }
;       + DO i1 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;       |   %conv = uitofp.i8.float((%src)[0]);
;       |   (%0)[i1] = %conv;
;       + END LOOP
; END REGION

; HIR after transformation

; CHECK: BEGIN REGION { modified }
; CHECK:          %conv = uitofp.i8.float((%src)[0]);
; CHECK:       + DO i1 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:       |   (%0)[i1] = %conv;
; CHECK:       + END LOOP
; CHECK: END REGION

; Verify that pass is dumped with print-changed when it triggers.

; CHECK-CHANGED: Dump Before HIRTempCleanup
; CHECK-CHANGED: Dump After HIRLMM

;Module Before HIR
; ModuleID = 'repo.cpp'
source_filename = "repo.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@data_ = dso_local local_unnamed_addr global ptr null

; Function Attrs: mustprogress nofree norecurse nosync nounwind memory(readwrite, inaccessiblemem: none) uwtable
define dso_local void @_Z19import_partial_grayPKhiii(ptr nocapture noundef noalias readonly %src, i32 noundef %n) {
entry:
  %0 = load ptr, ptr @data_
  %cmp = icmp sgt i32 %n, 0
  br i1 %cmp, label %for.body.preheader, label %if.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %pix.052 = phi i32 [ %inc, %for.body ], [ 0, %for.body.preheader ]
  %1 = load i8, ptr %src
  %conv = uitofp i8 %1 to float
  %idx = zext i32 %pix.052 to i64
  %dst_p.053 = getelementptr inbounds float, ptr %0, i64 %idx
  store float %conv, ptr %dst_p.053
  %inc = add i32 %pix.052, 1
  %exitcond.not = icmp eq i32 %inc, %n
  br i1 %exitcond.not, label %if.end.loopexit, label %for.body

if.end.loopexit:                                  ; preds = %for.body
  br label %if.end

if.end:                                           ; preds = %if.end.loopexit, %entry
  ret void
}
