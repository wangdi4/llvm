; RUN: opt < %s -hir-ssa-deconstruction -analyze -enable-new-pm=0  -hir-dd-analysis -hir-dd-analysis-verify=Region 2>&1 | FileCheck %s
; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-framework>,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region 2>&1 | FileCheck %s

; Input HIR-
; + DO i1 = 0, zext.i16.i64(%NumberOfLags) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 65535>
; |   %Accumulator.0.lcssa = 0;
; |
; |      %Accumulator.035 = 0;
; |   + DO i2 = 0, -1 * i1 + sext.i16.i64(%DataSize) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 32767>
; |   |   %2 = (%InputData)[i2];
; |   |   %4 = (%InputData)[i1 + i2];
; |   |   %shr = (sext.i16.i64(%2) * sext.i16.i64(%4))  >>  sext.i16.i32(%Scale);
; |   |   %Accumulator.035 = %shr  +  %Accumulator.035;
; |   + END LOOP
; |      %Accumulator.0.lcssa = %Accumulator.035;
; |
; |   (%AutoCorrData)[i1] = (%Accumulator.0.lcssa)/u65536;
; + END LOOP

; Verify that all of the following edges exist in DDG for %Accumulator.0.lcssa
; based on domination/post-domination analysis.

; CHECK-DAG: %Accumulator.0.lcssa --> %Accumulator.0.lcssa OUTPUT (=) (0)
; CHECK-DAG: %Accumulator.0.lcssa --> %Accumulator.0.lcssa FLOW (=) (0)
; CHECK-DAG: %Accumulator.0.lcssa --> %Accumulator.0.lcssa FLOW (=) (0)


define void @fxpAutoCorrelation(i16* %InputData, i16* %AutoCorrData, i16 %DataSize, i16 %NumberOfLags, i16 %Scale) {
entry:
  %cmp36 = icmp sgt i16 %NumberOfLags, 0
  br i1 %cmp36, label %for.body.lr.ph, label %for.end19

for.body.lr.ph:                                   ; preds = %entry
  %conv11 = sext i16 %Scale to i64
  %sh_prom = and i64 %conv11, 4294967295
  %0 = sext i16 %DataSize to i64
  %wide.trip.count4244 = zext i16 %NumberOfLags to i64
  br label %for.body

for.body:                                         ; preds = %for.end, %for.body.lr.ph
  %indvars.iv39 = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next40, %for.end ]
  %1 = sub nsw i64 %0, %indvars.iv39
  %cmp433 = icmp sgt i64 %1, 0
  br i1 %cmp433, label %for.body6.preheader, label %for.end

for.body6.preheader:                              ; preds = %for.body
  br label %for.body6

for.body6:                                        ; preds = %for.body6, %for.body6.preheader
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body6 ], [ 0, %for.body6.preheader ]
  %Accumulator.035 = phi i64 [ %add12, %for.body6 ], [ 0, %for.body6.preheader ]
  %ptridx = getelementptr inbounds i16, i16* %InputData, i64 %indvars.iv
  %2 = load i16, i16* %ptridx, align 2
  %conv7 = sext i16 %2 to i64
  %3 = add nuw nsw i64 %indvars.iv, %indvars.iv39
  %ptridx9 = getelementptr inbounds i16, i16* %InputData, i64 %3
  %4 = load i16, i16* %ptridx9, align 2
  %conv10 = sext i16 %4 to i64
  %mul = mul nsw i64 %conv10, %conv7
  %shr = ashr i64 %mul, %sh_prom
  %add12 = add nsw i64 %shr, %Accumulator.035
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %1
  br i1 %exitcond, label %for.end.loopexit, label %for.body6

for.end.loopexit:                                 ; preds = %for.body6
  %add12.lcssa = phi i64 [ %add12, %for.body6 ]
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %for.body
  %Accumulator.0.lcssa = phi i64 [ 0, %for.body ], [ %add12.lcssa, %for.end.loopexit ]
  %5 = lshr i64 %Accumulator.0.lcssa, 16
  %conv14 = trunc i64 %5 to i16
  %ptridx16 = getelementptr inbounds i16, i16* %AutoCorrData, i64 %indvars.iv39
  store i16 %conv14, i16* %ptridx16, align 2
  %indvars.iv.next40 = add nuw nsw i64 %indvars.iv39, 1
  %exitcond43 = icmp eq i64 %indvars.iv.next40, %wide.trip.count4244
  br i1 %exitcond43, label %for.end19.loopexit, label %for.body

for.end19.loopexit:                               ; preds = %for.end
  br label %for.end19

for.end19:                                        ; preds = %for.end19.loopexit, %entry
  ret void
}

