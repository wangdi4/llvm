; RUN: opt -disable-output -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-reroll,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s

; Check following daxpy form can be rerolled.

; #define SIZE 10
; double x[SIZE];
; double y[SIZE];
;
; void foo(double a, int n) {
;   for (int i=0;  i<n; i=i+4) {
;     y[i]   += a * x[i];
;     y[i+1] += a * x[i+1];
;     y[i+2] += a * x[i+2];
;     y[i+3] += a * x[i+3];
;   }
; }

; CHECK: Function: foo
;
; CHECK:       BEGIN REGION { }
; CHECK:             + DO i1 = 0, (sext.i32.i64(%n) + -1)/u4, 1   <DO_LOOP>  <MAX_TC_EST = 2>
; CHECK:             |   %mul = (@x)[0][4 * i1]  *  %a;
; CHECK:             |   %add = (@y)[0][4 * i1]  +  %mul;
; CHECK:             |   (@y)[0][4 * i1] = %add;
; CHECK:             |   %mul6 = (@x)[0][4 * i1 + 1]  *  %a;
; CHECK:             |   %add10 = (@y)[0][4 * i1 + 1]  +  %mul6;
; CHECK:             |   (@y)[0][4 * i1 + 1] = %add10;
; CHECK:             |   %mul14 = (@x)[0][4 * i1 + 2]  *  %a;
; CHECK:             |   %add18 = (@y)[0][4 * i1 + 2]  +  %mul14;
; CHECK:             |   (@y)[0][4 * i1 + 2] = %add18;
; CHECK:             |   %mul22 = (@x)[0][4 * i1 + 3]  *  %a;
; CHECK:             |   %add26 = (@y)[0][4 * i1 + 3]  +  %mul22;
; CHECK:             |   (@y)[0][4 * i1 + 3] = %add26;
; CHECK:             + END LOOP
; CHECK:       END REGION
;
; CHECK: Function: foo
;
; CHECK:       BEGIN REGION { }
; CHECK:             + DO i1 = 0, 4 * ((3 + sext.i32.i64(%n)) /u 4) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 8>
; CHECK:             |   %mul = (@x)[0][i1]  *  %a;
; CHECK:             |   %add = (@y)[0][i1]  +  %mul;
; CHECK:             |   (@y)[0][i1] = %add;
; CHECK:             + END LOOP
; CHECK:       END REGION

; Check the opt report remarks of loop reroll.

; RUN: opt -disable-output -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-reroll,hir-cg,simplifycfg,intel-ir-optreport-emitter" -aa-pipeline="basic-aa" -intel-opt-report=low -force-hir-cg 2>&1 < %s | FileCheck %s -check-prefix=OPTREPORT

; OPTREPORT: LOOP BEGIN
; OPTREPORT:    remark #25264: Loop rerolled by 4
; OPTREPORT: LOOP END

;Module Before HIR; ModuleID = 'daxpy.c'
source_filename = "daxpy.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@x = common dso_local local_unnamed_addr global [10 x double] zeroinitializer, align 16
@y = common dso_local local_unnamed_addr global [10 x double] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local void @foo(double %a, i32 %n) local_unnamed_addr #0 {
entry:
  %cmp41 = icmp sgt i32 %n, 0
  br i1 %cmp41, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %0 = sext i32 %n to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [10 x double], ptr @x, i64 0, i64 %indvars.iv
  %1 = load double, ptr %arrayidx, align 16
  %mul = fmul double %1, %a
  %arrayidx2 = getelementptr inbounds [10 x double], ptr @y, i64 0, i64 %indvars.iv
  %2 = load double, ptr %arrayidx2, align 16
  %add = fadd double %2, %mul
  store double %add, ptr %arrayidx2, align 16
  %3 = or i64 %indvars.iv, 1
  %arrayidx5 = getelementptr inbounds [10 x double], ptr @x, i64 0, i64 %3
  %4 = load double, ptr %arrayidx5, align 8
  %mul6 = fmul double %4, %a
  %arrayidx9 = getelementptr inbounds [10 x double], ptr @y, i64 0, i64 %3
  %5 = load double, ptr %arrayidx9, align 8
  %add10 = fadd double %5, %mul6
  store double %add10, ptr %arrayidx9, align 8
  %6 = or i64 %indvars.iv, 2
  %arrayidx13 = getelementptr inbounds [10 x double], ptr @x, i64 0, i64 %6
  %7 = load double, ptr %arrayidx13, align 16
  %mul14 = fmul double %7, %a
  %arrayidx17 = getelementptr inbounds [10 x double], ptr @y, i64 0, i64 %6
  %8 = load double, ptr %arrayidx17, align 16
  %add18 = fadd double %8, %mul14
  store double %add18, ptr %arrayidx17, align 16
  %9 = or i64 %indvars.iv, 3
  %arrayidx21 = getelementptr inbounds [10 x double], ptr @x, i64 0, i64 %9
  %10 = load double, ptr %arrayidx21, align 8
  %mul22 = fmul double %10, %a
  %arrayidx25 = getelementptr inbounds [10 x double], ptr @y, i64 0, i64 %9
  %11 = load double, ptr %arrayidx25, align 8
  %add26 = fadd double %11, %mul22
  store double %add26, ptr %arrayidx25, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 4
  %cmp = icmp slt i64 %indvars.iv.next, %0
  br i1 %cmp, label %for.body, label %for.cond.cleanup.loopexit
}



