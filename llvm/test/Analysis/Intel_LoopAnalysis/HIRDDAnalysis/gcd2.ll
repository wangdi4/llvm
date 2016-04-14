;    for  (i=0; i < n; i++) {	
;        a[2 *n * i] =   a[2 *n * i+ 1] ; 
;    }

; RUN:  opt < %s  -loop-simplify  -hir-ssa-deconstruction | opt  -hir-dd-analysis  -hir-dd-analysis-verify=Region  -analyze  | FileCheck %s 
; CHECK: 'HIR Data Dependence Analysis' for function 'sub8'
; CHECK-NOT:  @a 

; ModuleID = 'gcd2.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = common global [100 x float] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define void @sub8(i64 %n) #0 {
entry:
  %cmp.11 = icmp sgt i64 %n, 0
  br i1 %cmp.11, label %for.body.lr.ph, label %for.end

for.body.lr.ph:                                   ; preds = %entry
  %mul = shl nsw i64 %n, 1
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %i.012 = phi i64 [ 0, %for.body.lr.ph ], [ %inc, %for.body ]
  %mul1 = mul nsw i64 %i.012, %mul
  %add = or i64 %mul1, 1
  %arrayidx = getelementptr inbounds [100 x float], [100 x float]* @a, i64 0, i64 %add
  %0 = bitcast float* %arrayidx to i32*
  %1 = load i32, i32* %0, align 4, !tbaa !1
  %arrayidx4 = getelementptr inbounds [100 x float], [100 x float]* @a, i64 0, i64 %mul1
  %2 = bitcast float* %arrayidx4 to i32*
  store i32 %1, i32* %2, align 8, !tbaa !1
  %inc = add nuw nsw i64 %i.012, 1
  %exitcond = icmp eq i64 %inc, %n
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body, %entry
  ret void
}

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.7.0 (trunk 1049) (llvm/branches/loopopt 1130)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"float", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
