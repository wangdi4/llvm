; RUN: opt -passes="hir-ssa-deconstruction,hir-loop-fusion,print<hir>" -aa-pipeline="basic-aa" -disable-output -hir-create-function-level-region < %s 2>&1 | FileCheck %s

; Verify that unsafe and unsafe loops may NOT be fused.

; BEGIN REGION { }
;      + DO i1 = 0, 99, 1   <DO_LOOP>
;      |   %0 = (@B)[0][i1];
;      |   (@A)[0][i1] = i1 + %0;
;      |   %call = @printf(&((@.str)[0][0]),  i1 + %0);    <---- Unsafe call
;      + END LOOP
;
;      + DO i1 = 0, 99, 1   <DO_LOOP>
;      |   %2 = (@A)[0][i1];
;      |   (@C)[0][i1] = i1 + %2;
;      |   %call = @printf(&((@.str)[0][0]),  i1 + %2);    <---- Unsafe call
;      + END LOOP
;
;      ret ;
; END REGION

; CHECK: BEGIN REGION
; CHECK: DO i1
; CHECK: DO i1
; CHECK: END REGION

;Module Before HIR; ModuleID = 'unsafe-unsafe-fusion.c'
source_filename = "unsafe-unsafe-fusion.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16
@A = common dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16
@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1
@C = common dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define dso_local void @foo() local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv35 = phi i64 [ 0, %entry ], [ %indvars.iv.next36, %for.body ]
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* @B, i64 0, i64 %indvars.iv35
  %0 = load i32, i32* %arrayidx, align 4
  %1 = trunc i64 %indvars.iv35 to i32
  %add = add nsw i32 %0, %1
  %arrayidx2 = getelementptr inbounds [100 x i32], [100 x i32]* @A, i64 0, i64 %indvars.iv35
  store i32 %add, i32* %arrayidx2, align 4
  %call = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i64 0, i64 0), i32 %add)
  %indvars.iv.next36 = add nuw nsw i64 %indvars.iv35, 1
  %exitcond37 = icmp eq i64 %indvars.iv.next36, 100
  br i1 %exitcond37, label %for.body9.preheader, label %for.body

for.body9.preheader:                              ; preds = %for.body
  br label %for.body9

for.cond.cleanup8:                                ; preds = %for.body9
  ret void

for.body9:                                        ; preds = %for.body9.preheader, %for.body9
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body9 ], [ 0, %for.body9.preheader ]
  %arrayidx11 = getelementptr inbounds [100 x i32], [100 x i32]* @A, i64 0, i64 %indvars.iv
  %2 = load i32, i32* %arrayidx11, align 4
  %3 = trunc i64 %indvars.iv to i32
  %add12 = add nsw i32 %2, %3
  %arrayidx14 = getelementptr inbounds [100 x i32], [100 x i32]* @C, i64 0, i64 %indvars.iv
  store i32 %add12, i32* %arrayidx14, align 4
  %call17 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i64 0, i64 0), i32 %add12)
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.cond.cleanup8, label %for.body9
}

; Function Attrs: nounwind
declare dso_local i32 @printf(i8* nocapture readonly, ...) local_unnamed_addr #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }


