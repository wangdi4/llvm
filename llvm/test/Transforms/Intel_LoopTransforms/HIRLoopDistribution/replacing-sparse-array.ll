;RUN: opt -mattr=+avx512f -enable-intel-advanced-opts -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-distribute-memrec,print<hir>" -disable-output < %s 2>&1 | FileCheck %s

; Verify that we separate sparse array reductions into separate loop by 
; performing scalar expansion.

;*** IR Dump Before HIR Loop Distribution MemRec ***
;Function: nab
;
; BEGIN REGION { }
; + DO i1 = 0, sext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1000>
; |   %0 = (@a1)[0][i1];
; |   %div = 4 * %0  /  3;
; |   %1 = (@a2)[0][i1];
; |   %div4 = 4 * %1  /  3;
; |   %2 = (@atype)[0][i1];
; |   %sub8 = (@x)[0][%div]  -  (@x)[0][%div4];
; |   %sub12 = (@x)[0][%div + 1]  -  (@x)[0][%div4 + 1];
; |   %sub17 = (@x)[0][%div + 2]  -  (@x)[0][%div4 + 2];
; |   %sub27 = (@x)[0][%div + 3]  -  (@x)[0][%div4 + 3];
; |   %sub34 = 1.000000e+00  -  (@Req)[0][%2 + -1];
; |   %mul36 = (@Rk)[0][%2 + -1]  *  %sub34;
; |   %mul37 = %sub34  *  %mul36;
; |   %add38145 = %add38145  +  %mul37;
; |   %mul39 = %mul36  *  2.000000e+00;
; |   %mul40 = %sub8  *  %mul39;
; |   %add44 = (@f)[0][%div + %foff]  +  %mul40;
; |   (@f)[0][%div + %foff] = %add44;
; |   %mul45 = %sub12  *  %mul39;
; |   %add49 = %mul45  +  (@f)[0][%div + %foff + 1];
; |   (@f)[0][%div + %foff + 1] = %add49;
; |   %mul50 = %sub17  *  %mul39;
; |   %add54 = %mul50  +  (@f)[0][%div + %foff + 2];
; |   (@f)[0][%div + %foff + 2] = %add54;
; |   %sub59 = (@f)[0][%div4 + %foff]  -  %mul40;
; |   (@f)[0][%div4 + %foff] = %sub59;
; |   %sub64 = (@f)[0][%div4 + %foff + 1]  -  %mul45;
; |   (@f)[0][%div4 + %foff + 1] = %sub64;
; |   %sub69 = (@f)[0][%div4 + %foff + 2]  -  %mul50;
; |   (@f)[0][%div4 + %foff + 2] = %sub69;
; |   %mul70 = %sub27  *  %mul39;
; |   %add74 = %mul70  +  (@f)[0][%div + %foff + 3];
; |   (@f)[0][%div + %foff + 3] = %add74;
; |   %sub79 = (@f)[0][%div4 + %foff + 3]  -  %mul70;
; |   (@f)[0][%div4 + %foff + 3] = %sub79;
; + END LOOP
; END REGION
;
;*** IR Dump After HIR Loop Distribution MemRec ***
;Function: nab
;
; CHECK: modified

