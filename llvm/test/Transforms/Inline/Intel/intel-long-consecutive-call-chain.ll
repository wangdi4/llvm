; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced, asserts
; Ignore profitability check because it's not implemented yet.
; RUN: opt < %s -whole-program-assume -passes='module(agginliner),cgscc(inline)' -debug-only=agginliner -inline-report=0xe807 -S -inline-agg-long-consecutive-call-chain-ignore-profitability=true 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-INLREP
; RUN: opt < %s -whole-program-assume -passes='inlinereportsetup,module(agginliner),cgscc(inline),inlinereportemitter' -debug-only=agginliner -inline-report=0xe886 -S -inline-agg-long-consecutive-call-chain-ignore-profitability=true 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-MD-INLREP

; Check trace to test AggInliner's long-consecutive-call-chain heuristic.

; CHECK: AggInl: LongConsecutiveCallChainHeuristic
; CHECK-DAG: AggInl: Inserting:   tail call void @leaf_3(ptr noundef %p, i32 noundef %n)
; CHECK-DAG: AggInl: Inserting:   tail call void @leaf_2(ptr noundef %p, i32 noundef %n)
; CHECK-DAG: AggInl: Inserting:   tail call void @leaf_1(ptr noundef %p, i32 noundef %n)
; CHECK-DAG: AggInl: Inserting:   tail call void @leaf_2(ptr noundef %p, i32 noundef %n)
; CHECK-DAG: AggInl: Inserting:   tail call void @leaf_3(ptr noundef %p, i32 noundef %n)
; CHECK-DAG: AggInl: Inserting:   tail call void @leaf_2(ptr noundef %p, i32 noundef %n)
; CHECK-DAG: AggInl: Inserting:   tail call void @leaf_1(ptr noundef %p, i32 noundef %n)
; CHECK-DAG: AggInl: Inserting:   tail call void @leaf_2(ptr noundef %p, i32 noundef %n)
; CHECK-DAG: AggInl: Inserting:   tail call void @leaf_2(ptr noundef %p, i32 noundef %n)
; CHECK-DAG: AggInl: Inserting:   tail call void @leaf_2(ptr noundef %p, i32 noundef %n)
; CHECK-DAG: AggInl: Inserting:   tail call void @leaf_2(ptr noundef %p, i32 noundef %n)
; CHECK-DAG: AggInl: Inserting:   tail call void @leaf_3(ptr noundef %p, i32 noundef %n)
; CHECK-DAG: AggInl: Inserting:   tail call void @leaf_2(ptr noundef %p, i32 noundef %n)
; CHECK-DAG: AggInl: Inserting:   tail call void @leaf_2(ptr noundef %p, i32 noundef %n)
; CHECK-DAG: AggInl: Inserting:   tail call void @leaf_2(ptr noundef %p, i32 noundef %n)
; CHECK-DAG: AggInl: Inserting:   tail call void @leaf_2(ptr noundef %q, i32 noundef %m)

; CHECK-MD-INLREP: Begin Inlining Report{{.*}}(via metadata)
; CHECK-MD-INLREP: COMPILE FUNC: leaf_3
; CHECK-MD-INLREP: recursive_call_leaf_3 {{.*}}Callsite is noinline

; CHECK-MD-INLREP: COMPILE FUNC: call_leaves_1
; CHECK-MD-INLREP: INLINE: leaf_1 {{.*}}Aggressive inline
; CHECK-MD-INLREP: INLINE: leaf_2 {{.*}}Aggressive inline
; CHECK-MD-INLREP: INLINE: leaf_2 {{.*}}Aggressive inline
; CHECK-MD-INLREP: INLINE: leaf_2 {{.*}}Aggressive inline
; CHECK-MD-INLREP: INLINE: leaf_2 {{.*}}Aggressive inline
; CHECK-MD-INLREP: INLINE: leaf_2 {{.*}}Aggressive inline
; CHECK-MD-INLREP: INLINE: leaf_2 {{.*}}Aggressive inline
; CHECK-MD-INLREP: INLINE: leaf_3 {{.*}}Aggressive inline
; CHECK-MD-INLREP: -> recursive_call_leaf_3 {{.*}}Callsite is noinline
; CHECK-MD-INLREP: INLINE: leaf_1 {{.*}}Aggressive inline
; CHECK-MD-INLREP: INLINE: leaf_2 {{.*}}Aggressive inline
; CHECK-MD-INLREP: INLINE: leaf_2 {{.*}}Aggressive inline

