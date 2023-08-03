; RUN: opt -passes="hir-ssa-deconstruction,hir-loop-fusion,print<hir>" -aa-pipeline="basic-aa" -disable-output -hir-cost-model-throttling=0 < %s 2>&1 | FileCheck %s

; Verify that unsafe call between loops prevents fusion.

; BEGIN REGION { }
;      + DO i1 = 0, 1, 1   <DO_LOOP>
;      |   + DO i2 = 0, 99, 1   <DO_LOOP>
;      |   |   %0 = (@A)[0][i2];
;      |   |   (@B)[0][i2] = i2 + %0;
;      |   + END LOOP
;      |
;      |   %call = @bar();                   <----- Reads unknwon memory. Actually B[0] and C[0], making it illegal to fuse.
;      |
;      |   + DO i2 = 0, 99, 1   <DO_LOOP>
;      |   |   %2 = (@B)[0][i2];
;      |   |   (@C)[0][i2] = i2 + %call + %2;
;      |   + END LOOP
;      + END LOOP
; END REGION

; CHECK: BEGIN REGION
; CHECK:   DO i1
; CHECK:     DO i2
; CHECK:     END LOOP

; CHECK:     bar()

; CHECK:     DO i2
; CHECK:     END LOOP
; CHECK:   END LOOP
; CHECK: END REGION

;Module Before HIR; ModuleID = 'fus1.c'
source_filename = "fus1.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = dso_local local_unnamed_addr global [100 x i32] [i32 1, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0], align 16
@B = dso_local local_unnamed_addr global [100 x i32] [i32 2, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0], align 16
@C = dso_local local_unnamed_addr global [100 x i32] [i32 3, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0], align 16
@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

; Function Attrs: noinline norecurse nounwind readonly uwtable
define dso_local i32 @bar() local_unnamed_addr #0 {
entry:
  %0 = load i32, ptr @C, align 16
  %1 = load i32, ptr @B, align 16
  %add = add nsw i32 %1, %0
  ret i32 %add
}

; Function Attrs: nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #1 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond.cleanup10, %entry
  %j.041 = phi i32 [ 0, %entry ], [ %inc22, %for.cond.cleanup10 ]
  br label %for.body4

for.cond.cleanup:                                 ; preds = %for.cond.cleanup10
  %call.lcssa = phi i32 [ %call, %for.cond.cleanup10 ]
  %call24 = tail call i32 (ptr, ...) @printf(ptr @.str, i32 %call.lcssa)
  ret i32 0

for.cond.cleanup3:                                ; preds = %for.body4
  %call = tail call i32 @bar()
  br label %for.body11

for.body4:                                        ; preds = %for.body4, %for.cond1.preheader
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body4 ]
  %arrayidx = getelementptr inbounds [100 x i32], ptr @A, i64 0, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx, align 4
  %1 = trunc i64 %indvars.iv to i32
  %add = add nsw i32 %0, %1
  %arrayidx6 = getelementptr inbounds [100 x i32], ptr @B, i64 0, i64 %indvars.iv
  store i32 %add, ptr %arrayidx6, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.cond.cleanup3, label %for.body4

for.cond.cleanup10:                               ; preds = %for.body11
  %inc22 = add nuw nsw i32 %j.041, 1
  %exitcond45 = icmp eq i32 %inc22, 2
  br i1 %exitcond45, label %for.cond.cleanup, label %for.cond1.preheader

for.body11:                                       ; preds = %for.body11, %for.cond.cleanup3
  %indvars.iv42 = phi i64 [ 0, %for.cond.cleanup3 ], [ %indvars.iv.next43, %for.body11 ]
  %arrayidx13 = getelementptr inbounds [100 x i32], ptr @B, i64 0, i64 %indvars.iv42
  %2 = load i32, ptr %arrayidx13, align 4
  %3 = trunc i64 %indvars.iv42 to i32
  %add14 = add i32 %call, %3
  %add15 = add i32 %add14, %2
  %arrayidx17 = getelementptr inbounds [100 x i32], ptr @C, i64 0, i64 %indvars.iv42
  store i32 %add15, ptr %arrayidx17, align 4
  %indvars.iv.next43 = add nuw nsw i64 %indvars.iv42, 1
  %exitcond44 = icmp eq i64 %indvars.iv.next43, 100
  br i1 %exitcond44, label %for.cond.cleanup10, label %for.body11
}

; Function Attrs: nounwind
declare dso_local i32 @printf(ptr nocapture readonly, ...) local_unnamed_addr #2

attributes #0 = { noinline norecurse nounwind readonly uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }


