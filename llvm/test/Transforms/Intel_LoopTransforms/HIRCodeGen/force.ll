; With forced hir-cg, we should see HIR->LLVM IR occur
; RUN: opt -hir-ssa-deconstruction -hir-cg -force-hir-cg -S < %s | FileCheck %s

; terminator of entry bblock should have changed
; CHECK: for.body:
; CHECK: br i1 true, {{.*}}label %region

; 
; CHECK: region.0:
; CHECK: store i64 0, i64* %i1
; CHECK-NEXT: br label %[[L1Label:loop.[0-9]+]]

; CHECK: [[L1Label]]:
; check iv load is loaded
; CHECK-NEXT: load{{.*}} %i1

; load B[] and store it into a memslot for symbase of lval temp
; CHECK: [[GEP:%.*]] = getelementptr {{.*}} @B
; CHECK-NEXT: [[GEPLOAD:%.*]] = load{{.*}} [[GEP]]

; Check load attributes and metadata
; CHECK-SAME: align 4
; CHECK-SAME: tbaa
; CHECK-NEXT: store i32 [[GEPLOAD]], i32* [[TEMPSLOT:.*]]

; get addr of A[], load memslot from earlier and stored loaded 
; value at that addr
; CHECK-DAG: [[GEP:%.*]] = getelementptr {{.*}} @A
; CHECK-DAG: [[TEMPLOAD:%t.*]] = load i32, i32* [[TEMPSLOT]]
; CHECK-NEXT: store i32 [[TEMPLOAD]], i32* [[GEP]]

; Check store attributes and metadata
; CHECK-SAME: align 4
; CHECK-SAME: tbaa

; a value for iv is stored
; CHECK: store{{.*}} %i1
; CHECK: br{{.*}} label %[[L1Label]], label %after[[L1Label]]

; CHECK: after[[L1Label]]:
; CHECK-NEXT: br label %for.end

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