; CHECK-MD-INLREP: COMPILE FUNC: call_leaves_2
; CHECK-MD-INLREP: INLINE: leaf_2 {{.*}}Aggressive inline
; CHECK-MD-INLREP: INLINE: leaf_2 {{.*}}Aggressive inline
; CHECK-MD-INLREP: INLINE: leaf_3 {{.*}}Aggressive inline
; CHECK-MD-INLREP: -> recursive_call_leaf_3 {{.*}}Callsite is noinline
; CHECK-MD-INLREP: INLINE: leaf_3 {{.*}}Aggressive inline
; CHECK-MD-INLREP: -> recursive_call_leaf_3 {{.*}}Callsite is noinline
; CHECK-MD-INLREP: INLINE: leaf_2 {{.*}}Aggressive inline
; CHECK-MD-INLREP: End Inlining Report{{.*}}(via metadata)

; The call to recursive_call_leaf_3 can't be inlined since it'll make leaf_3 recursive
; CHECK-LABEL: define{{.*}}@leaf_3
; CHECK: tail call void @recursive_call_leaf_3({{.*}}) [[ATTR_NOINLINE:#[0-9]+]]
; CHECK: attributes [[ATTR_NOINLINE]] = {{{.*}} noinline

; CHECK-NOT: call void @leaf_1(ptr noundef %p, i32 noundef %n)
; CHECK-NOT: call void @leaf_2(ptr noundef %p, i32 noundef %n)
; CHECK-NOT: call void @leaf_3(ptr noundef %p, i32 noundef %n)

; CHECK-INLREP: Begin Inlining Report
; CHECK-INLREP: COMPILE FUNC: leaf_3
; CHECK-INLREP: recursive_call_leaf_3 {{.*}}Callsite is noinline

; CHECK-INLREP: COMPILE FUNC: call_leaves_1
; CHECK-INLREP: INLINE: leaf_1 {{.*}}Aggressive inline
; CHECK-INLREP: INLINE: leaf_2 {{.*}}Aggressive inline
; CHECK-INLREP: INLINE: leaf_2 {{.*}}Aggressive inline
; CHECK-INLREP: INLINE: leaf_2 {{.*}}Aggressive inline
; CHECK-INLREP: INLINE: leaf_2 {{.*}}Aggressive inline
; CHECK-INLREP: INLINE: leaf_2 {{.*}}Aggressive inline
; CHECK-INLREP: INLINE: leaf_2 {{.*}}Aggressive inline
; CHECK-INLREP: INLINE: leaf_3 {{.*}}Aggressive inline
; CHECK-INLREP: -> recursive_call_leaf_3 {{.*}}Callsite is noinline
; CHECK-INLREP: INLINE: leaf_1 {{.*}}Aggressive inline
; CHECK-INLREP: INLINE: leaf_2 {{.*}}Aggressive inline
; CHECK-INLREP: INLINE: leaf_2 {{.*}}Aggressive inline

; CHECK-INLREP: COMPILE FUNC: call_leaves_2
; CHECK-INLREP: INLINE: leaf_2 {{.*}}Aggressive inline
; CHECK-INLREP: INLINE: leaf_2 {{.*}}Aggressive inline
; CHECK-INLREP: INLINE: leaf_3 {{.*}}Aggressive inline
; CHECK-INLREP: -> recursive_call_leaf_3 {{.*}}Callsite is noinline
; CHECK-INLREP: INLINE: leaf_3 {{.*}}Aggressive inline
; CHECK-INLREP: -> recursive_call_leaf_3 {{.*}}Callsite is noinline
; CHECK-INLREP: INLINE: leaf_2 {{.*}}Aggressive inline
; CHECK-INLREP: End Inlining Report

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [21 x i8] c"Break the call chain\00", align 1

define void @leaf_1(ptr nocapture noundef %p, i32 noundef %n) #0 {
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

define void @leaf_2(ptr nocapture noundef %p, i32 noundef %n) #0 {
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
  %add = add nsw i32 %0, 2
  store i32 %add, ptr %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body
}

define void @recursive_call_leaf_3(ptr nocapture noundef %p, i32 noundef %n) #1 {
entry:
  %0 = load i32, ptr %p, align 4
  %cmp = icmp slt i32 %0, 10
  br i1 %cmp, label %if.then, label %if.end

if.then:
  tail call void @leaf_3(ptr noundef nonnull %p, i32 noundef %n)
  br label %if.end

if.end:
  ret void
}

define void @leaf_3(ptr nocapture noundef %p, i32 noundef %n) #1 {
entry:
  tail call void @recursive_call_leaf_3(ptr noundef %p, i32 noundef %n)
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
  %add = add nsw i32 %0, 3
  store i32 %add, ptr %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body
}

define void @not_leaf(ptr nocapture noundef %p, i32 noundef %n) #0 {
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
  %add = add nsw i32 %0, 4
  store i32 %add, ptr %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body
}

define dso_local void @call_leaves_1(ptr nocapture noundef %p, i32 noundef %n, ptr nocapture noundef %q, i32 noundef %m) #2 {
entry:
  %cmp = icmp sgt i32 %n, 16
  br i1 %cmp, label %if.then, label %if.end

if.then:
  tail call void @leaf_1(ptr noundef %p, i32 noundef %n)
  tail call void @leaf_2(ptr noundef %p, i32 noundef %n)
  tail call void @leaf_2(ptr noundef %p, i32 noundef %n)
  tail call void @leaf_2(ptr noundef %p, i32 noundef %n)
  tail call void @leaf_2(ptr noundef %p, i32 noundef %n)
  tail call void @leaf_2(ptr noundef %p, i32 noundef %n)
  tail call void @leaf_2(ptr noundef %p, i32 noundef %n)
  tail call void @leaf_3(ptr noundef %p, i32 noundef %n)
  %call = tail call i32 @puts(ptr noundef nonnull dereferenceable(1) @.str)
  tail call void @not_leaf(ptr noundef %p, i32 noundef %n)
  br label %if.end

if.end:
  tail call void @leaf_1(ptr noundef %p, i32 noundef %n)
  tail call void @leaf_2(ptr noundef %p, i32 noundef %n)
  ; This call will not be inlined because the trip count of its loop doesn't match with that of the previous call
  tail call void @leaf_2(ptr noundef %q, i32 noundef %m)
  ret void
}

; These calls are not long consecutive call chains but will also be inlined
define dso_local void @call_leaves_2(ptr nocapture noundef %p, i32 noundef %n) #2 {
entry:
  %cmp = icmp sgt i32 %n, 16
  tail call void @leaf_2(ptr noundef %p, i32 noundef %n)
  br i1 %cmp, label %if.then, label %if.end

if.then:
  tail call void @leaf_2(ptr noundef %p, i32 noundef %n)
  tail call void @leaf_3(ptr noundef %p, i32 noundef %n)
  %call = tail call i32 @puts(ptr noundef nonnull dereferenceable(1) @.str)
  tail call void @not_leaf(ptr noundef %p, i32 noundef %n)
  br label %if.end

if.end:
  tail call void @leaf_3(ptr noundef %p, i32 noundef %n)
  tail call void @leaf_2(ptr noundef %p, i32 noundef %n)
  ret void
}

declare dso_local noundef i32 @puts(ptr nocapture noundef readonly) #3

attributes #0 = { nofree norecurse nosync nounwind memory(argmem: readwrite) uwtable }
attributes #1 = { nofree nosync nounwind memory(argmem: readwrite) uwtable }
attributes #2 = { nofree nounwind uwtable }
attributes #3 = { nofree nounwind }
; end INTEL_FEATURE_SW_ADVANCED
