; Generated from following testcase using clang -O1 -S -emit-llvm
; void foo(int c2[100][100])
; {
;   int i, k;
; 
;   // Test Loop Interchange
;   for (i = 2; i < 89; i++) {
;     for (k = 32; k > 1; --k) {
;       c2[k][i-1] += c2[i][k+1];
;     }
;   }
; }
; 
; Check that we completely unroll inner loop and that no directive is added
; RUN: opt -S -hir-ssa-deconstruction -hir-post-vec-complete-unroll -hir-vec-dir-insert -print-before=hir-post-vec-complete-unroll -print-after=hir-post-vec-complete-unroll -print-after=hir-vec-dir-insert 2>&1 < %s | FileCheck %s
;
; CHECK: Dump Before HIR PostVec Complete Unroll
; CHECK: DO i1
; CHECK: DO i2


; CHECK: Dump After HIR PostVec Complete Unroll
; CHECK-NOT: DO i2

; CHECK: Dump After HIR Vec Directive Insertion
; CHECK-NOT: @llvm.intel.directive

; ModuleID = 't847_4.c'
source_filename = "t847_4.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @foo([100 x i32]* nocapture %c2) #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc11, %entry
  %indvars.iv25 = phi i64 [ 2, %entry ], [ %indvars.iv.next26, %for.inc11 ]
  %0 = add nsw i64 %indvars.iv25, -1
  br label %for.body3

for.body3:                                        ; preds = %for.cond1.preheader, %for.body3
  %indvars.iv = phi i64 [ 32, %for.cond1.preheader ], [ %indvars.iv.next, %for.body3 ]
  %1 = add nuw nsw i64 %indvars.iv, 1
  %arrayidx5 = getelementptr inbounds [100 x i32], [100 x i32]* %c2, i64 %indvars.iv25, i64 %1
  %2 = load i32, i32* %arrayidx5, align 4, !tbaa !1
  %arrayidx9 = getelementptr inbounds [100 x i32], [100 x i32]* %c2, i64 %indvars.iv, i64 %0
  %3 = load i32, i32* %arrayidx9, align 4, !tbaa !1
  %add10 = add nsw i32 %3, %2
  store i32 %add10, i32* %arrayidx9, align 4, !tbaa !1
  %indvars.iv.next = add nsw i64 %indvars.iv, -1
  %cmp2 = icmp sgt i64 %indvars.iv.next, 1
  br i1 %cmp2, label %for.body3, label %for.inc11

for.inc11:                                        ; preds = %for.body3
  %indvars.iv.next26 = add nuw nsw i64 %indvars.iv25, 1
  %exitcond = icmp eq i64 %indvars.iv.next26, 89
  br i1 %exitcond, label %for.end12, label %for.cond1.preheader

for.end12:                                        ; preds = %for.inc11
  ret void
}

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.9.0 (trunk 15842) (llvm/branches/loopopt 15863)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
