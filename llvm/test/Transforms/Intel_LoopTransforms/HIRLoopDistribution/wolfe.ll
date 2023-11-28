;RUN: opt -passes="hir-ssa-deconstruction,hir-loop-distribute-memrec,hir-vec-dir-insert,print<hir>" -disable-output < %s 2>&1 | FileCheck %s

; This case distributes into 3 loops to make 2 vectorizable loops and one serial

; Split at 8-10 and again 21-22
;          BEGIN REGION { }
;<30>         + DO i1 = 0, 98, 1   <DO_LOOP>
;<3>          |   %0 = (@B)[0][i1 + 1];
;<5>          |   %1 = (@C)[0][i1 + 1];
;<6>          |   %add = %0  +  %1;
;<8>          |   (@A)[0][i1 + 1] = %add;
;<10>         |   %2 = (@E)[0][i1 + 1];
;<11>         |   %reload = (@A)[0][i1 + 1];
;<12>         |   %div = %reload  /  %2;
;<14>         |   (@D)[0][i1 + 1] = %div;
;<17>         |   %3 = (@D)[0][i1 + 2];
;<18>         |   %add16 = %div  +  %3;
;<19>         |   %conv18 = %add16  *  5.000000e-01;
;<21>         |   (@E)[0][i1 + 2] = %conv18;
;<22>         |   %4 = (@E)[0][i1 + 1];
;<23>         |   %mul = %4  *  %4;
;<25>         |   (@F)[0][i1 + 1] = %mul;
;<30>         + END LOOP
;          END REGION


; CHECK: modified

; CHECK: %entry.region = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ]

; CHECK: + DO i1 = 0, 98, 1   <DO_LOOP>
; CHECK: |   %0 = (@B)[0][i1 + 1];
; CHECK: |   %1 = (@C)[0][i1 + 1];
; CHECK: |   %add = %0  +  %1;
; CHECK: |   (@A)[0][i1 + 1] = %add;
; CHECK: + END LOOP

; CHECK: @llvm.directive.region.exit(%entry.region); [ DIR.VPO.END.AUTO.VEC() ]

; CHECK: + DO i1 = 0, 98, 1   <DO_LOOP>
; CHECK: |   %2 = (@E)[0][i1 + 1];
; CHECK: |   %reload = (@A)[0][i1 + 1];
; CHECK: |   %div = %reload  /  %2;
; CHECK: |   (@D)[0][i1 + 1] = %div;
; CHECK: |   %3 = (@D)[0][i1 + 2];
; CHECK: |   %add16 = %div  +  %3;
; CHECK: |   %conv18 = %add16  *  5.000000e-01;
; CHECK: |   (@E)[0][i1 + 2] = %conv18;
; CHECK: + END LOOP

; CHECK: %entry.region1 = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ]

; CHECK: + DO i1 = 0, 98, 1   <DO_LOOP>
; CHECK: |   %4 = (@E)[0][i1 + 1];
; CHECK: |   %mul = %4  *  %4;
; CHECK: |   (@F)[0][i1 + 1] = %mul;
; CHECK: + END LOOP

; CHECK: @llvm.directive.region.exit(%entry.region1); [ DIR.VPO.END.AUTO.VEC() ]

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = global [100 x float] zeroinitializer, align 16
@B = global [100 x float] zeroinitializer, align 16
@C = global [100 x float] zeroinitializer, align 16
@D = global [100 x float] zeroinitializer, align 16
@E = global [100 x float] zeroinitializer, align 16
@F = global [100 x float] zeroinitializer, align 16



; Function Attrs: nounwind uwtable
define void @_Z3foov() {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 1, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [100 x float], ptr @B, i64 0, i64 %indvars.iv
  %0 = load float, ptr %arrayidx, align 4
  %arrayidx2 = getelementptr inbounds [100 x float], ptr @C, i64 0, i64 %indvars.iv
  %1 = load float, ptr %arrayidx2, align 4
  %add = fadd float %0, %1
  %arrayidx4 = getelementptr inbounds [100 x float], ptr @A, i64 0, i64 %indvars.iv
  store float %add, ptr %arrayidx4, align 4
  %arrayidx8 = getelementptr inbounds [100 x float], ptr @E, i64 0, i64 %indvars.iv
  %2 = load float, ptr %arrayidx8, align 4
  %reload = load float, ptr %arrayidx4, align 4
  %div = fdiv float %reload, %2
  %arrayidx10 = getelementptr inbounds [100 x float], ptr @D, i64 0, i64 %indvars.iv
  store float %div, ptr %arrayidx10, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %arrayidx15 = getelementptr inbounds [100 x float], ptr @D, i64 0, i64 %indvars.iv.next
  %3 = load float, ptr %arrayidx15, align 4
  %add16 = fadd float %div, %3
  %conv18 = fmul float %add16, 5.000000e-01
  %arrayidx21 = getelementptr inbounds [100 x float], ptr @E, i64 0, i64 %indvars.iv.next
  store float %conv18, ptr %arrayidx21, align 4
  %4 = load float, ptr %arrayidx8, align 4
  %mul = fmul float %4, %4
  %arrayidx27 = getelementptr inbounds [100 x float], ptr @F, i64 0, i64 %indvars.iv
  store float %mul, ptr %arrayidx27, align 4
  %exitcond = icmp ne i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.body, label %for.end

for.end:                                          ; preds = %for.body
  ret void
}

; Function Attrs: nounwind
declare void @llvm.lifetime.start(i64, ptr nocapture)

; Function Attrs: nounwind
declare void @llvm.lifetime.end(i64, ptr nocapture)

