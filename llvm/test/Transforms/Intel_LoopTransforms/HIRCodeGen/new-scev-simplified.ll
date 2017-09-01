;RUN: opt -hir-cg -force-hir-cg %s -S | FileCheck %s

;in cg for CE (-1 * %row.031 + umax((4 + %row.031), ((4 * sext.i32.i64(%n1)) 
;+ %row.031)) + -1)/u4
;SCEV expander is used for blobs. However, when the subscev 4 + %row.031 is 
;expanded, SCEVExpander replaces that add scev with a scev for 
;getelementpointer %row.031,1 and expands that. 

;CG assumes it can account for all SCEVUknowns as blobs. This is no longer
;true.

;
;          BEGIN REGION { }
;<10>         + DO i1 = 0, (-1 * %row.031 + umax((4 + %row.031), ((4 * sext.i32.i64(%n1)) + %row.031)) + -1oou4, 1   <DO_LOOP>
;<2>          |   %0 = (%row.031)[i1];
;<3>          |   %conv = fpext.float.double(%0);
;<4>          |   %sum.127 = %sum.127  +  %conv;
;<10>         + END LOOP
;          END REGION
;
;void
;calc_mean_2s(int n1, int n2, int w1, const float* in, float* p_mean)
;{
;  double sum = 0.0;
;  const float* row;
;  const float* elem;
;
;  for (row = in; row < in + w1 * n2; row += w1) {
;    for (elem = row; elem < row + n1; ++elem) {
;      sum += *elem;
;    }
;  }
;
;  *p_mean = sum / (n1 * n2);
;}
;
;using the umax portion of CE as test, ignoring rest of CE
;CHECK: region.0:
;other symbase shares same initial value, ignore it
;CHECK: store float* %row.031, float** [[ROW_SYM_DEAD:%t[0-9]+]]

;CHECK: store float* %row.031, float** [[ROW_SYM:%t[0-9]+]]

;t5 is load for -1*row multiplication, ignore it.
;CHECK: [[ROW_SYM_LD_DEAD:%.*]] = load float*, float** [[ROW_SYM]]

;Expander optimizes the following into a gep
;((4 * sext.i32.i64(%n1)) + %row.031)

;CHECK: [[ROW_SYM_LD:%.*]] = load float*, float** [[ROW_SYM]]
;CHECK: [[N1:%.*]] = sext i32 %n1 to i64
;CHECK: [[SCEV1:%.*]] = getelementptr float, float* [[ROW_SYM_LD]], i64 [[N1]]

;the following as well
;(4 + %row.031)
;CHECK: [[ROW_SYM_LD2:%.*]] = load float*, float** [[ROW_SYM]]
;CHECK: [[SCEV2:%.*]] = getelementptr float, float* [[ROW_SYM_LD2]], i64 1

;one is chosen based on earlier comparsion, the umax operation
;CHECK: select i1 {{.*}}, float* [[SCEV1]], float* [[SCEV2]]
;Module Before HIR; ModuleID = 'short.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @calc_mean_2s(i32 %n1, i32 %n2, i32 %w1, float* readonly %in, float* nocapture %p_mean) {
entry:
  %mul = mul nsw i32 %w1, %n2
  %idx.ext = sext i32 %mul to i64
  %add.ptr = getelementptr inbounds float, float* %in, i64 %idx.ext
  %cmp.29 = icmp sgt i32 %mul, 0
  br i1 %cmp.29, label %for.cond.1.preheader.lr.ph, label %for.end.9

for.cond.1.preheader.lr.ph:                       ; preds = %entry
  %idx.ext2 = sext i32 %n1 to i64
  %idx.ext7 = sext i32 %w1 to i64
  %cmp4.26 = icmp sgt i32 %n1, 0
  br label %for.cond.1.preheader

for.cond.1.preheader:                             ; preds = %for.cond.1.preheader.lr.ph, %for.inc.6
  %row.031 = phi float* [ %in, %for.cond.1.preheader.lr.ph ], [ %add.ptr8, %for.inc.6 ]
  %sum.030 = phi double [ 0.000000e+00, %for.cond.1.preheader.lr.ph ], [ %sum.1.lcssa, %for.inc.6 ]
  %add.ptr3 = getelementptr inbounds float, float* %row.031, i64 %idx.ext2
  br i1 %cmp4.26, label %for.body.5.preheader, label %for.inc.6

for.body.5.preheader:                             ; preds = %for.cond.1.preheader
  br label %for.body.5

for.body.5:                                       ; preds = %for.body.5.preheader, %for.body.5
  %elem.028 = phi float* [ %incdec.ptr, %for.body.5 ], [ %row.031, %for.body.5.preheader ]
  %sum.127 = phi double [ %add, %for.body.5 ], [ %sum.030, %for.body.5.preheader ]
  %0 = load float, float* %elem.028, align 4
  %conv = fpext float %0 to double
  %add = fadd double %sum.127, %conv
  %incdec.ptr = getelementptr inbounds float, float* %elem.028, i64 1
  %cmp4 = icmp ult float* %incdec.ptr, %add.ptr3
  br i1 %cmp4, label %for.body.5, label %for.inc.6.loopexit

for.inc.6.loopexit:                               ; preds = %for.body.5
  %add.lcssa = phi double [ %add, %for.body.5 ]
  br label %for.inc.6

for.inc.6:                                        ; preds = %for.inc.6.loopexit, %for.cond.1.preheader
  %sum.1.lcssa = phi double [ %sum.030, %for.cond.1.preheader ], [ %add.lcssa, %for.inc.6.loopexit ]
  %add.ptr8 = getelementptr inbounds float, float* %row.031, i64 %idx.ext7
  %cmp = icmp ult float* %add.ptr8, %add.ptr
  br i1 %cmp, label %for.cond.1.preheader, label %for.end.9.loopexit

for.end.9.loopexit:                               ; preds = %for.inc.6
  %sum.1.lcssa.lcssa = phi double [ %sum.1.lcssa, %for.inc.6 ]
  br label %for.end.9

for.end.9:                                        ; preds = %for.end.9.loopexit, %entry
  %sum.0.lcssa = phi double [ 0.000000e+00, %entry ], [ %sum.1.lcssa.lcssa, %for.end.9.loopexit ]
  %mul10 = mul nsw i32 %n2, %n1
  %conv11 = sitofp i32 %mul10 to double
  %div = fdiv double %sum.0.lcssa, %conv11
  %conv12 = fptrunc double %div to float
  store float %conv12, float* %p_mean, align 4
  ret void
}

