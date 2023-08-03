; Test for loop carried flow dep where the canon expr associated with a ddref is a constant
; which should not cause DD edges to be skipped.
;
; LLVM IR generated from following testcase using clang -O1 -S -emit-llvm
;  void foo(int *arr1)
;  {
;    int i;
;    int i2;
;  
;    i2 = 1;
;    for (i = 0; i < 1024; i++)  {
;      arr1[i] = i2;
;      i2 = 13;
;    }
;  }
;  
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,print<hir>" -S < %s 2>&1 | FileCheck %s
; HIR Test.
; CHECK-NOT: @llvm.intel.directive(!1)
; ModuleID = 'j.c'
source_filename = "j.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @foo(ptr nocapture %arr1) #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %i2.06 = phi i32 [ 1, %entry ], [ 13, %for.body ]
  %arrayidx = getelementptr inbounds i32, ptr %arr1, i64 %indvars.iv
  store i32 %i2.06, ptr %arrayidx, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.9.0 (trunk 15639) (llvm/branches/loopopt 15736)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
