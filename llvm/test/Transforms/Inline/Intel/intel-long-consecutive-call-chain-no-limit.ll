; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced, asserts
; Checks that long consecutive call chain heuristic doesn't crash with
; -inline-agg-long-consecutive-call-chain-size set to 0.
; RUN: opt < %s -whole-program-assume -passes='module(agginliner),cgscc(inline)' -debug-only=agginliner -inline-report=0xe807 -S -inline-agg-long-consecutive-call-chain-ignore-profitability=true -inline-agg-long-consecutive-call-chain-size=0 2>&1 | FileCheck %s --check-prefixes=CHECK
; RUN: opt < %s -whole-program-assume -passes='inlinereportsetup,module(agginliner),cgscc(inline),inlinereportemitter' -debug-only=agginliner -inline-report=0xe886 -S -inline-agg-long-consecutive-call-chain-ignore-profitability=true -inline-agg-long-consecutive-call-chain-size=0 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-MD-INLREP

; CHECK: AggInl: LongConsecutiveCallChainHeuristic
; CHECK: AggInl: long consecutive call chain located inside call_leaf, total length 1, the longest consecutive chain has 1 calls to leaf, chain starts from   tail call void @leaf(

; CHECK-MD-INLREP: Begin Inlining Report{{.*}}(via metadata)
; CHECK: COMPILE FUNC: call_leaf
; CHECK-NOT: INLINE: leaf {{.*}}Aggressive inline
; CHECK-MD-INLREP: End Inlining Report{{.*}}(via metadata)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [21 x i8] c"Break the call chain\00", align 1

define void @leaf(ptr nocapture noundef %p, i32 noundef %n) #0 {
entry:
  %cmp3 = icmp sgt i32 %n, 0
  br i1 %cmp3, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:
  %wide.trip.count = zext i32 %n to i64
  br label %for.body

for.cond.cleanup:
  ret void

for.body:
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds i32, ptr %p, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx, align 4
  %add = add nsw i32 %0, 1
  store i32 %add, ptr %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body
}

define dso_local void @call_leaf(ptr nocapture noundef %p, i32 noundef %n, ptr nocapture noundef %q, i32 noundef %m) #2 {
  tail call void @leaf(ptr noundef %p, i32 noundef %n)
  ret void
}

declare dso_local noundef i32 @puts(ptr nocapture noundef readonly) #3

attributes #0 = { nofree norecurse nosync nounwind memory(argmem: readwrite) uwtable }
attributes #1 = { nofree nosync nounwind memory(argmem: readwrite) uwtable }
attributes #2 = { nofree nounwind uwtable }
attributes #3 = { nofree nounwind }
; end INTEL_FEATURE_SW_ADVANCED
