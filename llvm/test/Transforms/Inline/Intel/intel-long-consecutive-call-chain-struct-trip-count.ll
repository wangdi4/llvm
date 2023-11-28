; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced, asserts
; RUN: opt < %s -whole-program-assume -passes='module(agginliner),cgscc(inline)' -debug-only=agginliner -inline-report=0xe807 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-INLREP
; RUN: opt < %s -whole-program-assume -passes='inlinereportsetup,module(agginliner),cgscc(inline),inlinereportemitter' -debug-only=agginliner -inline-report=0xe886 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-MD-INLREP
; RUN: opt < %s -whole-program-assume -passes='module(agginliner)' -debug-only=agginliner -S 2>&1 | FileCheck %s --check-prefix=NO-INLINE-NONPROFITABLE

; Ensure trip count analysis of long consecutive call chain heuristic works
; when the trip count is fetched from a struct field.

; CHECK: AggInl: LongConsecutiveCallChainHeuristic
; CHECK-DAG: AggInl: Inserting:   tail call void @leaf_1(ptr noundef nonnull %array)
; CHECK-DAG: AggInl: Inserting:   tail call void @leaf_2(ptr noundef nonnull %array)
; CHECK-DAG: AggInl: Inserting:   tail call void @leaf_2(ptr noundef nonnull %array)
; CHECK-DAG: AggInl: Inserting:   tail call void @leaf_2(ptr noundef nonnull %array)
; CHECK-DAG: AggInl: Inserting:   tail call void @leaf_2(ptr noundef nonnull %array)
; CHECK-DAG: AggInl: Inserting:   tail call void @leaf_2(ptr noundef nonnull %array)
; CHECK-DAG: AggInl: Inserting:   tail call void @leaf_2(ptr noundef nonnull %array)
; CHECK-DAG: AggInl: Inserting:   tail call void @leaf_1(ptr noundef nonnull %array)
; CHECK-DAG: AggInl: Inserting:   tail call void @leaf_2(ptr noundef nonnull %array)
; CHECK-DAG: AggInl: Inserting:   tail call void @leaf_different_field(ptr noundef nonnull %array, i32 0)
; CHECK-DAG: AggInl: Inserting:   tail call void @leaf_different_field(ptr noundef nonnull %array, i32 0)
; CHECK-DAG: AggInl: Inserting:   tail call void @leaf_different_field(ptr noundef nonnull %array, i32 0)
; CHECK-DAG: AggInl: Inserting:   tail call void @leaf_different_field(ptr noundef nonnull %array, i32 0)
; CHECK-DAG: AggInl: Inserting:   tail call void @leaf_different_field(ptr noundef nonnull %array, i32 0)
; CHECK-DAG: AggInl: Inserting:   tail call void @leaf_different_field(ptr noundef nonnull %array, i32 0)
; CHECK-DAG: AggInl: Inserting:   tail call void @leaf_different_field(ptr noundef nonnull %array, i32 0)
; CHECK-DAG: AggInl: Inserting:   tail call void @leaf_1(ptr noundef nonnull %array)
; CHECK-DAG: AggInl: Inserting:   tail call void @leaf_2(ptr noundef nonnull %array)
; CHECK-DAG: AggInl: Inserting:   tail call void @leaf_2(ptr noundef nonnull %array)

; NO-INLINE-NONPROFITABLE-NOT: AggInl: Inserting:   tail call void @leaf_different_field(ptr noundef nonnull %array, i32 1)

