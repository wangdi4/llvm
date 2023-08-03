; RUN: opt -enable-lv -passes=loop-vectorize -S --debug-only=loop-vectorize < %s 2>&1 | FileCheck %s

; CHECK: LV: Not vectorizing: Found an unidentified PHI [[STORE:%.*]] = phi ptr [ [[ADD_PTR:%.*]], [[FOR_BODY:%.*]] ], [ [[LOAD_INITIAL:%.*]], [[FOR_BODY_PREHEADER:%.*]] ]
; CHECK: LV: Not vectorizing: Cannot prove legality.
; CHECK-NOT: LV: Found an induction variable.
; CHECK-NOT: LV: We can vectorize this loop!

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree nounwind uwtable
define dso_local noalias ptr @alloc_matrix(ptr %0, i32 noundef %m) local_unnamed_addr #0 {
entry:
  %load_initial = load ptr, ptr %0, align 8
  %cmp18 = icmp sgt i32 %m, 1
  br i1 %cmp18, label %for.body, label %for.end

for.body:                                         ; preds = %entry, %for.body
  %store_forwarded = phi ptr [ %load_initial, %entry ], [ %add.ptr, %for.body ]
  %indvars.iv = phi i64 [ 1, %entry ], [ %indvars.iv.next, %for.body ]
  %idx.ext = sext i32 %m to i64
  %add.ptr = getelementptr inbounds double, ptr %store_forwarded, i64 %idx.ext
  %arrayidx6 = getelementptr inbounds ptr, ptr %0, i64 %indvars.iv
  store ptr %add.ptr, ptr %arrayidx6, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %idx.ext
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:                                          ; preds = %for.body, %entry
  ret ptr %0
}
