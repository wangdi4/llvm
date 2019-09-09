; Test VPValue based code generation phi blending
; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -VPlanDriverHIR -hir-cg -vplan-force-vf=4  -S -print-after=VPlanDriverHIR -enable-vp-value-codegen-hir %s 2>&1 | FileCheck %s
; CHECK:        DO i1 = 0, 99, 4
; CHECK:           [[ARR1VAL:%.*]] = (<4 x i64>*)(@arr1)[0][i1 + <i64 0, i64 1, i64 2, i64 3>];
; CHECK:           [[TMASK:%.*]] = [[ARR1VAL]] == 0;
; CHECK:           [[FMASK:%.*]] = [[TMASK]]  ^  -1;
; CHECK:           [[ARR2VAL:%.*]] = (<4 x i64>*)(@arr2)[0][i1 + <i64 0, i64 1, i64 2, i64 3>]; Mask = @{[[FMASK]]}
; CHECK:           [[ADD:%.*]] = [[ARR2VAL]]  +  10; Mask = @{[[FMASK]]}
; CHECK:           [[ADDCOPY:%.*]] = [[ADD]]; Mask = @{[[FMASK]]}
; CHECK:           [[ARR3VAL:%.*]] = (<4 x i64>*)(@arr3)[0][i1 + <i64 0, i64 1, i64 2, i64 3>]; Mask = @{[[TMASK]]}
; CHECK:           [[SUB:%.*]] = [[ARR3VAL]]  +  -20; Mask = @{[[TMASK]]}
; CHECK:           [[SUBCOPY:%.*]] = [[SUB]]; Mask = @{[[TMASK]]}
; CHECK:           [[SELECT:%.*]] = ([[TMASK]] == <i1 true, i1 true, i1 true, i1 true>) ? [[SUBCOPY]] : [[ADDCOPY]];
; CHECK:           (<4 x i64>*)(@arr1)[0][i1 + <i64 0, i64 1, i64 2, i64 3>] = [[SELECT]];
; CHECK:        END LOOP
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@arr1 = common dso_local local_unnamed_addr global [100 x i64] zeroinitializer, align 16
@arr2 = common dso_local local_unnamed_addr global [100 x i64] zeroinitializer, align 16
@arr3 = common dso_local local_unnamed_addr global [100 x i64] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local void @foo() local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %if.end, %entry
  %i.010 = phi i64 [ 0, %entry ], [ %inc, %if.end ]
  %arrayidx = getelementptr inbounds [100 x i64], [100 x i64]* @arr1, i64 0, i64 %i.010
  %0 = load i64, i64* %arrayidx, align 8
  %tobool = icmp eq i64 %0, 0
  br i1 %tobool, label %if.else, label %if.then

if.then:                                          ; preds = %for.body
  %arrayidx1 = getelementptr inbounds [100 x i64], [100 x i64]* @arr2, i64 0, i64 %i.010
  %1 = load i64, i64* %arrayidx1, align 8
  %add = add nsw i64 %1, 10
  br label %if.end

if.else:                                          ; preds = %for.body
  %arrayidx2 = getelementptr inbounds [100 x i64], [100 x i64]* @arr3, i64 0, i64 %i.010
  %2 = load i64, i64* %arrayidx2, align 8
  %sub = add nsw i64 %2, -20
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %t1.0 = phi i64 [ %add, %if.then ], [ %sub, %if.else ]
  store i64 %t1.0, i64* %arrayidx, align 8
  %inc = add nuw nsw i64 %i.010, 1
  %exitcond = icmp eq i64 %inc, 100
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %if.end
  ret void
}
