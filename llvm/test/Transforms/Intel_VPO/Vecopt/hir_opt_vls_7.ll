; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -VPlanDriverHIR -vplan-force-vf=4 -disable-output -print-after=VPlanDriverHIR  < %s 2>&1  | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,vplan-driver-hir" -vplan-force-vf=4 -disable-output -print-after=vplan-driver-hir < %s 2>&1 | FileCheck %s

;
; Test to check that VLS optimized code generation is suppressed in HIR
; mixed vector code generation mode. Currently we default to mixed CG mode
; when we have a liveout private while work is in progress to make its
; representation explicit. VLS support in mixed CG mode is being disabled
; to allow VLS analysis enhancements which are only expected to work with
; VPValue based code generation. The test uses two functions(f_noliveout and
; f_liveout) with essentially the following loop:
;
;            + DO i1 = 0, 99, 1   <DO_LOOP>
;            |   %0 = (@arr)[0][2 * i1];
;            |   (@arr2)[0][2 * i1] = %0;
;            |   %1 = (@arr)[0][2 * i1 + 1];
;            |   (@arr2)[0][2 * i1 + 1] = %1;
;            + END LOOP
;
; %1 is only liveout in the loop in function f_liveout. As a result, we expect
; VLS optimized CG for the loop in the function f_noliveout and gathers/scatters
; for the loop in the function f_liveout. The test will need to be updated
; once liveout privates are represented explicitly and when we stop defaulting
; to mixed CG mode.
;
; CHECK-LABEL: IR Dump After{{.+}}VPlan{{.*}}Driver{{.*}}HIR{{.*}}
; CHECK:       Function: f_noliveout
; CHECK:         BEGIN REGION { modified }
; CHECK-NEXT:    + DO i1 = 0, 99, 4   <DO_LOOP> <auto-vectorized> <novectorize>
; CHECK-NEXT:    |   %.vls.load = (<8 x i64>*)(@arr)[0][2 * i1];
; CHECK-NEXT:    |   %vls.shuf = shufflevector %.vls.load,  undef,  <i32 0, i32 2, i32 4, i32 6>;
; CHECK-NEXT:    |   %vls.shuf1 = shufflevector %.vls.load,  undef,  <i32 1, i32 3, i32 5, i32 7>;
; CHECK-NEXT:    |   %comb.shuf = shufflevector %vls.shuf,  %vls.shuf1,  <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>;
; CHECK-NEXT:    |   %vls.interleave = shufflevector %comb.shuf,  undef,  <i32 0, i32 4, i32 1, i32 5, i32 2, i32 6, i32 3, i32 7>;
; CHECK-NEXT:    |   (<8 x i64>*)(@arr2)[0][2 * i1] = %vls.interleave;
; CHECK-NEXT:    + END LOOP
; CHECK-NEXT:    END REGION
;
; CHECK-LABEL: IR Dump After{{.+}}VPlan{{.*}}Driver{{.*}}HIR{{.*}} ***
; CHECK:       Function: f_liveout
; CHECK:         BEGIN REGION { modified }
; CHECK-NEXT:    + DO i1 = 0, 99, 4   <DO_LOOP> <auto-vectorized> <novectorize>
; CHECK-NEXT:    |   %.vec = (<4 x i64>*)(@arr)[0][2 * i1 + <i64 0, i64 2, i64 4, i64 6>];
; CHECK-NEXT:    |   (<4 x i64>*)(@arr2)[0][2 * i1 + <i64 0, i64 2, i64 4, i64 6>] = %.vec;
; CHECK-NEXT:    |   %.vec1 = (<4 x i64>*)(@arr)[0][2 * i1 + <i64 0, i64 2, i64 4, i64 6> + 1];
; CHECK-NEXT:    |   (<4 x i64>*)(@arr2)[0][2 * i1 + <i64 0, i64 2, i64 4, i64 6> + 1] = %.vec1;
; CHECK-NEXT:    + END LOOP
; CHECK:         %1 = extractelement %.vec1,  3;
; CHECK-NEXT:    END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@arr = dso_local local_unnamed_addr global [200 x i64] zeroinitializer, align 16
@arr2 = dso_local local_unnamed_addr global [200 x i64] zeroinitializer, align 16

; Function Attrs: nofree norecurse nounwind uwtable
define dso_local i64 @f_noliveout(i64* nocapture readnone %larr) local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret i64 0

for.body:                                         ; preds = %entry, %for.body
  %l11.017 = phi i64 [ 0, %entry ], [ %inc, %for.body ]
  %mul = shl nuw nsw i64 %l11.017, 1
  %arrayidx = getelementptr inbounds [200 x i64], [200 x i64]* @arr, i64 0, i64 %mul
  %0 = load i64, i64* %arrayidx, align 16
  %arrayidx3 = getelementptr inbounds [200 x i64], [200 x i64]* @arr2, i64 0, i64 %mul
  store i64 %0, i64* %arrayidx3, align 16
  %add = or i64 %mul, 1
  %arrayidx5 = getelementptr inbounds [200 x i64], [200 x i64]* @arr, i64 0, i64 %add
  %1 = load i64, i64* %arrayidx5, align 8
  %arrayidx8 = getelementptr inbounds [200 x i64], [200 x i64]* @arr2, i64 0, i64 %add
  store i64 %1, i64* %arrayidx8, align 8
  %inc = add nuw nsw i64 %l11.017, 1
  %exitcond.not = icmp eq i64 %inc, 100
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body
}

; Function Attrs: nofree norecurse nounwind uwtable
define dso_local i64 @f_liveout(i64* nocapture readnone %larr) local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  %.lcssa = phi i64 [ %1, %for.body ]
  ret i64 %.lcssa

for.body:                                         ; preds = %entry, %for.body
  %l11.018 = phi i64 [ 0, %entry ], [ %inc, %for.body ]
  %mul = shl nuw nsw i64 %l11.018, 1
  %arrayidx = getelementptr inbounds [200 x i64], [200 x i64]* @arr, i64 0, i64 %mul
  %0 = load i64, i64* %arrayidx, align 16
  %arrayidx3 = getelementptr inbounds [200 x i64], [200 x i64]* @arr2, i64 0, i64 %mul
  store i64 %0, i64* %arrayidx3, align 16
  %add = or i64 %mul, 1
  %arrayidx5 = getelementptr inbounds [200 x i64], [200 x i64]* @arr, i64 0, i64 %add
  %1 = load i64, i64* %arrayidx5, align 8
  %arrayidx8 = getelementptr inbounds [200 x i64], [200 x i64]* @arr2, i64 0, i64 %add
  store i64 %1, i64* %arrayidx8, align 8
  %inc = add nuw nsw i64 %l11.018, 1
  %exitcond.not = icmp eq i64 %inc, 100
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body
}
