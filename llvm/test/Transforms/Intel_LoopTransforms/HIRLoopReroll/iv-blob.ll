; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-loop-reroll  -print-before=hir-loop-reroll -print-after=hir-loop-reroll  < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-reroll,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s

; Rerolls with IVs and Blobs in the right pattern

; CHECK: Function: foo

; CHECK:     BEGIN REGION { }
; CHECK:           + DO i1 = 0, (sext.i32.i64(%n) + -1)/u4, 1   <DO_LOOP>  <MAX_TC_EST = 2>
; CHECK:           |   (@B)[0][4 * i1] = 4 * i1 + ((1 + %n) * %n);
; CHECK:           |   (@B)[0][4 * i1 + 1] = 4 * i1 + ((2 + %n) * %n) + 1;
; CHECK:           |   (@B)[0][4 * i1 + 2] = 4 * i1 + ((1 + %n) * %n) + 2;
; CHECK:           |   (@B)[0][4 * i1 + 3] = 4 * i1 + ((2 + %n) * %n) + 3;
; CHECK:           + END LOOP
; CHECK:     END REGION

; CHECK: Function: foo

; CHECK:     BEGIN REGION { }
; CHECK:           + DO i1 = 0, 2 * ((3 + sext.i32.i64(%n)) /u 4) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4>
; CHECK:           |   (@B)[0][2 * i1] = 2 * i1 + ((1 + %n) * %n);
; CHECK:           |   (@B)[0][2 * i1 + 1] = 2 * i1 + ((2 + %n) * %n) + 1;
; CHECK:           + END LOOP
; CHECK:     END REGION

; Further check that reroll can be suppressed using a compiler flag

; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-loop-reroll  -print-after=hir-loop-reroll -hir-loop-reroll-size-threshold=3 < %s 2>&1 | FileCheck %s --check-prefix=NOREROLL
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-reroll,print<hir>" -hir-loop-reroll-size-threshold=3 -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s --check-prefix=NOREROLL

; NOREROLL: Function: foo

; NOREROLL:     BEGIN REGION { }
; NOREROLL:           + DO i1 = 0, (sext.i32.i64(%n) + -1)/u4, 1   <DO_LOOP>  <MAX_TC_EST = 2>
; NOREROLL:           |   (@B)[0][4 * i1] = 4 * i1 + ((1 + %n) * %n);
; NOREROLL:           |   (@B)[0][4 * i1 + 1] = 4 * i1 + ((2 + %n) * %n) + 1;
; NOREROLL:           |   (@B)[0][4 * i1 + 2] = 4 * i1 + ((1 + %n) * %n) + 2;
; NOREROLL:           |   (@B)[0][4 * i1 + 3] = 4 * i1 + ((2 + %n) * %n) + 3;
; NOREROLL:           + END LOOP
; NOREROLL:     END REGION

;Module Before HIR; ModuleID = 'blob-no-pattern.c'
source_filename = "blob-no-pattern.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common dso_local local_unnamed_addr global [10 x i32] zeroinitializer, align 16
@A = common dso_local local_unnamed_addr global [10 x i32] zeroinitializer, align 16
@C = common dso_local local_unnamed_addr global [10 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable writeonly
define dso_local void @foo(i32 %n) local_unnamed_addr #0 {
entry:
  %mul = mul nsw i32 %n, %n
  %add = add nsw i32 %n, 1
  %mul1 = mul nsw i32 %add, %n
  %cmp45 = icmp sgt i32 %n, 0
  br i1 %cmp45, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %0 = sext i32 %n to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %1 = trunc i64 %indvars.iv to i32
  %add2 = add nsw i32 %1, %n
  %add3 = add i32 %add2, %mul
  %arrayidx = getelementptr inbounds [10 x i32], [10 x i32]* @B, i64 0, i64 %indvars.iv
  store i32 %add3, i32* %arrayidx, align 16, !tbaa !2
  %add5 = add i32 %add2, %mul1
  %add6 = add i32 %add5, 1
  %2 = or i64 %indvars.iv, 1
  %arrayidx9 = getelementptr inbounds [10 x i32], [10 x i32]* @B, i64 0, i64 %2
  store i32 %add6, i32* %arrayidx9, align 4, !tbaa !2
  %add12 = add i32 %add3, 2
  %3 = or i64 %indvars.iv, 2
  %arrayidx15 = getelementptr inbounds [10 x i32], [10 x i32]* @B, i64 0, i64 %3
  store i32 %add12, i32* %arrayidx15, align 8, !tbaa !2
  %add18 = add i32 %add5, 3
  %4 = or i64 %indvars.iv, 3
  %arrayidx21 = getelementptr inbounds [10 x i32], [10 x i32]* @B, i64 0, i64 %4
  store i32 %add18, i32* %arrayidx21, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 4
  %cmp = icmp slt i64 %indvars.iv.next, %0
  br i1 %cmp, label %for.body, label %for.cond.cleanup.loopexit
}

attributes #0 = { norecurse nounwind uwtable writeonly "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang e6de10bf60ed5be7555542cd7b35318c8f7cb851) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 9418b1697133fe6fb0d391d3b1aea154274a2b79)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA10_i", !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
