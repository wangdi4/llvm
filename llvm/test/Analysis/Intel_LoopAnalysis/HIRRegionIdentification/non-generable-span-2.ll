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
;       q[j] = q[j] + j + bar();   <<< This makes second inner loop non-generable
;     }
;     for (int j = 0;j<n;++j) {
;       q[j] = q[j] + j;
;     }
;   }
;}

; Verify that first and last inner loop will not be placed into a single region because of non-generable loop in between.

; CHECK: Region 1
; CHECK: EntryBB: %for.body4
; CHECK: Member BBlocks: %for.body4

; CHECK: Region 2
; CHECK: EntryBB: %for.body27
; CHECK: Member BBlocks: %for.body27

;Module Before HIR; ModuleID = 'non-generable-span-2.c'
source_filename = "non-generable-span-2.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local void @foo(i32* noalias nocapture %p, i32* nocapture %q, i32 %n) local_unnamed_addr #0 {
entry:
  %cmp70 = icmp sgt i32 %n, 0
  br i1 %cmp70, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %entry
  %wide.trip.count = sext i32 %n to i64
  %wide.trip.count74 = sext i32 %n to i64
  %wide.trip.count78 = sext i32 %n to i64
  br label %for.body4.preheader

for.cond.cleanup.loopexit:                        ; preds = %for.cond.cleanup26
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body4.preheader:                              ; preds = %for.body.lr.ph, %for.cond.cleanup26
  %i.071 = phi i32 [ 0, %for.body.lr.ph ], [ %inc37, %for.cond.cleanup26 ]
  %call = tail call i32 (...) @bar() #2
  store i32 %call, i32* %p, align 4
  br label %for.body4

for.body4:                                        ; preds = %for.body4, %for.body4.preheader
  %indvars.iv = phi i64 [ 0, %for.body4.preheader ], [ %indvars.iv.next, %for.body4 ]
  %arrayidx5 = getelementptr inbounds i32, i32* %p, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx5, align 4
  %1 = trunc i64 %indvars.iv to i32
  %add = add nsw i32 %0, %1
  %arrayidx7 = getelementptr inbounds i32, i32* %q, i64 %indvars.iv
  store i32 %add, i32* %arrayidx7, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.body12.preheader, label %for.body4

for.body12.preheader:                             ; preds = %for.body4
  br label %for.body12

for.body12:                                       ; preds = %for.body12.preheader, %for.body12
  %indvars.iv72 = phi i64 [ %indvars.iv.next73, %for.body12 ], [ 0, %for.body12.preheader ]
  %arrayidx14 = getelementptr inbounds i32, i32* %q, i64 %indvars.iv72
  %2 = load i32, i32* %arrayidx14, align 4
  %3 = trunc i64 %indvars.iv72 to i32
  %add15 = add nsw i32 %2, %3
  %call16 = tail call i32 (...) @bar() #2
  %add17 = add nsw i32 %add15, %call16
  store i32 %add17, i32* %arrayidx14, align 4
  %indvars.iv.next73 = add nuw nsw i64 %indvars.iv72, 1
  %exitcond75 = icmp eq i64 %indvars.iv.next73, %wide.trip.count74
  br i1 %exitcond75, label %for.body27.preheader, label %for.body12

for.body27.preheader:                             ; preds = %for.body12
  br label %for.body27

for.cond.cleanup26:                               ; preds = %for.body27
  %inc37 = add nuw nsw i32 %i.071, 1
  %exitcond80 = icmp eq i32 %inc37, %n
  br i1 %exitcond80, label %for.cond.cleanup.loopexit, label %for.body4.preheader

for.body27:                                       ; preds = %for.body27.preheader, %for.body27
  %indvars.iv76 = phi i64 [ %indvars.iv.next77, %for.body27 ], [ 0, %for.body27.preheader ]
  %arrayidx29 = getelementptr inbounds i32, i32* %q, i64 %indvars.iv76
  %4 = load i32, i32* %arrayidx29, align 4
  %5 = trunc i64 %indvars.iv76 to i32
  %add30 = add nsw i32 %4, %5
  store i32 %add30, i32* %arrayidx29, align 4
  %indvars.iv.next77 = add nuw nsw i64 %indvars.iv76, 1
  %exitcond79 = icmp eq i64 %indvars.iv.next77, %wide.trip.count78
  br i1 %exitcond79, label %for.cond.cleanup26, label %for.body27
}

declare dso_local i32 @bar(...) local_unnamed_addr #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }


