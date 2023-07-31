; REQUIRES: asserts
; RUN: opt -aa-pipeline="basic-aa" -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-interchange" -debug-only=hir-loop-interchange < %s 2>&1 | FileCheck %s

; CHECK-NOT: Interchanged
;
; #define N 16
; #define M 16
; int A[16];
; int B[16][16];
;
; Will lead to a wrong result if load/store in the i loop are sinked into j loop.
;
; int foo() {
;   int c = 0;
;   int k = 10;
;   for (int i = 0; i < N-1; i++) {
;     c = A[i];
;     for (int j = 0; j < M; j++) {
;       c = c + k + B[j][i];
;     }
;     A[i+1] = c;
;   }
;
;   return c;
; }

; *** IR Dump Before HIR Loop Interchange ***
; Function: _Z3foov
;
; <0>       BEGIN REGION { }
; <28>            + DO i1 = 0, 14, 1   <DO_LOOP>
; <3>             |   %c.128 = (@A)[0][i1];
; <29>            |
; <29>            |   + DO i2 = 0, 15, 1   <DO_LOOP>
; <12>            |   |   %c.128 = %c.128 + 10  +  (@B)[0][i2][i1];
; <29>            |   + END LOOP
; <29>            |
; <22>            |   (@A)[0][i1 + 1] = %c.128;
; <28>            + END LOOP
; <0>       END REGION
;
; DDG's==
; 3:12 %c.128 --> %c.128 OUTPUT (*) (?)
; 3:12 %c.128 --> %c.128 FLOW (=) (0)
; 3:22 %c.128 --> %c.128 FLOW (=) (0)
; 12:12 %c.128 --> %c.128 FLOW (<= *) (? ?)
; 12:22 %c.128 --> %c.128 FLOW (=) (0)
; 12:12 %c.128 --> %c.128 ANTI (= =) (0 0)
; 22:3 (@A)[0][i1 + 1] --> (@A)[0][i1] FLOW (<) (1)
;
; *** IR Dump After HIR Loop Interchange ***
; Function: _Z3foov
;
; <0>       BEGIN REGION { }
; <28>            + DO i1 = 0, 14, 1   <DO_LOOP>
; <3>             |   %c.128 = (@A)[0][i1];
; <29>            |
; <29>            |   + DO i2 = 0, 15, 1   <DO_LOOP>
; <12>            |   |   %c.128 = %c.128 + 10  +  (@B)[0][i2][i1];
; <29>            |   + END LOOP
; <29>            |
; <22>            |   (@A)[0][i1 + 1] = %c.128;
; <28>            + END LOOP
; <0>       END REGION
;
;Module Before HIR; ModuleID = 'not-equal-load-store-invalid.cpp'
source_filename = "not-equal-load-store-invalid.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = dso_local local_unnamed_addr global [16 x i32] zeroinitializer, align 16
@B = dso_local local_unnamed_addr global [16 x [16 x i32]] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local i32 @_Z3foov() local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.cond.cleanup3
  %add9.lcssa.lcssa = phi i32 [ %add9.lcssa, %for.cond.cleanup3 ]
  ret i32 %add9.lcssa.lcssa

for.body:                                         ; preds = %for.cond.cleanup3, %entry
  %indvars.iv31 = phi i64 [ 0, %entry ], [ %indvars.iv.next32, %for.cond.cleanup3 ]
  %arrayidx = getelementptr inbounds [16 x i32], ptr @A, i64 0, i64 %indvars.iv31
  %0 = load i32, ptr %arrayidx, align 4, !tbaa !2
  br label %for.body4

for.cond.cleanup3:                                ; preds = %for.body4
  %add9.lcssa = phi i32 [ %add9, %for.body4 ]
  %indvars.iv.next32 = add nuw nsw i64 %indvars.iv31, 1
  %arrayidx12 = getelementptr inbounds [16 x i32], ptr @A, i64 0, i64 %indvars.iv.next32
  store i32 %add9.lcssa, ptr %arrayidx12, align 4, !tbaa !2
  %exitcond33 = icmp eq i64 %indvars.iv.next32, 15
  br i1 %exitcond33, label %for.cond.cleanup, label %for.body

for.body4:                                        ; preds = %for.body4, %for.body
  %indvars.iv = phi i64 [ 0, %for.body ], [ %indvars.iv.next, %for.body4 ]
  %c.128 = phi i32 [ %0, %for.body ], [ %add9, %for.body4 ]
  %add = add nsw i32 %c.128, 10
  %arrayidx8 = getelementptr inbounds [16 x [16 x i32]], ptr @B, i64 0, i64 %indvars.iv, i64 %indvars.iv31
  %1 = load i32, ptr %arrayidx8, align 4, !tbaa !7
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
