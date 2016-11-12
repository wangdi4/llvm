; Check that edges are not accumulated after getGraph().

;    for  (i=0; i< 1000; i++) {	
;        a[2 *n * i] =   a[2 *n * i+ 1] ; 
;    }

; RUN:  opt < %s -hir-ssa-deconstruction | opt -hir-loop-distribute-memrec -hir-dd-analysis -analyze  | FileCheck %s 

; CHECK: 'HIR Data Dependence Analysis' for function 'sub8'
; CHECK-DAG: FLOW
; CHECK-DAG: OUTPUT
; CHECK-NOT: -->
 
; ModuleID = 'gcd1.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = common global [100 x float] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define void @sub8(i64 %n) #0 {
entry:
  %mul = shl nsw i64 %n, 1
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %i.010 = phi i64 [ 0, %entry ], [ %inc, %for.body ]
  %mul1 = mul nsw i64 %i.010, %mul
  %add = or i64 %mul1, 1
  %arrayidx = getelementptr inbounds [100 x float], [100 x float]* @a, i64 0, i64 %add
  %0 = bitcast float* %arrayidx to i32*
  %1 = load i32, i32* %0, align 4
  %arrayidx4 = getelementptr inbounds [100 x float], [100 x float]* @a, i64 0, i64 %mul1
  %2 = bitcast float* %arrayidx4 to i32*
  store i32 %1, i32* %2, align 8
  %inc = add nuw nsw i64 %i.010, 1
  %exitcond = icmp eq i64 %inc, 1000
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

