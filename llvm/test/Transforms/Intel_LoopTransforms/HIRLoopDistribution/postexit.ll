; Problem: Missing Postexit Stmt after Loop Dist
; RUN: opt -loop-simplify -hir-ssa-deconstruction -hir-loop-distribute -print-after=hir-loop-distribute -hir-loop-distribute-heuristics=nest < %s 2>&1 | FileCheck %s
; CHECK: END LOOP
; CHECK-NEXT:  %t132.0114 = %t132.3;
;
;          BEGIN REGION { }
;<81>         + DO i1 = 0, zext.i32.i64(((-1 * %0) + %1)), 1   <DO_LOOP>
;<5>          |   %conv41 = sitofp.i32.double(i1 + 1);
;<6>          |   %mul42 = %conv41  *  1.000000e-03;
;<8>          |   %t132.1111 = %t132.0114;
;<82>         |   + DO i2 = 0, 31, 1   <DO_LOOP>
;<83>         |   |   
;<16>         |   |      %conv43 = sitofp.i32.double(i2 + 1);
;<17>         |   |      %mul44 = %conv43  *  1.000000e-02;
;<18>         |   |      %add45 = %mul42  +  %mul44;
;<83>         |   |   + DO i3 = 0, zext.i32.i64((-2 + %1)), 1   <DO_LOOP>
;<27>         |   |   |   %11 = {al:4}(@t11)[0][i1 + %0 + %6 + -1][i2 + 1][i3 + sext.i32.i64(%t4)];
;<31>         |   |   |   %12 = {al:4}(@t11)[0][i1 + %0 + %6 + -1][i2 + 1][i3 + sext.i32.i64((-1 + %t4))];
;<32>         |   |   |   %sub24 = %11  -  %12;
;<33>         |   |   |   %conv25 = fpext.float.double(%sub24);
;<34>         |   |   |   %mul26 = %mul  *  %conv25;
;<35>         |   |   |   %conv27 = fptrunc.double.float(%mul26);
;<37>         |   |   |   {al:4}(@t15)[0][i1 + %0 + %6 + -1][i2 + 1][i3 + sext.i32.i64(%t4)] = %conv27;
;<38>         |   |   |   %t132.3 = %t4;
;<39>         |   |   |   if (%t4 > 0)
;<39>         |   |   |   {
;<44>         |   |   |      %conv46 = sitofp.i32.double(i3 + 1);
;<45>         |   |   |      %add47 = %add45  +  %conv46;
;<46>         |   |   |      %conv48 = fptrunc.double.float(%add47);
;<48>         |   |   |      {al:4}(@t12)[0][i1 + 1][i2 + 1][i3 + sext.i32.i64(%t4)] = %conv48;
;<49>         |   |   |      %t132.3 = i3 + %t4;
;<39>         |   |   |   }
;<83>         |   |   + END LOOP
;<62>         |   |      %t132.1111 = %t132.3;
;<83>         |   |   
;<65>         |   |   %t132.1111.out = %t132.1111;
;<82>         |   + END LOOP
;<77>         |   %t132.0114 = %t132.1111.out;
;<81>         + END LOOP
;          END REGION
; ModuleID = 'postexit.c'
source_filename = "postexit.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@hpo_lower = common global i32 0, align 4
@hpo_upper = common global i32 0, align 4
@hpo_prv = common global i32 0, align 4
@hpo_prv256 = common global i32 0, align 4
@hpo_prv132 = common global i32 0, align 4
@hpo_prv63 = common global i32 0, align 4
@hpo_prv103 = common global i32 0, align 4
@hpo_prv160 = common global i32 0, align 4
@hpo_liter = common global i32 0, align 4
@t11 = common global [32 x [64 x [128 x float]]] zeroinitializer, align 16
@t15 = common global [32 x [64 x [128 x float]]] zeroinitializer, align 16
@t12 = common global [32 x [64 x [128 x float]]] zeroinitializer, align 16
;
; Function Attrs: norecurse nounwind uwtable
define void @foo(i32 %t4) #0 {
entry:
  %0 = load i32, i32* @hpo_lower, align 4, !tbaa !1
  %1 = load i32, i32* @hpo_upper, align 4, !tbaa !1
  %2 = load i32, i32* @hpo_prv132, align 4, !tbaa !1
  %3 = load i32, i32* @hpo_liter, align 4, !tbaa !1
  %4 = icmp slt i32 %1, %0
  br i1 %4, label %if.end61, label %for.cond3.preheader.lr.ph

for.cond3.preheader.lr.ph:                        ; preds = %entry
  %5 = load i32, i32* @hpo_prv160, align 4, !tbaa !1
  %6 = load i32, i32* @hpo_prv103, align 4, !tbaa !1
  %cmp7108 = icmp sgt i32 %1, 1
  %add = add i32 %0, -2
  %add9 = add i32 %add, %6
  %conv = sitofp i32 %5 to double
  %mul = fmul double %conv, 1.010000e+00
  %cmp36 = icmp sgt i32 %t4, 0
  %7 = add i32 %1, 2
  %8 = sub i32 %7, %0
  br label %for.cond3.preheader

