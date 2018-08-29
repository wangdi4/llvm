; LLVM IR generated from following source using icx -O1 -S -emit-llvm
; int arr[1024];
; 
; int foo()
; {
;   int index, sum = 0;
; 
;   for (index = 0; index < 1024; index++)
;     sum += arr[index];
; 
;   return sum;
; }
; 
; ModuleID = 't1.c'
;RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -vplan-force-vf=8 -VPlanDriverHIR -hir-cg -mem2reg -print-after=VPlanDriverHIR -S %s 2>&1 | FileCheck %s
;
; CHECK:           BEGIN REGION { modified }
; CHECK:           %result.vector = insertelement zeroinitializer,  %sum.07,  0;

; CHECK:           + DO i1 = 0, 1023, 8   <DO_LOOP>
; CHECK:           |   %.vec = (<8 x i32>*)(@arr)[0][i1];
; CHECK:           |   %result.vector = %.vec  +  %result.vector;
; CHECK:           + END LOOP

; CHECK:           %rdx.shuf = shufflevector %result.vector,  %result.vector,  <i32 4, i32 5, i32 6, i32 7, i32 undef, i32 undef, i32 undef, i32 undef>;
; CHECK:           %bin.rdx = %result.vector  +  %rdx.shuf;
; CHECK:           %rdx.shuf1 = shufflevector %bin.rdx,  %bin.rdx,  <i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>;
; CHECK:           %bin.rdx2 = %bin.rdx  +  %rdx.shuf1;
; CHECK:           %rdx.shuf3 = shufflevector %bin.rdx2,  %bin.rdx2,  <i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>;
; CHECK:           %bin.rdx4 = %bin.rdx2  +  %rdx.shuf3;
; CHECK:           %sum.07 = extractelement %bin.rdx4,  0;
; CHECK:           END REGION

; CHECK: loop
; CHECK: add {{.*}} <8 x i32>
; CHECK: afterloop
; CHECK: shufflevector <8 x i32>
; CHECK: add <8 x i32>
; CHECK: shufflevector <8 x i32>
; CHECK: add <8 x i32>
; CHECK: shufflevector <8 x i32>
; CHECK: add <8 x i32>
; CHECK: extractelement <8 x i32>
; CHECK-NOT: add i32
source_filename = "t1.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@arr = common local_unnamed_addr global [1024 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind readonly uwtable
define i32 @foo() local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %sum.07 = phi i32 [ 0, %entry ], [ %add, %for.body ]
  %arrayidx = getelementptr inbounds [1024 x i32], [1024 x i32]* @arr, i64 0, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %add = add nsw i32 %0, %sum.07
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  %add.lcssa = phi i32 [ %add, %for.body ]
  ret i32 %add.lcssa
}

attributes #0 = { norecurse nounwind readonly uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (trunk 17978) (llvm/branches/loopopt 20351)"}
!1 = !{!2, !3, i64 0}
!2 = !{!"array@_ZTSA1024_i", !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
