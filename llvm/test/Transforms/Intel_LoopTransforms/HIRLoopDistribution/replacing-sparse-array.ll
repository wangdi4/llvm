;RUN: opt -hir-ssa-deconstruction -analyze -hir-temp-cleanup -hir-loop-distribute-memrec -print-after=hir-loop-distribute-memrec < %s 2>&1 | FileCheck %s

;*** IR Dump Before HIR Loop Distribution MemRec ***
;Function: nab
;
;<0>       BEGIN REGION { }
;<98>            + DO i1 = 0, sext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1000>
;<3>             |   %0 = (@a1)[0][i1];
;<5>             |   %div = 4 * %0  /  3;
;<7>             |   %1 = (@a2)[0][i1];
;<9>             |   %div4 = 4 * %1  /  3;
;<11>            |   %2 = (@atype)[0][i1];
;<17>            |   %sub8 = (@x)[0][%div]  -  (@x)[0][%div4];
;<24>            |   %sub12 = (@x)[0][%div + 1]  -  (@x)[0][%div4 + 1];
;<31>            |   %sub17 = (@x)[0][%div + 2]  -  (@x)[0][%div4 + 2];
;<38>            |   %sub27 = (@x)[0][%div + 3]  -  (@x)[0][%div4 + 3];
;<41>            |   %sub34 = 1.000000e+00  -  (@Req)[0][%2 + -1];
;<44>            |   %mul36 = (@Rk)[0][%2 + -1]  *  %sub34;
;<45>            |   %mul37 = %sub34  *  %mul36;
;<46>            |   %add38145 = %add38145  +  %mul37;
;<47>            |   %mul39 = %mul36  *  2.000000e+00;
;<48>            |   %mul40 = %sub8  *  %mul39;
;<52>            |   %add44 = (@f)[0][%div + %foff]  +  %mul40;
;<53>            |   (@f)[0][%div + %foff] = %add44;
;<54>            |   %mul45 = %sub12  *  %mul39;
;<58>            |   %add49 = %mul45  +  (@f)[0][%div + %foff + 1];
;<59>            |   (@f)[0][%div + %foff + 1] = %add49;
;<60>            |   %mul50 = %sub17  *  %mul39;
;<64>            |   %add54 = %mul50  +  (@f)[0][%div + %foff + 2];
;<65>            |   (@f)[0][%div + %foff + 2] = %add54;
;<69>            |   %sub59 = (@f)[0][%div4 + %foff]  -  %mul40;
;<70>            |   (@f)[0][%div4 + %foff] = %sub59;
;<74>            |   %sub64 = (@f)[0][%div4 + %foff + 1]  -  %mul45;
;<75>            |   (@f)[0][%div4 + %foff + 1] = %sub64;
;<79>            |   %sub69 = (@f)[0][%div4 + %foff + 2]  -  %mul50;
;<80>            |   (@f)[0][%div4 + %foff + 2] = %sub69;
;<81>            |   %mul70 = %sub27  *  %mul39;
;<85>            |   %add74 = %mul70  +  (@f)[0][%div + %foff + 3];
;<86>            |   (@f)[0][%div + %foff + 3] = %add74;
;<90>            |   %sub79 = (@f)[0][%div4 + %foff + 3]  -  %mul70;
;<91>            |   (@f)[0][%div4 + %foff + 3] = %sub79;
;<98>            + END LOOP
;<0>       END REGION
;
;*** IR Dump After HIR Loop Distribution MemRec ***
;Function: nab
;
;<0>       BEGIN REGION { modified }
;<113>           + DO i1 = 0, (sext.i32.i64(%N) + -1)/u64, 1   <DO_LOOP>  <MAX_TC_EST = 1000>
;<114>           |   %min = (-64 * i1 + sext.i32.i64(%N) + -1 <= 63) ? -64 * i1 + sext.i32.i64(%N) + -1 : 63;
;<99>            |   
;<99>            |   + DO i2 = 0, %min, 1   <DO_LOOP>  <MAX_TC_EST = 1000>
;<11>            |   |   %2 = (@atype)[0][64 * i1 + i2];
;<41>            |   |   %sub34 = 1.000000e+00  -  (@Req)[0][%2 + -1];
;<44>            |   |   %mul36 = (@Rk)[0][%2 + -1]  *  %sub34;
;<47>            |   |   %mul39 = %mul36  *  2.000000e+00;
;<45>            |   |   %mul37 = %sub34  *  %mul36;
;<46>            |   |   %add38145 = %add38145  +  %mul37;
;<3>             |   |   %0 = (@a1)[0][64 * i1 + i2];
;<5>             |   |   %div = 4 * %0  /  3;
;<101>           |   |   (%.TempArray)[0][i2] = %div;
;<7>             |   |   %1 = (@a2)[0][64 * i1 + i2];
;<9>             |   |   %div4 = 4 * %1  /  3;
;<103>           |   |   (%.TempArray1)[0][i2] = %div4;
;<17>            |   |   %sub8 = (@x)[0][%div]  -  (@x)[0][%div4];
;<24>            |   |   %sub12 = (@x)[0][%div + 1]  -  (@x)[0][%div4 + 1];
;<31>            |   |   %sub17 = (@x)[0][%div + 2]  -  (@x)[0][%div4 + 2];
;<38>            |   |   %sub27 = (@x)[0][%div + 3]  -  (@x)[0][%div4 + 3];
;<48>            |   |   %mul40 = %sub8  *  %mul39;
;<105>           |   |   (%.TempArray3)[0][i2] = %mul40;
;<81>            |   |   %mul70 = %sub27  *  %mul39;
;<107>           |   |   (%.TempArray5)[0][i2] = %mul70;
;<60>            |   |   %mul50 = %sub17  *  %mul39;
;<109>           |   |   (%.TempArray7)[0][i2] = %mul50;
;<54>            |   |   %mul45 = %sub12  *  %mul39;
;<111>           |   |   (%.TempArray9)[0][i2] = %mul45;
;<99>            |   + END LOOP
;<99>            |   
;<100>           |   
;<100>           |   + DO i2 = 0, %min, 1   <DO_LOOP>  <MAX_TC_EST = 1000>
;<102>           |   |   %div = (%.TempArray)[0][i2];
;<106>           |   |   %mul40 = (%.TempArray3)[0][i2];
;<52>            |   |   %add44 = (@f)[0][%div + %foff]  +  %mul40;
;<53>            |   |   (@f)[0][%div + %foff] = %add44;
;<112>           |   |   %mul45 = (%.TempArray9)[0][i2];
;<58>            |   |   %add49 = %mul45  +  (@f)[0][%div + %foff + 1];
;<59>            |   |   (@f)[0][%div + %foff + 1] = %add49;
;<110>           |   |   %mul50 = (%.TempArray7)[0][i2];
;<64>            |   |   %add54 = %mul50  +  (@f)[0][%div + %foff + 2];
;<65>            |   |   (@f)[0][%div + %foff + 2] = %add54;
;<104>           |   |   %div4 = (%.TempArray1)[0][i2];
;<69>            |   |   %sub59 = (@f)[0][%div4 + %foff]  -  %mul40;
;<70>            |   |   (@f)[0][%div4 + %foff] = %sub59;
;<74>            |   |   %sub64 = (@f)[0][%div4 + %foff + 1]  -  %mul45;
;<75>            |   |   (@f)[0][%div4 + %foff + 1] = %sub64;
;<79>            |   |   %sub69 = (@f)[0][%div4 + %foff + 2]  -  %mul50;
;<80>            |   |   (@f)[0][%div4 + %foff + 2] = %sub69;
;<108>           |   |   %mul70 = (%.TempArray5)[0][i2];
;<85>            |   |   %add74 = %mul70  +  (@f)[0][%div + %foff + 3];
;<86>            |   |   (@f)[0][%div + %foff + 3] = %add74;
;<90>            |   |   %sub79 = (@f)[0][%div4 + %foff + 3]  -  %mul70;
;<91>            |   |   (@f)[0][%div4 + %foff + 3] = %sub79;
;<100>           |   + END LOOP
;<113>           + END LOOP
;<0>       END REGION
;
;CHECK: (%.TempArray)[0][i2] = %div;
;CHECK: (%.TempArray1)[0][i2] = %div4;
;CHECK: (%.TempArray3)[0][i2] = %mul40;
;CHECK: (%.TempArray5)[0][i2] = %mul70;
;CHECK: (%.TempArray7)[0][i2] = %mul50;
;CHECK: (%.TempArray9)[0][i2] = %mul45;
;CHECK: %div = (%.TempArray)[0][i2];
;CHECK: %mul40 = (%.TempArray3)[0][i2];
;CHECK: %mul45 = (%.TempArray9)[0][i2];
;CHECK: %mul50 = (%.TempArray7)[0][i2];
;CHECK: %div4 = (%.TempArray1)[0][i2];
;CHECK: %mul70 = (%.TempArray5)[0][i2];