; CHECK-NOT: call void @leaf_1(
; CHECK-NOT: call void @leaf_2(

; CHECK-MD-INLREP: Begin Inlining Report{{.*}}(via metadata)

; CHECK-MD-INLREP: COMPILE FUNC: call_leaves_1
; CHECK-MD-INLREP: INLINE: leaf_1 {{.*}}Aggressive inline
; CHECK-MD-INLREP: INLINE: leaf_2 {{.*}}Aggressive inline
; CHECK-MD-INLREP: INLINE: leaf_2 {{.*}}Aggressive inline
; CHECK-MD-INLREP: INLINE: leaf_2 {{.*}}Aggressive inline
; CHECK-MD-INLREP: INLINE: leaf_2 {{.*}}Aggressive inline
; CHECK-MD-INLREP: INLINE: leaf_2 {{.*}}Aggressive inline
; CHECK-MD-INLREP: INLINE: leaf_2 {{.*}}Aggressive inline
; CHECK-MD-INLREP: INLINE: leaf_1 {{.*}}Aggressive inline
; CHECK-MD-INLREP: INLINE: leaf_2 {{.*}}Aggressive inline
; CHECK-MD-INLREP-NOT: INLINE: leaf_different_field {{.*}}Aggressive inline

; CHECK-MD-INLREP: COMPILE FUNC: call_leaves_2
; CHECK-MD-INLREP: INLINE: leaf_different_field {{.*}}Aggressive inline
; CHECK-MD-INLREP: INLINE: leaf_different_field {{.*}}Aggressive inline
; CHECK-MD-INLREP: INLINE: leaf_different_field {{.*}}Aggressive inline
; CHECK-MD-INLREP: INLINE: leaf_different_field {{.*}}Aggressive inline
; CHECK-MD-INLREP: INLINE: leaf_different_field {{.*}}Aggressive inline
; CHECK-MD-INLREP: INLINE: leaf_different_field {{.*}}Aggressive inline
; CHECK-MD-INLREP: INLINE: leaf_different_field {{.*}}Aggressive inline
; CHECK-MD-INLREP: INLINE: leaf_1 {{.*}}Aggressive inline
; CHECK-MD-INLREP: INLINE: leaf_2 {{.*}}Aggressive inline
; CHECK-MD-INLREP: INLINE: leaf_2 {{.*}}Aggressive inline

; CHECK-MD-INLREP: End Inlining Report{{.*}}(via metadata)

; CHECK-INLREP: Begin Inlining Report

; CHECK-INLREP: COMPILE FUNC: call_leaves_1
; CHECK-INLREP: INLINE: leaf_1 {{.*}}Aggressive inline
; CHECK-INLREP: INLINE: leaf_2 {{.*}}Aggressive inline
; CHECK-INLREP: INLINE: leaf_2 {{.*}}Aggressive inline
; CHECK-INLREP: INLINE: leaf_2 {{.*}}Aggressive inline
; CHECK-INLREP: INLINE: leaf_2 {{.*}}Aggressive inline
; CHECK-INLREP: INLINE: leaf_2 {{.*}}Aggressive inline
; CHECK-INLREP: INLINE: leaf_2 {{.*}}Aggressive inline
; CHECK-INLREP: INLINE: leaf_1 {{.*}}Aggressive inline
; CHECK-INLREP: INLINE: leaf_2 {{.*}}Aggressive inline
; CHECK-INLREP-NOT: INLINE: leaf_different_field {{.*}}Aggressive inline

; CHECK-INLREP: COMPILE FUNC: call_leaves_2
; CHECK-INLREP: INLINE: leaf_different_field {{.*}}Aggressive inline
; CHECK-INLREP: INLINE: leaf_different_field {{.*}}Aggressive inline
; CHECK-INLREP: INLINE: leaf_different_field {{.*}}Aggressive inline
; CHECK-INLREP: INLINE: leaf_different_field {{.*}}Aggressive inline
; CHECK-INLREP: INLINE: leaf_different_field {{.*}}Aggressive inline
; CHECK-INLREP: INLINE: leaf_different_field {{.*}}Aggressive inline
; CHECK-INLREP: INLINE: leaf_different_field {{.*}}Aggressive inline
; CHECK-INLREP: INLINE: leaf_1 {{.*}}Aggressive inline
; CHECK-INLREP: INLINE: leaf_2 {{.*}}Aggressive inline
; CHECK-INLREP: INLINE: leaf_2 {{.*}}Aggressive inline

; CHECK-INLREP: End Inlining Report

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.array_t = type { i16, i16, ptr }

define void @leaf_1(ptr nocapture noundef readonly %array) #0 {
entry:
  %size = getelementptr inbounds %struct.array_t, ptr %array, i64 0, i32 0
  %0 = load i16, ptr %size, align 8
  %cmp5 = icmp sgt i16 %0, 0
  br i1 %cmp5, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:
  %data = getelementptr inbounds %struct.array_t, ptr %array, i64 0, i32 2
  %1 = load ptr, ptr %data, align 8
  %wide.trip.count = zext i16 %0 to i64
  br label %for.body

for.cond.cleanup:
  ret void

for.body:
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds i32, ptr %1, i64 %indvars.iv
  %2 = load i32, ptr %arrayidx, align 4
  %add = add nsw i32 %2, 1
  store i32 %add, ptr %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body
}

define void @leaf_2(ptr nocapture noundef readonly %array) #0 {
entry:
  %size = getelementptr inbounds %struct.array_t, ptr %array, i64 0, i32 0
  %0 = load i16, ptr %size, align 8
  %cmp5 = icmp sgt i16 %0, 0
  br i1 %cmp5, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:
  %data = getelementptr inbounds %struct.array_t, ptr %array, i64 0, i32 2
  %1 = load ptr, ptr %data, align 8
  %wide.trip.count = zext i16 %0 to i64
  br label %for.body

for.cond.cleanup:
  ret void

for.body:
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds i32, ptr %1, i64 %indvars.iv
  %2 = load i32, ptr %arrayidx, align 4
  %add = add nsw i32 %2, 2
  store i32 %add, ptr %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body
}

