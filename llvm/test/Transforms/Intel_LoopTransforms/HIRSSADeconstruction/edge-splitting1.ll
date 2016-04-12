; RUN: opt < %s -hir-ssa-deconstruction -S | FileCheck %s

; Check that we split edges (%for.cond.1.preheader -> %cleanup) and (%for.cond.16 -> %cleanup) when deconstructing the SCC consisting of %ok.092, %0 and %ok.1.

; CHECK: for.cond.1.preheader.cleanup_crit_edge:
; CHECK-NEXT: %ok.092.in
; CHECK-NEXT: br label %cleanup

; CHECK: for.cond.16.cleanup_crit_edge:
; CHECK-NEXT: %ok.092.in1
; CHECK-NEXT: br label %cleanup


; ModuleID = 't.ll'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"


; Function Attrs: uwtable
define i32 @main(i8 %in, float *%p, float %f) #2 {
entry:
  br label %for.cond.1.preheader

for.cond.1.preheader:                             ; preds = %cleanup, %entry
  %i.093 = phi i64 [ 0, %entry ], [ %add.1, %cleanup ]
  %ok.092 = phi i8 [ 1, %entry ], [ %ok.1, %cleanup ]
  %arrayidx.1 = getelementptr inbounds float, float* %p, i64 %i.093
  store float %f, float* %arrayidx.1, align 4
  %add.1 = add nuw nsw i64 %i.093, 1
  %cmp37 = fcmp ogt float %f, 1.000000e+00
  br i1 %cmp37, label %cleanup, label %for.cond.16

for.cond.16:                                      ; preds = %for.cond.1.preheader
  %arrayidx.2 = getelementptr inbounds float, float* %p, i64 %add.1
  store float %f, float* %arrayidx.2, align 4
  %cmp37.1 = fcmp ogt float %f, 1.000000e+00
  br i1 %cmp37.1, label %cleanup, label %for.cond.16.1

for.cond.16.1:                                    ; preds = %for.cond.16
  store float %f, float* %arrayidx.2, align 4
  %cmp37.2 = fcmp ogt float %f, 1.000000e+00
  %0 = select i1 %cmp37.2, i8 0, i8 %ok.092
  br label %cleanup

cleanup:                                          ; preds = %for.cond.16.1, %for.cond.16, %for.cond.1.preheader
  %ok.1 = phi i8 [ 0, %for.cond.1.preheader ], [ 0, %for.cond.16 ], [ %0, %for.cond.16.1 ]
  %exitcond94 = icmp eq i64 %add.1, 3
  br i1 %exitcond94, label %if.else.i, label %for.cond.1.preheader

if.else.i:                                        ; preds = %cleanup
  %ok.1.lcssa = phi i8 [ %ok.1, %cleanup ]
  ret i32 0
}

