
;RUN: opt -hir-ssa-deconstruction -hir-loop-distribute-loopnest -print-after=hir-loop-distribute-loopnest  < %s 2>&1 | FileCheck %s
;RUN: opt -passes="hir-ssa-deconstruction,hir-loop-distribute-loopnest,print<hir>" -aa-pipeline="basic-aa"  < %s 2>&1 | FileCheck %s
; matmul required suppressions of LICM and early CSE(did ld/st of c[][] in i3)
;          BEGIN REGION { }
;<35>         + DO i1 = 0, 99999, 1   <DO_LOOP>
;<34>         |   + DO i2 = 0, 99999, 1   <DO_LOOP>
;<5>          |   |   (@C)[0][i1][i2] = 0.000000e+00;
;<33>         |   |   + DO i3 = 0, 99999, 1   <DO_LOOP>
;<9>          |   |   |   %0 = (@A)[0][i1][i3];
;<11>         |   |   |   %1 = (@B)[0][i3][i2];
;<12>         |   |   |   %mul = %0  *  %1;
;<13>         |   |   |   %2 = (@C)[0][i1][i2];
;<14>         |   |   |   %add = %2  +  %mul;
;<15>         |   |   |   (@C)[0][i1][i2] = %add;
;<33>         |   |   + END LOOP
;<34>         |   + END LOOP
;<35>         + END LOOP
;          END REGION

; CHECK: BEGIN REGION { }
; check first loop nest has only initialization of C[][]
; CHECK: DO i1 = 0, 99999, 1
; CHECK: DO i2 = 0, 99999, 1
; CHECK: (@C)[0][i1][i2] = 0.000000e+00;
; CHECK-NEXT: END LOOP
; CHECK-NEXT: END LOOP

; second loop has only matmul code
; CHECK: DO i1 = 0, 99999, 1
; CHECK-NEXT: DO i2 = 0, 99999, 1
; CHECK-NEXT: DO i3 = 0, 99999, 1
; CHECK-DAG: [[A_LD:%.*]] = (@A)[0][i1][i3]
; CHECK-DAG: [[B_LD:%.*]] = (@B)[0][i3][i2]
; CHECK-NEXT: [[MUL:%.*]] = [[A_LD]] * [[B_LD]]
; CHECK-NEXT: [[C_LD:%.*]] = (@C)[0][i1][i2]
; CHECK-NEXT: [[ADD:%.*]] = [[C_LD]] + [[MUL]]
; CHECK-NEXT: (@C)[0][i1][i2] = [[ADD]]
; CHECK-NEXT: END LOOP
; CHECK-NEXT: END LOOP
; CHECK-NEXT: END LOOP
; CHECK: END REGION

;RUN: opt -loop-simplify -hir-ssa-deconstruction -hir-loop-distribute-loopnest -hir-cg -force-hir-cg -intel-loop-optreport=low -simplifycfg -intel-ir-optreport-emitter 2>&1 < %s -S | FileCheck %s  -check-prefix=OPTREPORT
;RUN: opt -passes="loop-simplify,hir-ssa-deconstruction,hir-loop-distribute-loopnest,hir-cg,simplify-cfg,intel-ir-optreport-emitter" -aa-pipeline="basic-aa" -force-hir-cg -intel-loop-optreport=low 2>&1 < %s -S | FileCheck %s  -check-prefix=OPTREPORT
;
;OPTREPORT: LOOP BEGIN
;OPTREPORT: <Distributed chunk 1>
;OPTREPORT:     Remark: Loop distributed (2 way)
;OPTREPORT:     LOOP BEGIN
;OPTREPORT:     <Distributed chunk 1>
;OPTREPORT:         Remark: Loop distributed (2 way)
;OPTREPORT:     LOOP END
;OPTREPORT: LOOP END
;OPTREPORT: LOOP BEGIN
;OPTREPORT: <Distributed chunk 2>
;OPTREPORT:     LOOP BEGIN
;OPTREPORT:     <Distributed chunk 2>
;OPTREPORT:         LOOP BEGIN
;OPTREPORT:         LOOP END
;OPTREPORT:     LOOP END
;OPTREPORT: LOOP END

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@C = common global [100000 x [100000 x float]] zeroinitializer, align 16
@A = common global [100000 x [100000 x float]] zeroinitializer, align 16
@B = common global [100000 x [100000 x float]] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define void @matrix_mul_matrix() {
entry:
  br label %for.cond.1.preheader

for.cond.1.preheader:                             ; preds = %for.inc.24, %entry
  %indvars.iv48 = phi i64 [ 0, %entry ], [ %indvars.iv.next49, %for.inc.24 ]
  br label %for.body.3

for.body.3:                                       ; preds = %for.inc.21, %for.cond.1.preheader
  %indvars.iv44 = phi i64 [ 0, %for.cond.1.preheader ], [ %indvars.iv.next45, %for.inc.21 ]
  %arrayidx5 = getelementptr inbounds [100000 x [100000 x float]], [100000 x [100000 x float]]* @C, i64 0, i64 %indvars.iv48, i64 %indvars.iv44
  store float 0.000000e+00, float* %arrayidx5, align 4
  br label %for.body.8

for.body.8:                                       ; preds = %for.body.8, %for.body.3
  %indvars.iv = phi i64 [ 0, %for.body.3 ], [ %indvars.iv.next, %for.body.8 ]
  %arrayidx12 = getelementptr inbounds [100000 x [100000 x float]], [100000 x [100000 x float]]* @A, i64 0, i64 %indvars.iv48, i64 %indvars.iv
  %0 = load float, float* %arrayidx12, align 4
  %arrayidx16 = getelementptr inbounds [100000 x [100000 x float]], [100000 x [100000 x float]]* @B, i64 0, i64 %indvars.iv, i64 %indvars.iv44
  %1 = load float, float* %arrayidx16, align 4
  %mul = fmul float %0, %1
  %2 = load float, float* %arrayidx5, align 4
  %add = fadd float %2, %mul
  store float %add, float* %arrayidx5, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100000
  br i1 %exitcond, label %for.inc.21, label %for.body.8

for.inc.21:                                       ; preds = %for.body.8
  %indvars.iv.next45 = add nuw nsw i64 %indvars.iv44, 1
  %exitcond46 = icmp eq i64 %indvars.iv.next45, 100000
  br i1 %exitcond46, label %for.inc.24, label %for.body.3

for.inc.24:                                       ; preds = %for.inc.21
  %indvars.iv.next49 = add nuw nsw i64 %indvars.iv48, 1
  %exitcond50 = icmp eq i64 %indvars.iv.next49, 100000
  br i1 %exitcond50, label %for.end.26, label %for.cond.1.preheader

for.end.26:                                       ; preds = %for.inc.24
  ret void
}

; Function Attrs: nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture)

; Function Attrs: nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture)

