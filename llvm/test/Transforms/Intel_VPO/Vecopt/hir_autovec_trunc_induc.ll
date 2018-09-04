; LLVM IR generated from following testcase using clang -O1 -S -emit-llvm
; int sarr[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
; int arr[100];
; 
; void foo()
; {
;   int i;
; 
;   for (i = 0; i < 100; i++)
;     arr[i] = sarr[(i + 1) & 3];
; }
;   
; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -VPlanDriverHIR -hir-cg -S  < %s | FileCheck %s
; CHECK: llvm.masked.gather.v4i32
; CHECK-NEXT: store <4 x i32>
; 
; ModuleID = 'indconv.c'
source_filename = "indconv.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@sarr = local_unnamed_addr global [10 x i32] [i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9], align 16
@arr = common local_unnamed_addr global [100 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define void @foo() local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %and = and i64 %indvars.iv.next, 3
  %arrayidx = getelementptr inbounds [10 x i32], [10 x i32]* @sarr, i64 0, i64 %and
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %arrayidx2 = getelementptr inbounds [100 x i32], [100 x i32]* @arr, i64 0, i64 %indvars.iv
  store i32 %0, i32* %arrayidx2, align 4, !tbaa !1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.9.0 (trunk 15884) (llvm/branches/loopopt 16232)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
