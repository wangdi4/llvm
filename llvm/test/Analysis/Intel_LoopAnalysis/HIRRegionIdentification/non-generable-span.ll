; RUN: opt < %s -analyze -enable-new-pm=0 -hir-region-identification -disable-hir-create-fusion-regions=0 | FileCheck %s
; RUN: opt < %s -passes="print<hir-region-identification>" -disable-hir-create-fusion-regions=0 2>&1 | FileCheck %s

; int bar();
; void foo(int * restrict p, int *q, int n) {
;   for (int i = 0;i<n;++i) {
;     p[0] = bar();                <<< This makes outer loops non-generable
;     for (int j = 0;j<n;++j) {
;       q[j] = p[j] + j;
;     }
;     for (int j = 0;j<n;++j) {
;       q[j] = q[j] + j;
;     }
;     for (int j = 0;j<n;++j) {
;       q[j] = q[j] + j;
;     }
;   }
;}

; Verify that three inner loops are placed into one regions

; CHECK: Region 1
; CHECK: EntryBB: %for.body4
; CHECK: Member BBlocks: %for.body4, %for.body13, %for.body26, %for.body13.preheader, %for.body26.preheader
; CHECK-NOT: Region 2

;Module Before HIR; ModuleID = 'non-generable-span.c'
source_filename = "non-generable-span.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local void @foo(i32* noalias nocapture %p, i32* noalias nocapture %q, i32 %n) local_unnamed_addr #0 {
entry:
  %cmp64 = icmp sgt i32 %n, 0
  br i1 %cmp64, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = sext i32 %n to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.cond.cleanup25
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.cond.cleanup25, %for.body.preheader
  %indvars.iv72 = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next73, %for.cond.cleanup25 ]
  %call = tail call i32 (...) @bar() #2
  %arrayidx = getelementptr inbounds i32, i32* %p, i64 %indvars.iv72
  store i32 %call, i32* %arrayidx, align 4
  br label %for.body4

for.body4:                                        ; preds = %for.body4, %for.body
  %indvars.iv = phi i64 [ 0, %for.body ], [ %indvars.iv.next, %for.body4 ]
  %arrayidx6 = getelementptr inbounds i32, i32* %q, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx6, align 4
  %1 = trunc i64 %indvars.iv to i32
  %add = add nsw i32 %0, %1
  %arrayidx8 = getelementptr inbounds i32, i32* %p, i64 %indvars.iv
  store i32 %add, i32* %arrayidx8, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.body13.preheader, label %for.body4

for.body13.preheader:                             ; preds = %for.body4
  br label %for.body13

for.body13:                                       ; preds = %for.body13.preheader, %for.body13
  %indvars.iv66 = phi i64 [ %indvars.iv.next67, %for.body13 ], [ 0, %for.body13.preheader ]
  %arrayidx15 = getelementptr inbounds i32, i32* %p, i64 %indvars.iv66
  %2 = load i32, i32* %arrayidx15, align 4
  %3 = trunc i64 %indvars.iv66 to i32
  %add16 = add nsw i32 %2, %3
  store i32 %add16, i32* %arrayidx15, align 4
  %indvars.iv.next67 = add nuw nsw i64 %indvars.iv66, 1
  %exitcond68 = icmp eq i64 %indvars.iv.next67, 100
  br i1 %exitcond68, label %for.body26.preheader, label %for.body13

for.body26.preheader:                             ; preds = %for.body13
  br label %for.body26

for.cond.cleanup25:                               ; preds = %for.body26
  %indvars.iv.next73 = add nuw nsw i64 %indvars.iv72, 1
  %exitcond74 = icmp eq i64 %indvars.iv.next73, %wide.trip.count
  br i1 %exitcond74, label %for.cond.cleanup.loopexit, label %for.body

for.body26:                                       ; preds = %for.body26.preheader, %for.body26
  %indvars.iv69 = phi i64 [ %indvars.iv.next70, %for.body26 ], [ 0, %for.body26.preheader ]
  %arrayidx28 = getelementptr inbounds i32, i32* %q, i64 %indvars.iv69
  %4 = load i32, i32* %arrayidx28, align 4
  %5 = trunc i64 %indvars.iv69 to i32
  %add29 = add nsw i32 %4, %5
  store i32 %add29, i32* %arrayidx28, align 4
  %indvars.iv.next70 = add nuw nsw i64 %indvars.iv69, 1
  %exitcond71 = icmp eq i64 %indvars.iv.next70, 100
  br i1 %exitcond71, label %for.cond.cleanup25, label %for.body26
}

declare dso_local i32 @bar(...) local_unnamed_addr #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }


