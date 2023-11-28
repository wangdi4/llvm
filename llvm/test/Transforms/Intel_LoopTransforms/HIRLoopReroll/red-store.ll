; RUN: opt -disable-output -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-reroll,print<hir>" -aa-pipeline="basic-aa" -hir-verify-cf-def-level < %s 2>&1 | FileCheck %s

; Mix of store and reduction instruction

;#define N 100
;double A[N];
;double B[N];
;double C[N];
;
;double foo() {
;  double S = 0;
;  for (int i = 0; i < N ; i=i+2) {
;    A[i] = C[i]* B[i];
;    S += A[i];
;    A[i+1] = C[i+1]*B[i+1];
;    S += A[i+1];
;  }
;  return S;
;}

; CHECK:Function: foo

; CHECK:        BEGIN REGION { }
; CHECK:              + DO i1 = 0, 49, 1   <DO_LOOP>
; CHECK:              |   %mul = (@C)[0][2 * i1]  *  (@B)[0][2 * i1];
; CHECK:              |   (@A)[0][2 * i1] = %mul;
; CHECK:              |   %add = %S.035  +  %mul;
; CHECK:              |   %mul13 = (@C)[0][2 * i1 + 1]  *  (@B)[0][2 * i1 + 1];
; CHECK:              |   (@A)[0][2 * i1 + 1] = %mul13;
; CHECK:              |   %S.035 = %add  +  %mul13;
; CHECK:              + END LOOP
; CHECK:        END REGION

; CHECK:Function: foo

; CHECK:        BEGIN REGION { }
; CHECK:              + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:              |   %mul = (@C)[0][i1]  *  (@B)[0][i1];
; CHECK:              |   (@A)[0][i1] = %mul;
; CHECK:              |   %S.035 = %S.035  +  %mul;
; CHECK:              + END LOOP
; CHECK:        END REGION

;Module Before HIR
; ModuleID = 'store-red-3.c'
source_filename = "store-red-3.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@C = common dso_local local_unnamed_addr global [100 x double] zeroinitializer, align 16
@B = common dso_local local_unnamed_addr global [100 x double] zeroinitializer, align 16
@A = common dso_local local_unnamed_addr global [100 x double] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local double @foo() local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  %add20.lcssa = phi double [ %add20, %for.body ]
  ret double %add20.lcssa

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %S.035 = phi double [ 0.000000e+00, %entry ], [ %add20, %for.body ]
  %arrayidx = getelementptr inbounds [100 x double], ptr @C, i64 0, i64 %indvars.iv
  %0 = load double, ptr %arrayidx, align 16
  %arrayidx2 = getelementptr inbounds [100 x double], ptr @B, i64 0, i64 %indvars.iv
  %1 = load double, ptr %arrayidx2, align 16
  %mul = fmul double %0, %1
  %arrayidx4 = getelementptr inbounds [100 x double], ptr @A, i64 0, i64 %indvars.iv
  store double %mul, ptr %arrayidx4, align 16
  %add = fadd double %S.035, %mul
  %2 = or i64 %indvars.iv, 1
  %arrayidx9 = getelementptr inbounds [100 x double], ptr @C, i64 0, i64 %2
  %3 = load double, ptr %arrayidx9, align 8
  %arrayidx12 = getelementptr inbounds [100 x double], ptr @B, i64 0, i64 %2
  %4 = load double, ptr %arrayidx12, align 8
  %mul13 = fmul double %3, %4
  %arrayidx16 = getelementptr inbounds [100 x double], ptr @A, i64 0, i64 %2
  store double %mul13, ptr %arrayidx16, align 8
  %add20 = fadd double %add, %mul13
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 2
  %cmp = icmp ult i64 %indvars.iv.next, 100
  br i1 %cmp, label %for.body, label %for.cond.cleanup
}



