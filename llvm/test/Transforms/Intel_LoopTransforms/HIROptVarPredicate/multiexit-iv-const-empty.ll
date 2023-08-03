; RUN: opt -passes="hir-ssa-deconstruction,hir-opt-var-predicate,print<hir>,hir-cg" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; XFAIL: *

; BEGIN REGION { }
;      + DO i1 = 0, 48, 1   <DO_MULTI_EXIT_LOOP>
;      |   if (i1 == 10)
;      |   {
;      |      goto if.end3.loopexit;
;      |   }
;      + END LOOP
; END REGION

; CHECK: Function
; CHECK: BEGIN REGION { modified }
; CHECK:       goto if.end3.loopexit;
; CHECK: END REGION

; ModuleID = 'loop.ll'
source_filename = "lpbench.c"
target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

@.str = external hidden unnamed_addr constant [4 x i8], align 1

; Function Attrs: nounwind
define i32 @main(i32 %argc, ptr nocapture readonly %argv) local_unnamed_addr #0 {
entry:
  %cmp = icmp sgt i32 %argc, 1
  br i1 %cmp, label %for.cond.preheader, label %exit

for.cond.preheader:                               ; preds = %entry
  %arrayidx = getelementptr inbounds ptr, ptr %argv, i32 1
  %0 = load ptr, ptr %arrayidx, align 4
  %call = tail call i32 @strcmp(ptr %0, ptr @.str) #2
  br label %for.body

for.cond:                                         ; preds = %for.body
  %cmp1 = icmp slt i32 %inc, 50
  br i1 %cmp1, label %for.body, label %if.end3.loopexit

for.body:                                         ; preds = %for.cond, %for.cond.preheader
  %i.058 = phi i32 [ 1, %for.cond.preheader ], [ %inc, %for.cond ]
  %inc = add nuw nsw i32 %i.058, 1
  %dec = add nuw nsw i32 %i.058, -1
  %tobool = icmp eq i32 %dec, 10
  br i1 %tobool, label %if.end3.loopexit, label %for.cond

if.end3.loopexit:                                 ; preds = %for.body, %for.cond
  %ga_testing.0.ph = phi i1 [ true, %for.body ], [ false, %for.cond ]
  br label %exit

exit:                                             ; preds = %if.end3.loopexit, %entry
  ret i32 0
}

; Function Attrs: nounwind readonly
declare i32 @strcmp(ptr nocapture, ptr nocapture) local_unnamed_addr #1

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="pentium4" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readonly "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="pentium4" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind readonly }


