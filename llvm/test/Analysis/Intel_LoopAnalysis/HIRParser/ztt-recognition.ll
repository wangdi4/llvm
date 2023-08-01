; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s

; Check that we are able to recognize ztt of i2 loop by suppressing instcombine's cmp optimization.
; CHECK: DO i1
; CHECK-NOT: if (
; CHECK: DO i2


target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"

define void @foo(ptr nocapture readonly %InputData, ptr nocapture %AutoCorrData, i16 signext %DataSize, i16 signext %NumberOfLags, i16 signext %Scale) #0 {
entry:
  %conv = sext i16 %NumberOfLags to i32
  %cmp35 = icmp sgt i16 %NumberOfLags, 0
  br i1 %cmp35, label %for.body.lr.ph, label %for.end17

for.body.lr.ph:                                   ; preds = %entry
  %conv2 = sext i16 %DataSize to i32
  %conv10 = sext i16 %Scale to i32
  br label %for.body

for.body:                                         ; preds = %for.end, %for.body.lr.ph
  %indvars.iv = phi i32 [ %conv2, %for.body.lr.ph ], [ %indvars.iv.next, %for.end ]
  %lag.036 = phi i32 [ 0, %for.body.lr.ph ], [ %inc16, %for.end ]
  %sub = sub nsw i32 %conv2, %lag.036
  %cmp432 = icmp sgt i32 %sub, 0
  br i1 %cmp432, label %for.body6.preheader, label %for.end

for.body6.preheader:                              ; preds = %for.body
  br label %for.body6

for.body6:                                        ; preds = %for.body6, %for.body6.preheader
  %Accumulator.034 = phi i32 [ %add11, %for.body6 ], [ 0, %for.body6.preheader ]
  %i.033 = phi i32 [ %inc, %for.body6 ], [ 0, %for.body6.preheader ]
  %arrayidx = getelementptr inbounds i16, ptr %InputData, i32 %i.033
  %tmp = load i16, ptr %arrayidx, align 2
  %conv7 = sext i16 %tmp to i32
  %add = add nuw nsw i32 %i.033, %lag.036
  %arrayidx8 = getelementptr inbounds i16, ptr %InputData, i32 %add
  %tmp1 = load i16, ptr %arrayidx8, align 2
  %conv9 = sext i16 %tmp1 to i32
  %mul = mul nsw i32 %conv9, %conv7
  %shr = ashr i32 %mul, %conv10
  %add11 = add nsw i32 %shr, %Accumulator.034
  %inc = add nuw nsw i32 %i.033, 1
  %exitcond = icmp ne i32 %inc, %indvars.iv
  br i1 %exitcond, label %for.body6, label %for.end.loopexit

for.end.loopexit:                                 ; preds = %for.body6
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %for.body
  %Accumulator.0.lcssa = phi i32 [ 0, %for.body ], [ %add11, %for.end.loopexit ]
  %shr1231 = lshr i32 %Accumulator.0.lcssa, 16
  %conv13 = trunc i32 %shr1231 to i16
  %arrayidx14 = getelementptr inbounds i16, ptr %AutoCorrData, i32 %lag.036
  store i16 %conv13, ptr %arrayidx14, align 2
  %inc16 = add nuw nsw i32 %lag.036, 1
  %indvars.iv.next = add nsw i32 %indvars.iv, -1
  %exitcond38 = icmp ne i32 %inc16, %conv
  br i1 %exitcond38, label %for.body, label %for.end17.loopexit

for.end17.loopexit:                               ; preds = %for.end
  br label %for.end17

for.end17:                                        ; preds = %for.end17.loopexit, %entry
  ret void
}

attributes #0 = { "pre_loopopt" }
