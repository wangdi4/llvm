; INTEL_FEATURE_SW_ADVANCED
; This test originates from ip_cloning_spec_4.ll, but its purpose is to verify
; if the cloning candidate's call site has the correct number of actual
; parameters. It is crucial to ensure that optimization doesn't receive
; incorrect input IR when the count of actual parameters is less than the
; count of formal parameters. In such situations, the optimization should
; disregard the call site.

; REQUIRES: intel_feature_sw_advanced,asserts
; RUN: opt < %s -passes='module(ip-cloning)' -ip-specialization-cloning -debug-only=ipcloning  -disable-output  2>&1 | FileCheck %s

; CHECK: Cloning Analysis for:  foo
; CHECK:    Selected Specialization cloning  
; CHECK: Processing for Spe cloning  foo
; CHECK: Skipping non-candidate foo
; CHECK: Total clones:  0

@Val = external local_unnamed_addr global i32, align 4
@a = external local_unnamed_addr global [2056 x i32], align 16
@b = external local_unnamed_addr global [2056 x i32], align 16
@c = external local_unnamed_addr global [2056 x i32], align 16

; Function Attrs: norecurse nounwind uwtable
define void @bar() local_unnamed_addr  {
entry:
  %0 = load i32, ptr @Val, align 4
  switch i32 %0, label %sw.epilog [
    i32 100, label %sw.bb
    i32 200, label %sw.bb1
    i32 300, label %sw.bb2
  ]

sw.bb:                                            ; preds = %entry
  br label %sw.epilog

sw.bb1:                                           ; preds = %entry
  br label %sw.epilog

sw.bb2:                                           ; preds = %entry
  br label %sw.epilog

sw.epilog:                                        ; preds = %entry, %sw.bb2, %sw.bb1, %sw.bb
  %TripC.0 = phi i32 [ 0, %entry ], [ 64, %sw.bb2 ], [ 32, %sw.bb1 ], [ 16, %sw.bb ]
  tail call fastcc void @foo(i32 %TripC.0)
  ret void
}

; Function Attrs: noinline norecurse nounwind uwtable
define internal fastcc void @foo(i32 %UB, i32 %dummy) unnamed_addr  {
entry:
  %cmp20 = icmp eq i32 %UB, 0
  br i1 %cmp20, label %for.cond.cleanup, label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.cond.cleanup3
  %j.021 = phi i32 [ %inc10, %for.cond.cleanup3 ], [ 0, %entry ]
  br label %for.body4

for.cond.cleanup:                                 ; preds = %for.cond.cleanup3, %entry
  ret void

for.cond.cleanup3:                                ; preds = %for.body4
  %inc10 = add nuw i32 %j.021, 1
  %exitcond22 = icmp eq i32 %inc10, %UB
  br i1 %exitcond22, label %for.cond.cleanup, label %for.cond1.preheader

for.body4:                                        ; preds = %for.body4, %for.cond1.preheader
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body4 ]
  %arrayidx = getelementptr inbounds [2056 x i32], ptr @a, i64 0, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx, align 4
  %arrayidx6 = getelementptr inbounds [2056 x i32], ptr @b, i64 0, i64 %indvars.iv
  %1 = load i32, ptr %arrayidx6, align 4
  %mul = mul nsw i32 %1, %0
  %arrayidx8 = getelementptr inbounds [2056 x i32], ptr @c, i64 0, i64 %indvars.iv
  %2 = load i32, ptr %arrayidx8, align 4
  %add = add nsw i32 %2, %mul
  store i32 %add, ptr %arrayidx8, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 2056
  br i1 %exitcond, label %for.cond.cleanup3, label %for.body4
}
; end INTEL_FEATURE_SW_ADVANCED
