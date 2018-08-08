; REQUIRES: asserts                                                                         
; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-loop-interchange -debug-only=hir-loop-interchange -hir-loop-interchange-near-perfect-profitability-tc-threshold=4 < %s 2>&1 | FileCheck %s
; RUN: opt -aa-pipeline="basic-aa" -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-interchange" -debug-only=hir-loop-interchange -hir-loop-interchange-near-perfect-profitability-tc-threshold=4 < %s 2>&1 | FileCheck %s

; CHECK: Loopnest Interchanged: ( 1 2 ) --> ( 2 1 )

; Source code
; #define N 16
; #define M 16
; int A[16];
; int B[16][16];
; 
; It is valid to sink load/store in i-loop to j-loop.
;
; int foo_2() {
;   int c = 0;
; 
;   for (int i = 0; i < N-1; i++) {
;     int k = A[i];
;     for (int j = 0; j < M; j++) {
;       c = c + k + B[j][i];
;     }
;     A[i+1] = k;
;   }
; 
;   return c;
; }

; *** IR Dump Before HIR Loop Interchange ***
; Function: _Z5foo_2v
; 
; <0>       BEGIN REGION { }
; <28>            + DO i1 = 0, 14, 1   <DO_LOOP>
; <3>             |   %0 = (@A)[0][i1];
; <29>            |   
; <29>            |   + DO i2 = 0, 15, 1   <DO_LOOP>
; <11>            |   |   %c.031 = %0 + %c.031  +  (@B)[0][i2][i1];
; <29>            |   + END LOOP
; <29>            |   
; <22>            |   (@A)[0][i1 + 1] = %0;
; <28>            + END LOOP
; <0>       END REGION
; 
; DDG's==
; 11:11 %c.031 --> %c.031 FLOW (<= *) (? ?)  
; 11:11 %c.031 --> %c.031 ANTI (= =) (0 0)  
; 3:11 %0 --> %0 FLOW (=) (0)  
; 3:22 %0 --> %0 FLOW (=) (0)  
; 22:3 (@A)[0][i1 + 1] --> (@A)[0][i1] FLOW (<) (1)  
; *** IR Dump After HIR Loop Interchange ***
; Function: _Z5foo_2v
; 
; <0>       BEGIN REGION { modified }
; <28>            + DO i1 = 0, 15, 1   <DO_LOOP>
; <29>            |   + DO i2 = 0, 14, 1   <DO_LOOP>
; <3>             |   |   %0 = (@A)[0][i2];
; <11>            |   |   %c.031 = %0 + %c.031  +  (@B)[0][i1][i2];
; <22>            |   |   (@A)[0][i2 + 1] = %0;
; <29>            |   + END LOOP
; <28>            + END LOOP
; <0>       END REGION

;Module Before HIR; ModuleID = 'not-equal-load-store-valid.cpp'
source_filename = "not-equal-load-store-valid.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = dso_local local_unnamed_addr global [16 x i32] zeroinitializer, align 16
@B = dso_local local_unnamed_addr global [16 x [16 x i32]] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local i32 @_Z5foo_2v() local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.cond.cleanup3
  %add9.lcssa.lcssa = phi i32 [ %add9.lcssa, %for.cond.cleanup3 ]
  ret i32 %add9.lcssa.lcssa

for.body:                                         ; preds = %for.cond.cleanup3, %entry
  %indvars.iv32 = phi i64 [ 0, %entry ], [ %indvars.iv.next33, %for.cond.cleanup3 ]
  %c.031 = phi i32 [ 0, %entry ], [ %add9.lcssa, %for.cond.cleanup3 ]
  %arrayidx = getelementptr inbounds [16 x i32], [16 x i32]* @A, i64 0, i64 %indvars.iv32
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !2
  br label %for.body4

for.cond.cleanup3:                                ; preds = %for.body4
  %add9.lcssa = phi i32 [ %add9, %for.body4 ]
  %indvars.iv.next33 = add nuw nsw i64 %indvars.iv32, 1
  %arrayidx12 = getelementptr inbounds [16 x i32], [16 x i32]* @A, i64 0, i64 %indvars.iv.next33
  store i32 %0, i32* %arrayidx12, align 4, !tbaa !2
  %exitcond34 = icmp eq i64 %indvars.iv.next33, 15
  br i1 %exitcond34, label %for.cond.cleanup, label %for.body

for.body4:                                        ; preds = %for.body4, %for.body
  %indvars.iv = phi i64 [ 0, %for.body ], [ %indvars.iv.next, %for.body4 ]
  %c.128 = phi i32 [ %c.031, %for.body ], [ %add9, %for.body4 ]
  %add = add nsw i32 %c.128, %0
  %arrayidx8 = getelementptr inbounds [16 x [16 x i32]], [16 x [16 x i32]]* @B, i64 0, i64 %indvars.iv, i64 %indvars.iv32
  %1 = load i32, i32* %arrayidx8, align 4, !tbaa !7
  %add9 = add nsw i32 %add, %1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 16
  br i1 %exitcond, label %for.cond.cleanup3, label %for.body4
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 7.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang ee02616b2c2a59ccaa0f22d40bfa39737ab8afa7) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm a9e599c763e0d8713d293019a2501a0be3175608)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA16_i", !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C++ TBAA"}
!7 = !{!8, !4, i64 0}
!8 = !{!"array@_ZTSA16_A16_i", !3, i64 0}
