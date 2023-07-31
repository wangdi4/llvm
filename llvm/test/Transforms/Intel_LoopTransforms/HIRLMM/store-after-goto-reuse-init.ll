; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-lmm,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s

; Verify that LMM reuses %liveout for initialization of %limm in preheader.
; By using this initialization, it is able to skip generation of condition
; (i2 != 0) for the early exit store within the loop. Refer to after-goto.ll
; for an example of the generated condition.

; Incoming HIR-
; + DO i1 = 0, 99, 1   <DO_LOOP>
; |   %liveout = (@C)[0][5];
; |
; |   + DO i2 = 0, 99, 1   <DO_MULTI_EXIT_LOOP>
; |   |   %t1 = (@A)[0][i2];
; |   |   if (%t1 > 0)
; |   |   {
; |   |      goto early.exit;
; |   |   }
; |   |   (@C)[0][5] = %t1 + 1;
; |   + END LOOP
; |
; |   early.exit:
; + END LOOP

; CHECK: BEGIN REGION { modified }
; CHECK: + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK: |   %liveout = (@C)[0][5];
; CHECK: |
; CHECK: |      %limm = %liveout;
; CHECK: |   + DO i2 = 0, 99, 1   <DO_MULTI_EXIT_LOOP>
; CHECK: |   |   %t1 = (@A)[0][i2];
; CHECK: |   |   if (%t1 > 0)
; CHECK: |   |   {
; CHECK: |   |      (@C)[0][5] = %limm;
; CHECK: |   |      goto early.exit;
; CHECK: |   |   }
; CHECK: |   |   %limm = %t1 + 1;
; CHECK: |   + END LOOP
; CHECK: |      (@C)[0][5] = %limm;
; CHECK: |
; CHECK: |   early.exit:
; CHECK: + END LOOP

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common dso_local local_unnamed_addr global [1000 x i32] zeroinitializer, align 16
@C = common dso_local local_unnamed_addr global [1000 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local i32 @foo() local_unnamed_addr #0 {
entry:
  br label %for.outer

for.outer:
  %iv.outer = phi i64 [ 0, %entry ], [ %iv.outer.next, %latch ]
  %liveout = load i32, ptr getelementptr inbounds ([1000 x i32], ptr @C, i64 0, i64 5), align 4, !tbaa !2 
  br label %for.body.preheader

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %if.end
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %if.end ]
  %arrayidx = getelementptr inbounds [1000 x i32], ptr @A, i64 0, i64 %indvars.iv
  %t1 = load i32, ptr %arrayidx, align 4, !tbaa !2
  %cmp1 = icmp sgt i32 %t1, 0
  br i1 %cmp1, label %early.exit, label %if.end

if.end:                                           ; preds = %for.body
  %add = add nsw i32 %t1, 1
  store i32 %add, ptr getelementptr inbounds ([1000 x i32], ptr @C, i64 0, i64 5), align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp = icmp slt i64 %indvars.iv.next, 100
  br i1 %cmp, label %for.body, label %for.end.loopexit

for.end.loopexit:                                 ; preds = %for.body, %if.end
  br label %latch

early.exit:
  br label %latch

latch:
  %iv.outer.next = add nuw nsw i64 %iv.outer, 1
  %cmp9 = icmp slt i64 %iv.outer.next, 100
  br i1 %cmp9, label %for.outer, label %for.end

for.end:                                          ; preds = %for.end.loopexit
  %lcssa = phi i32 [ %liveout, %latch ]
  ret i32 %lcssa
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang fc25755a7dd8cc64339b342d0cba7a81391fbd6e) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm d501475078e65b9fddb732ee8c0291f04fef5546)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA1000_i", !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
