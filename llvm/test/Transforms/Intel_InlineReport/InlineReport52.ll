; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; RUN: opt -enable-new-pm=0 -inline -inline-report=0xe807 -dtrans-inline-heuristics -intel-libirc-allowed -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 < %s -S 2>&1 | FileCheck --check-prefix=CHECK-OLD %s
; RUN: opt -passes='cgscc(inline)' -inline-report=0xe807 -dtrans-inline-heuristics -intel-libirc-allowed -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 < %s -S 2>&1 | FileCheck --check-prefix=CHECK-NEW %s
; RUN: opt -inlinereportsetup -inline-report=0xe886 < %s -S | opt -enable-new-pm=0 -inline -inline-report=0xe886 -dtrans-inline-heuristics -intel-libirc-allowed -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -S | opt -inlinereportemitter -inline-report=0xe886 -S 2>&1 | FileCheck %s --check-prefix=CHECK-NEW
; RUN: opt -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='cgscc(inline)' -inline-report=0xe886 -dtrans-inline-heuristics -intel-libirc-allowed -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -S 2>&1 | FileCheck %s --check-prefix=CHECK-NEW

; Check that foo1 meets all of the criteria for preferring multiversioning to inlining, while foo2 does not.

; CHECK-NEW: define{{.*}}@foo1
; CHECK-NEW: define{{.*}}@foo2
; CHECK-NEW: define{{.*}}@main
; CHECK-NEW: call{{.*}}@foo1
; CHECK-NEW-NOT: call{{.*}}@foo2

; CHECK: COMPILE FUNC: foo1
; CHECK: COMPILE FUNC: foo2
; CHECK: COMPILE FUNC: main
; CHECK: foo1{{.*}}Callsite preferred for multiversioning
; CHECK: INLINE: foo2

; CHECK-OLD: define{{.*}}@foo1
; CHECK-OLD: define{{.*}}@foo2
; CHECK-OLD: define{{.*}}@main
; CHECK-OLD: call{{.*}}@foo1
; CHECK-OLD-NOT: call{{.*}}@foo2

target triple = "x86_64-unknown-linux-gnu"

@a = dso_local global [100 x i32] zeroinitializer, align 16
@b = dso_local global [100 x i32] zeroinitializer, align 16
@tca = internal dso_local global i32 100, align 4
@tcb = internal dso_local global i32 100, align 4

define dso_local i32 @foo1(i32* nocapture %a, i32* nocapture readonly %b, i32 %tca, i32 %tcb) local_unnamed_addr {
entry:
  %and = and i32 %tca, 7
  %and1 = and i32 %tcb, 7
  %cmp29 = icmp eq i32 %and, 0
  br i1 %cmp29, label %for.cond.cleanup, label %for.cond2.preheader.lr.ph

for.cond2.preheader.lr.ph:                        ; preds = %entry
  %cmp327 = icmp eq i32 %and1, 0
  %wide.trip.count35 = zext i32 %and to i64
  %wide.trip.count = zext i32 %and1 to i64
  br label %for.cond2.preheader

for.cond2.preheader:                              ; preds = %for.cond.cleanup4, %for.cond2.preheader.lr.ph
  %indvars.iv33 = phi i64 [ 0, %for.cond2.preheader.lr.ph ], [ %indvars.iv.next34, %for.cond.cleanup4 ]
  br i1 %cmp327, label %for.cond.cleanup4, label %for.body5

for.cond.cleanup:                                 ; preds = %for.cond.cleanup4, %entry
  %0 = load i32, i32* %a, align 4
  ret i32 %0

for.cond.cleanup4:                                ; preds = %for.body5, %for.cond2.preheader
  %indvars.iv.next34 = add nuw nsw i64 %indvars.iv33, 1
  %exitcond36 = icmp eq i64 %indvars.iv.next34, %wide.trip.count35
  br i1 %exitcond36, label %for.cond.cleanup, label %for.cond2.preheader

for.body5:                                        ; preds = %for.cond2.preheader, %for.body5
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body5 ], [ 0, %for.cond2.preheader ]
  %1 = add nuw nsw i64 %indvars.iv, %indvars.iv33
  %arrayidx = getelementptr inbounds i32, i32* %b, i64 %1
  %2 = load i32, i32* %arrayidx, align 4
  %mul = mul nsw i32 %2, %and
  %add6 = add nsw i32 %mul, %and1
  %arrayidx9 = getelementptr inbounds i32, i32* %a, i64 %1
  store i32 %add6, i32* %arrayidx9, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup4, label %for.body5
}

define dso_local i32 @foo2(i32* nocapture %a, i32* nocapture readonly %b, i32 %tca, i32 %tcb) local_unnamed_addr {
entry:
  %and = and i32 %tca, 6
  %and1 = and i32 %tcb, 5
  %cmp29 = icmp eq i32 %and, 0
  br i1 %cmp29, label %for.cond.cleanup, label %for.cond2.preheader.lr.ph

for.cond2.preheader.lr.ph:                        ; preds = %entry
  %cmp327 = icmp eq i32 %and1, 0
  %wide.trip.count35 = zext i32 %and to i64
  %wide.trip.count = zext i32 %and1 to i64
  br label %for.cond2.preheader

for.cond2.preheader:                              ; preds = %for.cond.cleanup4, %for.cond2.preheader.lr.ph
  %indvars.iv33 = phi i64 [ 0, %for.cond2.preheader.lr.ph ], [ %indvars.iv.next34, %for.cond.cleanup4 ]
  br i1 %cmp327, label %for.cond.cleanup4, label %for.body5

for.cond.cleanup:                                 ; preds = %for.cond.cleanup4, %entry
  %0 = load i32, i32* %a, align 4
  ret i32 %0

for.cond.cleanup4:                                ; preds = %for.body5, %for.cond2.preheader
  %indvars.iv.next34 = add nuw nsw i64 %indvars.iv33, 1
  %exitcond36 = icmp eq i64 %indvars.iv.next34, %wide.trip.count35
  br i1 %exitcond36, label %for.cond.cleanup, label %for.cond2.preheader

for.body5:                                        ; preds = %for.cond2.preheader, %for.body5
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body5 ], [ 0, %for.cond2.preheader ]
  %1 = add nuw nsw i64 %indvars.iv, %indvars.iv33
  %arrayidx = getelementptr inbounds i32, i32* %b, i64 %1
  %2 = load i32, i32* %arrayidx, align 4
  %mul = mul nsw i32 %2, %and
  %add6 = add nsw i32 %mul, %and1
  %arrayidx9 = getelementptr inbounds i32, i32* %a, i64 %1
  store i32 %add6, i32* %arrayidx9, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup4, label %for.body5
}

define dso_local i32 @main() local_unnamed_addr {
entry:
  %0 = load i32, i32* @tca, align 4
  %1 = load i32, i32* @tcb, align 4
  %call1 = call i32 @foo1(i32* getelementptr inbounds ([100 x i32], [100 x i32]* @a, i64 0, i64 0), i32* getelementptr inbounds ([100 x i32], [100 x i32]* @b, i64 0, i64 0), i32 %0, i32 %1)
  %call2 = call i32 @foo2(i32* getelementptr inbounds ([100 x i32], [100 x i32]* @a, i64 0, i64 0), i32* getelementptr inbounds ([100 x i32], [100 x i32]* @b, i64 0, i64 0), i32 %0, i32 %1)
  %rv = add nsw i32 %call1, %call2
  ret i32 %rv
}

; end INTEL_FEATURE_SW_ADVANCED
