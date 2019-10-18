; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-conditional-temp-sinking -print-before=hir-conditional-temp-sinking -print-after=hir-conditional-temp-sinking < %s 2>&1 | FileCheck %s
; RUN: opt -aa-pipeline="basic-aa" -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir-framework>,hir-conditional-temp-sinking,print<hir-framework>" 2>&1 < %s | FileCheck %s

; Verify that the nested if-else reduction is converted into unconditional reduction.
; Missing definition in one of the if-else cases is handled by initializing the
; new temp 'tmp' with 0 before the if.

; Before Change-

; CHECK: + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK: |   %0 = (@A)[0][i1];
; CHECK: |   %conv = fpext.float.double(%0);
; CHECK: |   if (%conv > 5.200000e+00)
; CHECK: |   {
; CHECK: |      %t.02 = %t.02  -  %0;
; CHECK: |   }
; CHECK: |   else
; CHECK: |   {
; CHECK: |      %1 = (@B)[0][i1];
; CHECK: |      %conv7 = fpext.float.double(%1);
; CHECK: |      if (%conv7 < 2.300000e+00)
; CHECK: |      {
; CHECK: |         %t.02 = %t.02  -  %1;
; CHECK: |      }
; CHECK: |   }
; CHECK: + END LOOP

; After Change-

; CHECK: + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK: |   %0 = (@A)[0][i1];
; CHECK: |   %conv = fpext.float.double(%0);
; CHECK: |   %tmp = 0.000000e+00;
; CHECK: |   if (%conv > 5.200000e+00)
; CHECK: |   {
; CHECK: |      %tmp = %0;
; CHECK: |   }
; CHECK: |   else
; CHECK: |   {
; CHECK: |      %1 = (@B)[0][i1];
; CHECK: |      %conv7 = fpext.float.double(%1);
; CHECK: |      if (%conv7 < 2.300000e+00)
; CHECK: |      {
; CHECK: |         %tmp = %1;
; CHECK: |      }
; CHECK: |   }
; CHECK: |   %t.02 = %t.02  -  %tmp;
; CHECK: + END LOOP


@A = common dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16
@B = common dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16

define dso_local float @foo() local_unnamed_addr {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc
  %t.02 = phi float [ 0.000000e+00, %entry ], [ %t.1, %for.inc ]
  %i.01 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  %idxprom = zext i32 %i.01 to i64
  %arrayidx = getelementptr inbounds [100 x float], [100 x float]* @A, i64 0, i64 %idxprom
  %0 = load float, float* %arrayidx, align 4
  %conv = fpext float %0 to double
  %cmp1 = fcmp fast ogt double %conv, 5.200000e+00
  br i1 %cmp1, label %if.then, label %if.else

if.then:                                          ; preds = %for.body
  %sub = fsub fast float %t.02, %0
  br label %for.inc

if.else:                                          ; preds = %for.body
  %arrayidx6 = getelementptr inbounds [100 x float], [100 x float]* @B, i64 0, i64 %idxprom
  %1 = load float, float* %arrayidx6, align 4
  %conv7 = fpext float %1 to double
  %cmp8 = fcmp fast olt double %conv7, 2.300000e+00
  br i1 %cmp8, label %if.then10, label %for.inc

if.then10:                                        ; preds = %if.else
  %sub13 = fsub fast float %t.02, %1
  br label %for.inc

for.inc:                                          ; preds = %if.then10, %if.else, %if.then
  %t.1 = phi float [ %sub, %if.then ], [ %sub13, %if.then10 ], [ %t.02, %if.else ]
  %inc = add nuw nsw i32 %i.01, 1
  %cmp = icmp ult i32 %inc, 100
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.inc
  %t.0.lcssa = phi float [ %t.1, %for.inc ]
  ret float %t.0.lcssa
}
