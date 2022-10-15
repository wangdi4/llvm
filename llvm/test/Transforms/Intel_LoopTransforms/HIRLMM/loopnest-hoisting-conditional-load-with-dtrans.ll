; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_dtrans

; RUN: opt -whole-program-assume -intel-libirc-allowed -dtrans-fieldmodref-analysis -hir-ssa-deconstruction -hir-lmm -print-before=hir-lmm -print-after=hir-lmm -hir-cost-model-throttling=0 -hir-lmm-loopnest-hoisting=true < %s 2>&1 | FileCheck %s

; RUN: opt -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-fieldmodref-analysis>,function(hir-ssa-deconstruction,print<hir>,hir-lmm,print<hir>)' -aa-pipeline="basic-aa" -hir-cost-model-throttling=0 -hir-lmm-loopnest-hoisting=true < %s 2>&1 | FileCheck %s

; This command verifies that we do not compfail in hir-lmm in the absence of FieldModRef results.
; RUN: opt -passes='hir-ssa-deconstruction,hir-lmm,print<hir>' -aa-pipeline="basic-aa" -hir-cost-model-throttling=0 -hir-lmm-loopnest-hoisting=true < %s 2>&1 | FileCheck %s --check-prefix=NO-COMPFAIL

; NO-COMPFAIL: DO i1

; Verify that we hoist the conditional load (%Pair)[0].0 in loopnest hoisting mode
; because of the identical region dominating load %firs.dom.load.

; Dump Before-

; CHECK: + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 10>
; CHECK: |   @bar(&((%Pair)[0]));
; CHECK: |
; CHECK: |   + DO i2 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 10>
; CHECK: |   |   if (i1 <u i2)
; CHECK: |   |   {
; CHECK: |   |      %first.load = (%Pair)[0].0;
; CHECK: |   |      %load = (@A)[0][i1][i2];
; CHECK: |   |      (@A)[0][i1][i2] = %first.load + %load;
; CHECK: |   |   }
; CHECK: |   + END LOOP
; CHECK: + END LOOP


; Dump After-

; CHECK: BEGIN REGION { modified }

; CHECK:   %first.load = (%Pair)[0].0;
; CHECK: + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 10>
; CHECK: |   @bar(&((%Pair)[0]));
; CHECK: |
; CHECK: |   + DO i2 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 10>
; CHECK: |   |   if (i1 <u i2)
; CHECK: |   |   {
; CHECK: |   |      %load = (@A)[0][i1][i2];
; CHECK: |   |      (@A)[0][i1][i2] = %first.load + %load;
; CHECK: |   |   }
; CHECK: |   + END LOOP
; CHECK: + END LOOP


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.IntPair = type { i32, i32 }

@A = common dso_local local_unnamed_addr global [10 x [10 x i32]] zeroinitializer, align 16

; Function Attrs: nofree norecurse nounwind uwtable
define dso_local void @bar(%struct.IntPair* noalias %Pair) {
entry:
  %second = getelementptr inbounds %struct.IntPair, %struct.IntPair* %Pair, i64 0, i32 1
  %second.load = load i32, i32* %second, align 4
  %inc = add nsw i32 %second.load, 1
  store i32 %inc, i32* %second, align 4
  ret void
}

; Function Attrs: nofree norecurse nounwind uwtable
define dso_local void @foo(i32 %n, %struct.IntPair* noalias %Pair) local_unnamed_addr #0 {
entry:
  %cmp22 = icmp sgt i32 %n, 0
  %first.dom = getelementptr inbounds %struct.IntPair, %struct.IntPair* %Pair, i64 0, i32 0
  %firs.dom.load = load i32, i32* %first.dom, align 4
  br i1 %cmp22, label %for.body.lr.ph, label %for.end9

for.body.lr.ph:                                   ; preds = %entry
  %first = getelementptr inbounds %struct.IntPair, %struct.IntPair* %Pair, i64 0, i32 0
  %wide.trip.count2729 = zext i32 %n to i64
  br label %for.body3.lr.ph

for.body3.lr.ph:                                  ; preds = %for.body.lr.ph, %for.inc7
  %indvars.iv25 = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next26, %for.inc7 ]
  tail call void @bar(%struct.IntPair* %Pair) #1
  br label %for.body3

for.body3:                                        ; preds = %for.inc, %for.body3.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body3.lr.ph ], [ %indvars.iv.next, %for.inc ]
  %cmp4 = icmp ult i64 %indvars.iv25, %indvars.iv
  br i1 %cmp4, label %if.then, label %for.inc

if.then:                                          ; preds = %for.body3
  %first.load = load i32, i32* %first, align 4
  %arrayidx6 = getelementptr inbounds [10 x [10 x i32]], [10 x [10 x i32]]* @A, i64 0, i64 %indvars.iv25, i64 %indvars.iv
  %load = load i32, i32* %arrayidx6, align 4
  %add = add nsw i32 %load, %first.load
  store i32 %add, i32* %arrayidx6, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body3, %if.then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count2729
  br i1 %exitcond, label %for.inc7, label %for.body3

for.inc7:                                         ; preds = %for.inc
  %indvars.iv.next26 = add nuw nsw i64 %indvars.iv25, 1
  %exitcond28 = icmp eq i64 %indvars.iv.next26, %wide.trip.count2729
  br i1 %exitcond28, label %for.end9.loopexit, label %for.body3.lr.ph

for.end9.loopexit:                                ; preds = %for.inc7
  br label %for.end9

for.end9:                                         ; preds = %for.end9.loopexit, %entry
  ret void
}

; end INTEL_FEATURE_SW_ADVANCED
