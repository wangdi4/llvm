; Test for safe reduction chain - loop should be marked vectorizable
; LLVM IR generated from following testcase using clang -O1 -S -emit-llvm
;  int arr1[1024];
;  int arr2[1024];
;  
;  int foo(int t3, int t4)
;  {
;    int i;
;  
;    for (i = 0; i < 1024; i++)  {
;      t3 = t4 + arr1[i];
;      t4 = t3 + arr2[i];
;    }
;  
;    return t4;
;  }
;  
; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -print-after=hir-vec-dir-insert -S < %s 2>&1 | FileCheck %s
; RUN: opt --passes="hir-ssa-deconstruction,hir-vec-dir-insert,print<hir>" -S < %s 2>&1 | FileCheck %s
; HIR Test.
; CHECK: @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ]
; CHECK: DO i1 = 0, 1023, 1   <DO_LOOP>
; ModuleID = 'j4.c'
source_filename = "j4.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@arr1 = common global [1024 x i32] zeroinitializer, align 16
@arr2 = common global [1024 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind readonly uwtable
define i32 @foo(i32 %t3, i32 %t4) #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %t4.addr.08 = phi i32 [ %t4, %entry ], [ %add3, %for.body ]
  %arrayidx = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr1, i64 0, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %add = add nsw i32 %0, %t4.addr.08
  %arrayidx2 = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr2, i64 0, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx2, align 4, !tbaa !1
  %add3 = add nsw i32 %add, %1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret i32 %add3
}

attributes #0 = { norecurse nounwind readonly uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.9.0 (trunk 15639) (llvm/branches/loopopt 15736)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