;Module Before HIR; ModuleID = 'eff.c'
source_filename = "eff.c"
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
  store float 0.000000e+00, float* @e_bond, align 4, !tbaa !2
  %conv = sext i32 %N to i64
  %cmp143 = icmp sgt i32 %N, 0
  br i1 %cmp143, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %add38145 = phi float [ %add38, %for.body ], [ 0.000000e+00, %for.body.preheader ]
  %i.0144 = phi i64 [ %inc, %for.body ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds [1000 x i64], [1000 x i64]* @a1, i64 0, i64 %i.0144
  %0 = load i64, i64* %arrayidx, align 8, !tbaa !6
  %mul = shl nsw i64 %0, 2
  %div = sdiv i64 %mul, 3
  %arrayidx2 = getelementptr inbounds [1000 x i64], [1000 x i64]* @a2, i64 0, i64 %i.0144
  %1 = load i64, i64* %arrayidx2, align 8, !tbaa !6
  %mul3 = shl nsw i64 %1, 2
  %div4 = sdiv i64 %mul3, 3
  %arrayidx5 = getelementptr inbounds [1000 x i64], [1000 x i64]* @atype, i64 0, i64 %i.0144
  %2 = load i64, i64* %arrayidx5, align 8, !tbaa !6
  %sub = add nsw i64 %2, -1
  %arrayidx6 = getelementptr inbounds [1000 x float], [1000 x float]* @x, i64 0, i64 %div
  %3 = load float, float* %arrayidx6, align 4, !tbaa !9
  %arrayidx7 = getelementptr inbounds [1000 x float], [1000 x float]* @x, i64 0, i64 %div4
  %4 = load float, float* %arrayidx7, align 4, !tbaa !9
  %sub8 = fsub float %3, %4
  %add = add nsw i64 %div, 1
  %arrayidx9 = getelementptr inbounds [1000 x float], [1000 x float]* @x, i64 0, i64 %add
  %5 = load float, float* %arrayidx9, align 4, !tbaa !9
  %add10 = add nsw i64 %div4, 1
  %arrayidx11 = getelementptr inbounds [1000 x float], [1000 x float]* @x, i64 0, i64 %add10
  %6 = load float, float* %arrayidx11, align 4, !tbaa !9
  %sub12 = fsub float %5, %6
  %add13 = add nsw i64 %div, 2
  %arrayidx14 = getelementptr inbounds [1000 x float], [1000 x float]* @x, i64 0, i64 %add13
  %7 = load float, float* %arrayidx14, align 4, !tbaa !9
  %add15 = add nsw i64 %div4, 2
  %arrayidx16 = getelementptr inbounds [1000 x float], [1000 x float]* @x, i64 0, i64 %add15
  %8 = load float, float* %arrayidx16, align 4, !tbaa !9
  %sub17 = fsub float %7, %8
  %add23 = add nsw i64 %div, 3
  %arrayidx24 = getelementptr inbounds [1000 x float], [1000 x float]* @x, i64 0, i64 %add23
  %9 = load float, float* %arrayidx24, align 4, !tbaa !9
  %add25 = add nsw i64 %div4, 3
  %arrayidx26 = getelementptr inbounds [1000 x float], [1000 x float]* @x, i64 0, i64 %add25
  %10 = load float, float* %arrayidx26, align 4, !tbaa !9
  %sub27 = fsub float %9, %10
  %arrayidx33 = getelementptr inbounds [1000 x float], [1000 x float]* @Req, i64 0, i64 %sub
  %11 = load float, float* %arrayidx33, align 4, !tbaa !9
  %sub34 = fsub float 1.000000e+00, %11
  %arrayidx35 = getelementptr inbounds [1000 x float], [1000 x float]* @Rk, i64 0, i64 %sub
  %12 = load float, float* %arrayidx35, align 4, !tbaa !9
  %mul36 = fmul float %12, %sub34
  %mul37 = fmul float %sub34, %mul36
  %add38 = fadd float %add38145, %mul37
  %mul39 = fmul float %mul36, 2.000000e+00
  %mul40 = fmul float %sub8, %mul39
  %add41 = add nsw i64 %div, %foff
  %arrayidx43 = getelementptr inbounds [1000 x float], [1000 x float]* @f, i64 0, i64 %add41
  %13 = load float, float* %arrayidx43, align 4, !tbaa !9
  %add44 = fadd float %13, %mul40
  store float %add44, float* %arrayidx43, align 4, !tbaa !9
  %mul45 = fmul float %sub12, %mul39
  %add47 = add nsw i64 %add41, 1
  %arrayidx48 = getelementptr inbounds [1000 x float], [1000 x float]* @f, i64 0, i64 %add47
  %14 = load float, float* %arrayidx48, align 4, !tbaa !9
  %add49 = fadd float %mul45, %14
  store float %add49, float* %arrayidx48, align 4, !tbaa !9
  %mul50 = fmul float %sub17, %mul39
  %add52 = add nsw i64 %add41, 2
  %arrayidx53 = getelementptr inbounds [1000 x float], [1000 x float]* @f, i64 0, i64 %add52
  %15 = load float, float* %arrayidx53, align 4, !tbaa !9
  %add54 = fadd float %mul50, %15
  store float %add54, float* %arrayidx53, align 4, !tbaa !9
  %add56 = add nsw i64 %div4, %foff
  %arrayidx58 = getelementptr inbounds [1000 x float], [1000 x float]* @f, i64 0, i64 %add56
  %16 = load float, float* %arrayidx58, align 4, !tbaa !9
  %sub59 = fsub float %16, %mul40
  store float %sub59, float* %arrayidx58, align 4, !tbaa !9
  %add62 = add nsw i64 %add56, 1
  %arrayidx63 = getelementptr inbounds [1000 x float], [1000 x float]* @f, i64 0, i64 %add62
  %17 = load float, float* %arrayidx63, align 4, !tbaa !9
  %sub64 = fsub float %17, %mul45
  store float %sub64, float* %arrayidx63, align 4, !tbaa !9
  %add67 = add nsw i64 %add56, 2
  %arrayidx68 = getelementptr inbounds [1000 x float], [1000 x float]* @f, i64 0, i64 %add67
  %18 = load float, float* %arrayidx68, align 4, !tbaa !9
  %sub69 = fsub float %18, %mul50
  store float %sub69, float* %arrayidx68, align 4, !tbaa !9
  %mul70 = fmul float %sub27, %mul39
  %add72 = add nsw i64 %add41, 3
  %arrayidx73 = getelementptr inbounds [1000 x float], [1000 x float]* @f, i64 0, i64 %add72
  %19 = load float, float* %arrayidx73, align 4, !tbaa !9
  %add74 = fadd float %mul70, %19
  store float %add74, float* %arrayidx73, align 4, !tbaa !9
  %add77 = add nsw i64 %add56, 3
  %arrayidx78 = getelementptr inbounds [1000 x float], [1000 x float]* @f, i64 0, i64 %add77
  %20 = load float, float* %arrayidx78, align 4, !tbaa !9
  %sub79 = fsub float %20, %mul70
  store float %sub79, float* %arrayidx78, align 4, !tbaa !9
  %inc = add nuw nsw i64 %i.0144, 1
  %exitcond = icmp eq i64 %inc, %conv
  br i1 %exitcond, label %for.cond.for.end_crit_edge, label %for.body

for.cond.for.end_crit_edge:                       ; preds = %for.body
  %add38.lcssa = phi float [ %add38, %for.body ]
  store float %add38.lcssa, float* @e_bond, align 4, !tbaa !2
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
