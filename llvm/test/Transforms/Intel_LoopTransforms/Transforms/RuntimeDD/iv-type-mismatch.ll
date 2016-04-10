; Check if there is an IV-based condition outside of the loop

; float matmul(float *px, float *vy, float *cx, int n, int loop) {
;   int i,j,k,l;
;   float ret;
;   for ( l=1 ; l<=loop ; l++ ) {
;     for ( k=0 ; k<25 ; k++ ) {
;       for ( i=0 ; i<n ; i++ ) {
;         for ( j=0 ; j<25 ; j++ ) {
;           px[j*25+i] += vy[k*n+i] * cx[j*25+k];
;         }
;       }
;     }
;   }
;   return px[0];
; }

; RUN: opt -hir-ssa-deconstruction -hir-runtime-dd -print-after=hir-runtime-dd -S < %s 2>&1 | FileCheck %s

; CHECK: IR Dump After
; CHECK-NOT: i1
; CHECK-NOT: i2
; CHECK-NOT: i3
; CHECK: + DO i1 = 0, %loop + -1, 1   <DO_LOOP>

; ModuleID = 'iv-type-mismatch.ll'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define float @matmul(float* %px, float* %vy, float* %cx, i32 %n, i32 %loop) #0 {
entry:
  %cmp6 = icmp sle i32 1, %loop
  br i1 %cmp6, label %for.body.lr.ph, label %for.end28

for.body.lr.ph:                                   ; preds = %entry
  %0 = sext i32 %n to i64
  %1 = add i32 %loop, 1
  br label %for.body

for.body:                                         ; preds = %for.inc26, %for.body.lr.ph
  %l.07 = phi i32 [ 1, %for.body.lr.ph ], [ %inc27, %for.inc26 ]
  br label %for.body3

for.body3:                                        ; preds = %for.inc23, %for.body
  %indvars.iv9 = phi i64 [ %indvars.iv.next10, %for.inc23 ], [ 0, %for.body ]
  %cmp52 = icmp slt i32 0, %n
  br i1 %cmp52, label %for.body6.lr.ph, label %for.end22

for.body6.lr.ph:                                  ; preds = %for.body3
  br label %for.body6

for.body6:                                        ; preds = %for.inc20, %for.body6.lr.ph
  %indvars.iv5 = phi i64 [ %indvars.iv.next6, %for.inc20 ], [ 0, %for.body6.lr.ph ]
  br label %for.body9

for.body9:                                        ; preds = %for.inc, %for.body6
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.inc ], [ 0, %for.body6 ]
  %2 = mul nsw i64 %indvars.iv9, %0
  %3 = add nsw i64 %2, %indvars.iv5
  %arrayidx = getelementptr inbounds float, float* %vy, i64 %3
  %4 = load float, float* %arrayidx, align 4
  %5 = mul nuw nsw i64 %indvars.iv, 25
  %6 = add nuw nsw i64 %5, %indvars.iv9
  %arrayidx13 = getelementptr inbounds float, float* %cx, i64 %6
  %7 = load float, float* %arrayidx13, align 4
  %mul14 = fmul float %4, %7
  %8 = mul nuw nsw i64 %indvars.iv, 25
  %9 = add nuw nsw i64 %8, %indvars.iv5
  %arrayidx18 = getelementptr inbounds float, float* %px, i64 %9
  %10 = load float, float* %arrayidx18, align 4
  %add19 = fadd float %10, %mul14
  store float %add19, float* %arrayidx18, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body9
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp ne i64 %indvars.iv.next, 25
  br i1 %exitcond, label %for.body9, label %for.end

for.end:                                          ; preds = %for.inc
  br label %for.inc20

for.inc20:                                        ; preds = %for.end
  %indvars.iv.next6 = add nuw nsw i64 %indvars.iv5, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next6 to i32
  %exitcond8 = icmp ne i32 %lftr.wideiv, %n
  br i1 %exitcond8, label %for.body6, label %for.cond4.for.end22_crit_edge

for.cond4.for.end22_crit_edge:                    ; preds = %for.inc20
  br label %for.end22

for.end22:                                        ; preds = %for.cond4.for.end22_crit_edge, %for.body3
  br label %for.inc23

for.inc23:                                        ; preds = %for.end22
  %indvars.iv.next10 = add nuw nsw i64 %indvars.iv9, 1
  %exitcond12 = icmp ne i64 %indvars.iv.next10, 25
  br i1 %exitcond12, label %for.body3, label %for.end25

for.end25:                                        ; preds = %for.inc23
  br label %for.inc26

for.inc26:                                        ; preds = %for.end25
  %inc27 = add nuw nsw i32 %l.07, 1
  %exitcond13 = icmp ne i32 %inc27, %1
  br i1 %exitcond13, label %for.body, label %for.cond.for.end28_crit_edge

for.cond.for.end28_crit_edge:                     ; preds = %for.inc26
  br label %for.end28

for.end28:                                        ; preds = %for.cond.for.end28_crit_edge, %entry
  %arrayidx29 = getelementptr inbounds float, float* %px, i64 0
  %11 = load float, float* %arrayidx29, align 4
  ret float %11
}