for.cond3.preheader:                              ; preds = %for.inc58, %for.cond3.preheader.lr.ph
  %indvars.iv119 = phi i64 [ %indvars.iv.next120, %for.inc58 ], [ 1, %for.cond3.preheader.lr.ph ]
  %t132.0114 = phi i32 [ %t132.2.lcssa, %for.inc58 ], [ %2, %for.cond3.preheader.lr.ph ]
  %9 = trunc i64 %indvars.iv119 to i32
  %sub10 = add i32 %add9, %9
  %idxprom14 = sext i32 %sub10 to i64
  %10 = trunc i64 %indvars.iv119 to i32
  %conv41 = sitofp i32 %10 to double
  %mul42 = fmul double %conv41, 1.000000e-03
  br label %for.cond6.preheader

for.cond6.preheader:                              ; preds = %for.inc55, %for.cond3.preheader
  %indvars.iv116 = phi i64 [ 1, %for.cond3.preheader ], [ %indvars.iv.next117, %for.inc55 ]
  %t132.1111 = phi i32 [ %t132.0114, %for.cond3.preheader ], [ %t132.2.lcssa, %for.inc55 ]
  br i1 %cmp7108, label %for.body8.lr.ph, label %for.inc55

for.body8.lr.ph:                                  ; preds = %for.cond6.preheader
  %11 = trunc i64 %indvars.iv116 to i32
  %conv43 = sitofp i32 %11 to double
  %mul44 = fmul double %conv43, 1.000000e-02
  %add45 = fadd double %mul42, %mul44
  br label %for.body8

for.body8:                                        ; preds = %if.end, %for.body8.lr.ph
  %indvars.iv = phi i64 [ 1, %for.body8.lr.ph ], [ %indvars.iv.next, %if.end ]
  %i3.0109 = phi i32 [ 1, %for.body8.lr.ph ], [ %inc, %if.end ]
  %add11 = add nsw i32 %i3.0109, %t4
  %sub12 = add nsw i32 %add11, -1
  %idxprom = sext i32 %sub12 to i64
  %arrayidx16 = getelementptr inbounds [32 x [64 x [128 x float]]], [32 x [64 x [128 x float]]]* @t11, i64 0, i64 %idxprom14, i64 %indvars.iv116, i64 %idxprom
  %12 = load float, float* %arrayidx16, align 4, !tbaa !5
  %sub17 = add nsw i32 %add11, -2
  %idxprom18 = sext i32 %sub17 to i64
  %arrayidx23 = getelementptr inbounds [32 x [64 x [128 x float]]], [32 x [64 x [128 x float]]]* @t11, i64 0, i64 %idxprom14, i64 %indvars.iv116, i64 %idxprom18
  %13 = load float, float* %arrayidx23, align 4, !tbaa !5
  %sub24 = fsub float %12, %13
  %conv25 = fpext float %sub24 to double
  %mul26 = fmul double %mul, %conv25
  %conv27 = fptrunc double %mul26 to float
  %arrayidx35 = getelementptr inbounds [32 x [64 x [128 x float]]], [32 x [64 x [128 x float]]]* @t15, i64 0, i64 %idxprom14, i64 %indvars.iv116, i64 %idxprom
  store float %conv27, float* %arrayidx35, align 4, !tbaa !5
  br i1 %cmp36, label %if.then38, label %if.end

if.then38:                                        ; preds = %for.body8
  %14 = trunc i64 %indvars.iv to i32
  %conv46 = sitofp i32 %14 to double
  %add47 = fadd double %add45, %conv46
  %conv48 = fptrunc double %add47 to float
  %arrayidx54 = getelementptr inbounds [32 x [64 x [128 x float]]], [32 x [64 x [128 x float]]]* @t12, i64 0, i64 %indvars.iv119, i64 %indvars.iv116, i64 %idxprom
  store float %conv48, float* %arrayidx54, align 4, !tbaa !5
  br label %if.end

if.end:                                           ; preds = %if.then38, %for.body8
  %t132.3 = phi i32 [ %sub12, %if.then38 ], [ %t4, %for.body8 ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %inc = add nuw nsw i32 %i3.0109, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %1
  br i1 %exitcond, label %for.inc55, label %for.body8

for.inc55:                                        ; preds = %if.end, %for.cond6.preheader
  %t132.2.lcssa = phi i32 [ %t132.1111, %for.cond6.preheader ], [ %t132.3, %if.end ]
  %indvars.iv.next117 = add nuw nsw i64 %indvars.iv116, 1
  %exitcond118 = icmp eq i64 %indvars.iv.next117, 33
  br i1 %exitcond118, label %for.inc58, label %for.cond6.preheader

for.inc58:                                        ; preds = %for.inc55
  %indvars.iv.next120 = add nuw nsw i64 %indvars.iv119, 1
  %lftr.wideiv121 = trunc i64 %indvars.iv.next120 to i32
  %exitcond122 = icmp eq i32 %lftr.wideiv121, %8
  br i1 %exitcond122, label %if.end61, label %for.cond3.preheader

if.end61:                                         ; preds = %for.inc58, %entry
  %t132.4 = phi i32 [ %2, %entry ], [ %t132.2.lcssa, %for.inc58 ]
  %cmp62 = icmp eq i32 %3, 0
  br i1 %cmp62, label %if.end65, label %if.then64

if.then64:                                        ; preds = %if.end61
  store i32 %t132.4, i32* @hpo_prv, align 4, !tbaa !1
  br label %if.end65

if.end65:                                         ; preds = %if.end61, %if.then64
  ret void
}

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.9.0 (trunk 9725) (llvm/branches/loopopt 9736)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
!5 = !{!6, !6, i64 0}
!6 = !{!"float", !3, i64 0}
