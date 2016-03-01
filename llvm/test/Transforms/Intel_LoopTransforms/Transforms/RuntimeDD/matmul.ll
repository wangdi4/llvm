; Check for dd multiversioning for matmul loopnest

; RUN: opt -hir-ssa-deconstruction -hir-runtime-dd -print-after=hir-runtime-dd -S < %s 2>&1 | FileCheck %s

; float matmul(e_fp *px, e_fp *vy, e_fp *cx, int n, int loop) {
;  float ret;
;  for ( l=1 ; l<=loop ; l++ ) {
;   for ( k=0 ; k<25 ; k++ ) {
;    for ( i=0 ; i<25 ; i++ ) {
;     for ( j=0 ; j<25 ; j++ ) {
;      px[j*25+i] += vy[25*k+i] * cx[j*25+k];
;      }
;     }
;    }
;   }
;  return px[0];
; }

; CHECK: IR Dump After
; CHECK: if (&((%vy)[624]) >= &((%px)[0]) && &((%px)[624]) >= &((%vy)[0])) 
; CHECK: if (&((%cx)[624]) >= &((%px)[0]) && &((%px)[624]) >= &((%cx)[0]))

; ModuleID = 'mat-mul.ll'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define float @matmul(float* %px, float* %vy, float* %cx, i32 %n, i32 %loop) {
entry:
  %cmp.4 = icmp sle i32 1, %loop
  br i1 %cmp.4, label %for.body.lr.ph, label %for.end.28

for.body.lr.ph:                                   ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.lr.ph, %for.inc.26
  %l.05 = phi i32 [ 1, %for.body.lr.ph ], [ %inc27, %for.inc.26 ]
  br label %for.body.3

for.body.3:                                       ; preds = %for.body, %for.inc.23
  %k.03 = phi i32 [ 0, %for.body ], [ %inc24, %for.inc.23 ]
  br label %for.body.6

for.body.6:                                       ; preds = %for.body.3, %for.inc.20
  %i.02 = phi i32 [ 0, %for.body.3 ], [ %inc21, %for.inc.20 ]
  br label %for.body.9

for.body.9:                                       ; preds = %for.body.6, %for.inc
  %j.01 = phi i32 [ 0, %for.body.6 ], [ %inc, %for.inc ]
  %mul = mul nsw i32 %k.03, 25
  %add = add nsw i32 %mul, %i.02
  %idxprom = sext i32 %add to i64
  %arrayidx = getelementptr inbounds float, float* %vy, i64 %idxprom
  %0 = load float, float* %arrayidx, align 4
  %mul10 = mul nsw i32 %j.01, 25
  %add11 = add nsw i32 %mul10, %k.03
  %idxprom12 = sext i32 %add11 to i64
  %arrayidx13 = getelementptr inbounds float, float* %cx, i64 %idxprom12
  %1 = load float, float* %arrayidx13, align 4
  %mul14 = fmul float %0, %1
  %mul15 = mul nsw i32 %j.01, 25
  %add16 = add nsw i32 %mul15, %i.02
  %idxprom17 = sext i32 %add16 to i64
  %arrayidx18 = getelementptr inbounds float, float* %px, i64 %idxprom17
  %2 = load float, float* %arrayidx18, align 4
  %add19 = fadd float %2, %mul14
  store float %add19, float* %arrayidx18, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body.9
  %inc = add nsw i32 %j.01, 1
  %cmp8 = icmp slt i32 %inc, 25
  br i1 %cmp8, label %for.body.9, label %for.end

for.end:                                          ; preds = %for.inc
  br label %for.inc.20

for.inc.20:                                       ; preds = %for.end
  %inc21 = add nsw i32 %i.02, 1
  %cmp5 = icmp slt i32 %inc21, 25
  br i1 %cmp5, label %for.body.6, label %for.end.22

for.end.22:                                       ; preds = %for.inc.20
  br label %for.inc.23

for.inc.23:                                       ; preds = %for.end.22
  %inc24 = add nsw i32 %k.03, 1
  %cmp2 = icmp slt i32 %inc24, 25
  br i1 %cmp2, label %for.body.3, label %for.end.25

for.end.25:                                       ; preds = %for.inc.23
  br label %for.inc.26

for.inc.26:                                       ; preds = %for.end.25
  %inc27 = add nsw i32 %l.05, 1
  %cmp = icmp sle i32 %inc27, %loop
  br i1 %cmp, label %for.body, label %for.cond.for.end.28_crit_edge

for.cond.for.end.28_crit_edge:                    ; preds = %for.inc.26
  br label %for.end.28

for.end.28:                                       ; preds = %for.cond.for.end.28_crit_edge, %entry
  %arrayidx29 = getelementptr inbounds float, float* %px, i64 0
  %3 = load float, float* %arrayidx29, align 4
  ret float %3
}

