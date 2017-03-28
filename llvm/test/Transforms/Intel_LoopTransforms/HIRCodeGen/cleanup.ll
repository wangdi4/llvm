; This is more properly a full pipeline test. With full set of cleanup passes
; we should see IR close to incoming IR
; RUN: opt -hir-cg -force-hir-cg -simplifycfg -mem2reg -gvn -indvars -S < %s | FileCheck %s

; terminator of entry bblock should have changed
; CHECK-NOT: for.body
; CHECK: entry:
; CHECK-NOT: for.body

; CHECK-NOT: br i1 true, {{.*}}label %region
; CHECK: br label %loop

; 
; CHECK-NOT: region.0:
; CHECK: loop.[[NUM:.*]]:
; CHECK-NEXT: phi{{.*}}[ 0, %entry ] 

; CHECK: br{{.*}} label %loop.[[NUM]], label %afterloop.[[NUM1:.*]]

; CHECK: afterloop.[[NUM1]]:
; CHECK-NEXT: load{{.*}} @A
; CHECK-NEXT: ret

; ModuleID = 'test.cpp'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = global [10 x i32] zeroinitializer, align 16
@B = global [10 x i32] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define i32 @_Z3foov() #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %i.05 = phi i64 [ 1, %entry ], [ %inc, %for.body ]
  %arrayidx = getelementptr inbounds [10 x i32], [10 x i32]* @B, i64 0, i64 %i.05
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %arrayidx1 = getelementptr inbounds [10 x i32], [10 x i32]* @A, i64 0, i64 %i.05
  store i32 %0, i32* %arrayidx1, align 4, !tbaa !1
  %inc = add nuw nsw i64 %i.05, 1
  %exitcond = icmp eq i64 %inc, 5
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  %1 = load i32, i32* getelementptr inbounds ([10 x i32], [10 x i32]* @A, i64 0, i64 2), align 8, !tbaa !1
  ret i32 %1
}

attributes #0 = { nounwind uwtable "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.7.0 (trunk 637) (llvm/branches/loopopt 655)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