; CHECK: |   + DO i2 = 0, %min, 1   <DO_LOOP>  <MAX_TC_EST = 64>  <LEGAL_MAX_TC = 64>
; CHECK: |   |   %0 = (@a1)[0][64 * i1 + i2];
; CHECK: |   |   %div = 4 * %0  /  3;
; CHECK: |   |   (%.TempArray)[0][i2] = %div;
; CHECK: |   |   %1 = (@a2)[0][64 * i1 + i2];
; CHECK: |   |   %div4 = 4 * %1  /  3;
; CHECK: |   |   (%.TempArray1)[0][i2] = %div4;
; CHECK: |   |   %2 = (@atype)[0][64 * i1 + i2];
; CHECK: |   |   %sub8 = (@x)[0][%div]  -  (@x)[0][%div4];
; CHECK: |   |   %sub12 = (@x)[0][%div + 1]  -  (@x)[0][%div4 + 1];
; CHECK: |   |   %sub17 = (@x)[0][%div + 2]  -  (@x)[0][%div4 + 2];
; CHECK: |   |   %sub27 = (@x)[0][%div + 3]  -  (@x)[0][%div4 + 3];
; CHECK: |   |   %sub34 = 1.000000e+00  -  (@Req)[0][%2 + -1];
; CHECK: |   |   %mul36 = (@Rk)[0][%2 + -1]  *  %sub34;
; CHECK: |   |   %mul37 = %sub34  *  %mul36;
; CHECK: |   |   (%.TempArray3)[0][i2] = %mul37;
; CHECK: |   |   %mul39 = %mul36  *  2.000000e+00;
; CHECK: |   |   %mul40 = %sub8  *  %mul39;
; CHECK: |   |   (%.TempArray5)[0][i2] = %mul40;
; CHECK: |   |   %mul45 = %sub12  *  %mul39;
; CHECK: |   |   (%.TempArray7)[0][i2] = %mul45;
; CHECK: |   |   %mul50 = %sub17  *  %mul39;
; CHECK: |   |   (%.TempArray9)[0][i2] = %mul50;
; CHECK: |   |   %mul70 = %sub27  *  %mul39;
; CHECK: |   |   (%.TempArray11)[0][i2] = %mul70;
; CHECK: |   + END LOOP
; CHECK: |
; CHECK: |
; CHECK: |   + DO i2 = 0, %min, 1   <DO_LOOP>  <MAX_TC_EST = 64>  <LEGAL_MAX_TC = 64>
; CHECK: |   |   %div = (%.TempArray)[0][i2];
; CHECK: |   |   %div4 = (%.TempArray1)[0][i2];
; CHECK: |   |   %mul37 = (%.TempArray3)[0][i2];
; CHECK: |   |   %mul40 = (%.TempArray5)[0][i2];
; CHECK: |   |   %mul45 = (%.TempArray7)[0][i2];
; CHECK: |   |   %mul50 = (%.TempArray9)[0][i2];
; CHECK: |   |   %mul70 = (%.TempArray11)[0][i2];
; CHECK: |   |   %add38145 = %add38145  +  %mul37;
; CHECK: |   |   %add44 = (@f)[0][%div + %foff]  +  %mul40;
; CHECK: |   |   (@f)[0][%div + %foff] = %add44;
; CHECK: |   |   %add49 = %mul45  +  (@f)[0][%div + %foff + 1];
; CHECK: |   |   (@f)[0][%div + %foff + 1] = %add49;
; CHECK: |   |   %add54 = %mul50  +  (@f)[0][%div + %foff + 2];
; CHECK: |   |   (@f)[0][%div + %foff + 2] = %add54;
; CHECK: |   |   %sub59 = (@f)[0][%div4 + %foff]  -  %mul40;
; CHECK: |   |   (@f)[0][%div4 + %foff] = %sub59;
; CHECK: |   |   %sub64 = (@f)[0][%div4 + %foff + 1]  -  %mul45;
; CHECK: |   |   (@f)[0][%div4 + %foff + 1] = %sub64;
; CHECK: |   |   %sub69 = (@f)[0][%div4 + %foff + 2]  -  %mul50;
; CHECK: |   |   (@f)[0][%div4 + %foff + 2] = %sub69;
; CHECK: |   |   %add74 = %mul70  +  (@f)[0][%div + %foff + 3];
; CHECK: |   |   (@f)[0][%div + %foff + 3] = %add74;
; CHECK: |   |   %sub79 = (@f)[0][%div4 + %foff + 3]  -  %mul70;
; CHECK: |   |   (@f)[0][%div4 + %foff + 3] = %sub79;
; CHECK: |   + END LOOP
;
;RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-distribute-memrec,hir-cg,intel-ir-optreport-emitter" -aa-pipeline="basic-aa" -force-hir-cg -intel-opt-report=low -disable-output 2>&1 %s | FileCheck %s  -check-prefix=OPTREPORT

