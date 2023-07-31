; RUN: opt -passes="hir-ssa-deconstruction,hir-loop-distribute-memrec,print<hir>" -S -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; Check that %add definition will be done before its use after the distribution.

; BEGIN REGION { }
;       + DO i1 = 0, 99, 1   <DO_LOOP>
;       |   %add = (@A)[0][i1]  +  (@C)[0][i1];
;       |   if (i1 <u 8)
;       |   {
;       |      (@B)[0][i1] = %add;
;       |   }
;       |   else
;       |   {
;       |      (@B)[0][i1 + 200] = %add;
;       |      %add12 = (@B)[0][i1 + 1]  +  1.000000e+01;
;       |      (@B)[0][i1 + 300] = %add12;
;       |   }
;       + END LOOP
; END REGION

; CHECK: modified

; CHECK: DO i1
; CHECK: END LOOP

; CHECK: DO i1
; CHECK: %add =
; CHECK: = %add
; CHECK: END LOOP

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16
@C = common dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16
@B = common dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16
@D = common dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16

define dso_local void @foo() {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %if.end
  ret void

for.body:                                         ; preds = %if.end, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %if.end ]
  %arrayidx = getelementptr inbounds [100 x float], ptr @A, i64 0, i64 %indvars.iv
  %0 = load float, ptr %arrayidx, align 4
  %arrayidx2 = getelementptr inbounds [100 x float], ptr @C, i64 0, i64 %indvars.iv
  %1 = load float, ptr %arrayidx2, align 4
  %add = fadd float %0, %1
  %cmp3 = icmp ult i64 %indvars.iv, 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  br i1 %cmp3, label %if.then, label %if.else

if.then:                                          ; preds = %for.body
  %arrayidx5 = getelementptr inbounds [100 x float], ptr @B, i64 0, i64 %indvars.iv
  store float %add, ptr %arrayidx5, align 4
  br label %if.end

if.else:                                          ; preds = %for.body
  %2 = add nuw nsw i64 %indvars.iv, 200
  %arrayidx8 = getelementptr inbounds [100 x float], ptr @B, i64 0, i64 %2
  store float %add, ptr %arrayidx8, align 4
  %arrayidx11 = getelementptr inbounds [100 x float], ptr @B, i64 0, i64 %indvars.iv.next
  %3 = load float, ptr %arrayidx11, align 4
  %add12 = fadd float %3, 1.000000e+01
  %4 = add nuw nsw i64 %indvars.iv, 300
  %arrayidx15 = getelementptr inbounds [100 x float], ptr @B, i64 0, i64 %4
  store float %add12, ptr %arrayidx15, align 4
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