define void @leaf_different_field(ptr nocapture noundef readonly %array, i32 %placeholder) #1 {
entry:
  %capacity = getelementptr inbounds %struct.array_t, ptr %array, i64 0, i32 1
  %0 = load i16, ptr %capacity, align 2
  %cmp5 = icmp sgt i16 %0, 0
  br i1 %cmp5, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:
  %data = getelementptr inbounds %struct.array_t, ptr %array, i64 0, i32 2
  %1 = load ptr, ptr %data, align 8
  %wide.trip.count = zext i16 %0 to i64
  br label %for.body

for.cond.cleanup:
  ret void

for.body:
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds i32, ptr %1, i64 %indvars.iv
  store i32 0, ptr %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body
}

define void @call_leaves_1(ptr nocapture noundef readonly %array) #2 {
entry:
  %size = getelementptr inbounds %struct.array_t, ptr %array, i64 0, i32 0
  %0 = load i16, ptr %size, align 8
  %cmp = icmp sgt i16 %0, 16
  br i1 %cmp, label %if.then, label %if.end

if.then:
  tail call void @leaf_1(ptr noundef nonnull %array)
  tail call void @leaf_2(ptr noundef nonnull %array)
  tail call void @leaf_2(ptr noundef nonnull %array)
  tail call void @leaf_2(ptr noundef nonnull %array)
  tail call void @leaf_2(ptr noundef nonnull %array)
  tail call void @leaf_2(ptr noundef nonnull %array)
  tail call void @leaf_2(ptr noundef nonnull %array)
  br label %if.end

if.end:
  tail call void @leaf_1(ptr noundef nonnull %array)
  tail call void @leaf_2(ptr noundef nonnull %array)
  ; This call wouldn't be marked for inline because the trip count of its loop
  ; is different from the previous one.
  tail call void @leaf_different_field(ptr noundef nonnull %array, i32 1)
  ret void
}

define void @call_leaves_2(ptr nocapture noundef readonly %array) #2 {
entry:
  %size = getelementptr inbounds %struct.array_t, ptr %array, i64 0, i32 0
  %0 = load i16, ptr %size, align 8
  %cmp = icmp sgt i16 %0, 16
  br i1 %cmp, label %if.then, label %if.end

if.then:
  tail call void @leaf_different_field(ptr noundef nonnull %array, i32 0)
  tail call void @leaf_different_field(ptr noundef nonnull %array, i32 0)
  tail call void @leaf_different_field(ptr noundef nonnull %array, i32 0)
  tail call void @leaf_different_field(ptr noundef nonnull %array, i32 0)
  tail call void @leaf_different_field(ptr noundef nonnull %array, i32 0)
  tail call void @leaf_different_field(ptr noundef nonnull %array, i32 0)
  tail call void @leaf_different_field(ptr noundef nonnull %array, i32 0)
  br label %if.end

if.end:
  tail call void @leaf_1(ptr noundef nonnull %array)
  tail call void @leaf_2(ptr noundef nonnull %array)
  tail call void @leaf_2(ptr noundef nonnull %array)
  ret void
}

attributes #0 = { nofree norecurse nosync nounwind memory(readwrite, inaccessiblemem: none) uwtable }
attributes #1 = { nofree norecurse nosync nounwind memory(write, argmem: readwrite, inaccessiblemem: none) uwtable }
attributes #2 = { nofree norecurse nosync nounwind memory(readwrite, inaccessiblemem: none) uwtable }
; end INTEL_FEATURE_SW_ADVANCED