;OPTREPORT: LOOP BEGIN
;OPTREPORT: <Loop stripmined by 64>
;OPTREPORT:     LOOP BEGIN
;OPTREPORT:     <Distributed chunk1>
;OPTREPORT:         remark #25427: Loop distributed (2 way) for enabling vectorization on some chunks
;OPTREPORT:     LOOP END
;OPTREPORT:     LOOP BEGIN
;OPTREPORT:     <Distributed chunk2>
;OPTREPORT:     LOOP END
;OPTREPORT: LOOP END

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@e_bond = common dso_local local_unnamed_addr global float 0.000000e+00, align 4
@a1 = common dso_local local_unnamed_addr global [1000 x i64] zeroinitializer, align 16
@a2 = common dso_local local_unnamed_addr global [1000 x i64] zeroinitializer, align 16
@atype = common dso_local local_unnamed_addr global [1000 x i64] zeroinitializer, align 16
@x = common dso_local local_unnamed_addr global [1000 x float] zeroinitializer, align 16
@Req = common dso_local local_unnamed_addr global [1000 x float] zeroinitializer, align 16
@Rk = common dso_local local_unnamed_addr global [1000 x float] zeroinitializer, align 16
@f = common dso_local local_unnamed_addr global [1000 x float] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local i32 @nab(i32 %N, i64 %foff) local_unnamed_addr #0 {
entry:
  store float 0.000000e+00, ptr @e_bond, align 4, !tbaa !2
  %conv = sext i32 %N to i64
  %cmp143 = icmp sgt i32 %N, 0
  br i1 %cmp143, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %add38145 = phi float [ %add38, %for.body ], [ 0.000000e+00, %for.body.preheader ]
  %i.0144 = phi i64 [ %inc, %for.body ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds [1000 x i64], ptr @a1, i64 0, i64 %i.0144
  %0 = load i64, ptr %arrayidx, align 8, !tbaa !6
  %mul = shl nsw i64 %0, 2
  %div = sdiv i64 %mul, 3
  %arrayidx2 = getelementptr inbounds [1000 x i64], ptr @a2, i64 0, i64 %i.0144
  %1 = load i64, ptr %arrayidx2, align 8, !tbaa !6
  %mul3 = shl nsw i64 %1, 2
  %div4 = sdiv i64 %mul3, 3
  %arrayidx5 = getelementptr inbounds [1000 x i64], ptr @atype, i64 0, i64 %i.0144
  %2 = load i64, ptr %arrayidx5, align 8, !tbaa !6
  %sub = add nsw i64 %2, -1
  %arrayidx6 = getelementptr inbounds [1000 x float], ptr @x, i64 0, i64 %div
  %3 = load float, ptr %arrayidx6, align 4, !tbaa !9
  %arrayidx7 = getelementptr inbounds [1000 x float], ptr @x, i64 0, i64 %div4
  %4 = load float, ptr %arrayidx7, align 4, !tbaa !9
  %sub8 = fsub float %3, %4
  %add = add nsw i64 %div, 1
  %arrayidx9 = getelementptr inbounds [1000 x float], ptr @x, i64 0, i64 %add
  %5 = load float, ptr %arrayidx9, align 4, !tbaa !9
  %add10 = add nsw i64 %div4, 1
  %arrayidx11 = getelementptr inbounds [1000 x float], ptr @x, i64 0, i64 %add10
  %6 = load float, ptr %arrayidx11, align 4, !tbaa !9
  %sub12 = fsub float %5, %6
  %add13 = add nsw i64 %div, 2
  %arrayidx14 = getelementptr inbounds [1000 x float], ptr @x, i64 0, i64 %add13
  %7 = load float, ptr %arrayidx14, align 4, !tbaa !9
  %add15 = add nsw i64 %div4, 2
  %arrayidx16 = getelementptr inbounds [1000 x float], ptr @x, i64 0, i64 %add15
  %8 = load float, ptr %arrayidx16, align 4, !tbaa !9
  %sub17 = fsub float %7, %8
  %add23 = add nsw i64 %div, 3
  %arrayidx24 = getelementptr inbounds [1000 x float], ptr @x, i64 0, i64 %add23
  %9 = load float, ptr %arrayidx24, align 4, !tbaa !9
  %add25 = add nsw i64 %div4, 3
  %arrayidx26 = getelementptr inbounds [1000 x float], ptr @x, i64 0, i64 %add25
  %10 = load float, ptr %arrayidx26, align 4, !tbaa !9
  %sub27 = fsub float %9, %10
  %arrayidx33 = getelementptr inbounds [1000 x float], ptr @Req, i64 0, i64 %sub
  %11 = load float, ptr %arrayidx33, align 4, !tbaa !9
  %sub34 = fsub float 1.000000e+00, %11
  %arrayidx35 = getelementptr inbounds [1000 x float], ptr @Rk, i64 0, i64 %sub
  %12 = load float, ptr %arrayidx35, align 4, !tbaa !9
  %mul36 = fmul float %12, %sub34
  %mul37 = fmul float %sub34, %mul36
  %add38 = fadd float %add38145, %mul37
  %mul39 = fmul float %mul36, 2.000000e+00
  %mul40 = fmul float %sub8, %mul39
  %add41 = add nsw i64 %div, %foff
  %arrayidx43 = getelementptr inbounds [1000 x float], ptr @f, i64 0, i64 %add41
  %13 = load float, ptr %arrayidx43, align 4, !tbaa !9
  %add44 = fadd float %13, %mul40
  store float %add44, ptr %arrayidx43, align 4, !tbaa !9
  %mul45 = fmul float %sub12, %mul39
  %add47 = add nsw i64 %add41, 1
  %arrayidx48 = getelementptr inbounds [1000 x float], ptr @f, i64 0, i64 %add47
  %14 = load float, ptr %arrayidx48, align 4, !tbaa !9
  %add49 = fadd float %mul45, %14
  store float %add49, ptr %arrayidx48, align 4, !tbaa !9
  %mul50 = fmul float %sub17, %mul39
  %add52 = add nsw i64 %add41, 2
  %arrayidx53 = getelementptr inbounds [1000 x float], ptr @f, i64 0, i64 %add52
  %15 = load float, ptr %arrayidx53, align 4, !tbaa !9
  %add54 = fadd float %mul50, %15
  store float %add54, ptr %arrayidx53, align 4, !tbaa !9
  %add56 = add nsw i64 %div4, %foff
  %arrayidx58 = getelementptr inbounds [1000 x float], ptr @f, i64 0, i64 %add56
  %16 = load float, ptr %arrayidx58, align 4, !tbaa !9
  %sub59 = fsub float %16, %mul40
  store float %sub59, ptr %arrayidx58, align 4, !tbaa !9
  %add62 = add nsw i64 %add56, 1
  %arrayidx63 = getelementptr inbounds [1000 x float], ptr @f, i64 0, i64 %add62
  %17 = load float, ptr %arrayidx63, align 4, !tbaa !9
  %sub64 = fsub float %17, %mul45
  store float %sub64, ptr %arrayidx63, align 4, !tbaa !9
  %add67 = add nsw i64 %add56, 2
  %arrayidx68 = getelementptr inbounds [1000 x float], ptr @f, i64 0, i64 %add67
  %18 = load float, ptr %arrayidx68, align 4, !tbaa !9
  %sub69 = fsub float %18, %mul50
  store float %sub69, ptr %arrayidx68, align 4, !tbaa !9
  %mul70 = fmul float %sub27, %mul39
  %add72 = add nsw i64 %add41, 3
  %arrayidx73 = getelementptr inbounds [1000 x float], ptr @f, i64 0, i64 %add72
  %19 = load float, ptr %arrayidx73, align 4, !tbaa !9
  %add74 = fadd float %mul70, %19
  store float %add74, ptr %arrayidx73, align 4, !tbaa !9
  %add77 = add nsw i64 %add56, 3
  %arrayidx78 = getelementptr inbounds [1000 x float], ptr @f, i64 0, i64 %add77
  %20 = load float, ptr %arrayidx78, align 4, !tbaa !9
  %sub79 = fsub float %20, %mul70
  store float %sub79, ptr %arrayidx78, align 4, !tbaa !9
  %inc = add nuw nsw i64 %i.0144, 1
  %exitcond = icmp eq i64 %inc, %conv
  br i1 %exitcond, label %for.cond.for.end_crit_edge, label %for.body

for.cond.for.end_crit_edge:                       ; preds = %for.body
  %add38.lcssa = phi float [ %add38, %for.body ]
  store float %add38.lcssa, ptr @e_bond, align 4, !tbaa !2
  br label %for.end

for.end:                                          ; preds = %for.cond.for.end_crit_edge, %entry
  ret i32 0
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 7.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang cd5be4ea094f77df25a78b0ab0ead6e58960206c) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 63c428e8d6195d6fed4cc00322f74ccaa5b06334)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"float", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !8, i64 0}
!7 = !{!"array@_ZTSA1000_l", !8, i64 0}
!8 = !{!"long", !4, i64 0}
!9 = !{!10, !3, i64 0}
!10 = !{!"array@_ZTSA1000_f", !3, i64 0}
