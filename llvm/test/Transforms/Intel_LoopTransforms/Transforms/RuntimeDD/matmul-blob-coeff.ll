; Check for dd multiversioning for matmul loopnest

; RUN: opt -hir-ssa-deconstruction -hir-runtime-dd -print-after=hir-runtime-dd -S < %s 2>&1 | FileCheck %s

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

; CHECK: IR Dump After
; CHECK: if (&((%vy)[25 * %n + -1]) >= &((%px)[0]) && &((%px)[%n + 599]) >= &((%vy)[0]))
; CHECK: if (&((%cx)[624]) >= &((%px)[0]) && &((%px)[%n + 599]) >= &((%cx)[0]))

; ModuleID = 'matmul-blob-coeff.ll'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define float @matmul(float* %px, float* %vy, float* %cx, i32 %n, i32 %loop) #0 {
entry:
  %cmp6 = icmp sle i32 1, %loop
  br i1 %cmp6, label %for.body.lr.ph, label %for.end28

for.body.lr.ph:                                   ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.lr.ph, %for.inc26
  %l.07 = phi i32 [ 1, %for.body.lr.ph ], [ %inc27, %for.inc26 ]
  br label %for.body3

for.body3:                                        ; preds = %for.body, %for.inc23
  %k.04 = phi i32 [ 0, %for.body ], [ %inc24, %for.inc23 ]
  %cmp52 = icmp slt i32 0, %n
  br i1 %cmp52, label %for.body6.lr.ph, label %for.end22

for.body6.lr.ph:                                  ; preds = %for.body3
  br label %for.body6

for.body6:                                        ; preds = %for.body6.lr.ph, %for.inc20
  %i.03 = phi i32 [ 0, %for.body6.lr.ph ], [ %inc21, %for.inc20 ]
  br label %for.body9

for.body9:                                        ; preds = %for.body6, %for.inc
  %j.01 = phi i32 [ 0, %for.body6 ], [ %inc, %for.inc ]
  %mul = mul nsw i32 %k.04, %n
  %add = add nsw i32 %mul, %i.03
  %idxprom = sext i32 %add to i64
  %arrayidx = getelementptr inbounds float, float* %vy, i64 %idxprom
  %0 = load float, float* %arrayidx, align 4
  %mul10 = mul nsw i32 %j.01, 25
  %add11 = add nsw i32 %mul10, %k.04
  %idxprom12 = sext i32 %add11 to i64
  %arrayidx13 = getelementptr inbounds float, float* %cx, i64 %idxprom12
  %1 = load float, float* %arrayidx13, align 4
  %mul14 = fmul float %0, %1
  %mul15 = mul nsw i32 %j.01, 25
  %add16 = add nsw i32 %mul15, %i.03
  %idxprom17 = sext i32 %add16 to i64
  %arrayidx18 = getelementptr inbounds float, float* %px, i64 %idxprom17
  %2 = load float, float* %arrayidx18, align 4
  %add19 = fadd float %2, %mul14
  store float %add19, float* %arrayidx18, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body9
  %inc = add nsw i32 %j.01, 1
  %cmp8 = icmp slt i32 %inc, 25
  br i1 %cmp8, label %for.body9, label %for.end

for.end:                                          ; preds = %for.inc
  br label %for.inc20

for.inc20:                                        ; preds = %for.end
  %inc21 = add nsw i32 %i.03, 1
  %cmp5 = icmp slt i32 %inc21, %n
  br i1 %cmp5, label %for.body6, label %for.cond4.for.end22_crit_edge

for.cond4.for.end22_crit_edge:                    ; preds = %for.inc20
  br label %for.end22

for.end22:                                        ; preds = %for.cond4.for.end22_crit_edge, %for.body3
  br label %for.inc23

for.inc23:                                        ; preds = %for.end22
  %inc24 = add nsw i32 %k.04, 1
  %cmp2 = icmp slt i32 %inc24, 25
  br i1 %cmp2, label %for.body3, label %for.end25

for.end25:                                        ; preds = %for.inc23
  br label %for.inc26

for.inc26:                                        ; preds = %for.end25
  %inc27 = add nsw i32 %l.07, 1
  %cmp = icmp sle i32 %inc27, %loop
  br i1 %cmp, label %for.body, label %for.cond.for.end28_crit_edge

for.cond.for.end28_crit_edge:                     ; preds = %for.inc26
  br label %for.end28

for.end28:                                        ; preds = %for.cond.for.end28_crit_edge, %entry
  %arrayidx29 = getelementptr inbounds float, float* %px, i64 0
  %3 = load float, float* %arrayidx29, align 4
  ret float %3
}

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (trunk 2039) (llvm/branches/loopopt 2056)"}
