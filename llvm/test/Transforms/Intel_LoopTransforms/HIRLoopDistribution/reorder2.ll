;    for (i=0; i<51; i++) {
;			a[100 -2 *i ] += 1;
;			a[50  -i  ] +=  i+2; }
;   dist should not happen.  the DV is (<=) 
;   
;RUN: opt -hir-ssa-deconstruction -hir-loop-distribute -S -print-after=hir-loop-distribute -hir-loop-distribute-heuristics=mem-rec  < %s 2>&1 | FileCheck %s
;
;Explicitly check contents of first loop
; CHECK: DO i1 = 0, 50, 1   
; CHECK-NEXT:  %2 = (@a)[0][-2 * i1 + 100];
; CHECK-NEXT:  %add = %2  +  1.000000e+00;
; CHECK-NEXT: (@a)[0][-2 * i1 + 100] = %add;
; CHECK-NEXT: %conv = sitofp.i32.float(i1 + 2);
; CHECK-NEXT:  %6 = (@a)[0][-1 * i1 + 50];
;  no need to compare all 
; ModuleID = 'reorder2.c'
source_filename = "reorder2.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = common local_unnamed_addr global [1000 x float] zeroinitializer, align 16
@b = common local_unnamed_addr global [1000 x float] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define i32 @sub(i32 %n) local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %0 = shl nsw i64 %indvars.iv, 1
  %1 = sub nuw nsw i64 100, %0
  %arrayidx = getelementptr inbounds [1000 x float], [1000 x float]* @a, i64 0, i64 %1
  %2 = load float, float* %arrayidx, align 8, !tbaa !1
  %add = fadd float %2, 1.000000e+00
  store float %add, float* %arrayidx, align 8, !tbaa !1
  %3 = add nuw nsw i64 %indvars.iv, 2
  %4 = trunc i64 %3 to i32
  %conv = sitofp i32 %4 to float
  %5 = sub nuw nsw i64 50, %indvars.iv
  %arrayidx4 = getelementptr inbounds [1000 x float], [1000 x float]* @a, i64 0, i64 %5
  %6 = load float, float* %arrayidx4, align 4, !tbaa !1
  %add5 = fadd float %conv, %6
  store float %add5, float* %arrayidx4, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 51
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret i32 0
}

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.9.0 (trunk 17939) (llvm/branches/loopopt 17975)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"float", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
