; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-last-value-computation -print-after=hir-last-value-computation < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-last-value-computation,print<hir>" -aa-pipeline="basic-aa" 2>&1 < %s | FileCheck %s
;
; When the number of gotos is larger than 1, the pass does not trigger the code generation
;
;*** IR Dump Before HIR Last Value Computation ***
;Function: foo
;
;<0>       BEGIN REGION { }
;<23>            + DO i1 = 0, 39, 1   <DO_MULTI_EXIT_LOOP>
;<2>             |   %indvars.iv.out = i1;
;<6>             |   if ((@A)[0][i1] > 0)
;<6>             |   {
;<7>             |      goto for.end;
;<6>             |   }
;<6>             |   else
;<6>             |   {
;<13>            |      if ((@B)[0][i1] < 40)
;<13>            |      {
;<14>            |         goto for.end;
;<13>            |      }
;<6>             |   }
;<23>            + END LOOP
;<0>       END REGION
;
;*** IR Dump After HIR Last Value Computation ***
;Function: foo
;
; CHECK:     BEGIN REGION { }
; CHECK:        + DO i1 = 0, 39, 1   <DO_MULTI_EXIT_LOOP>
; CHECK:        |   if ((@A)[0][i1] > 0)
; CHECK:        |   {
; CHECK:        |      %indvars.iv.out = i1;
; CHECK:        |      goto for.end;
; CHECK:        |   }
; CHECK:        |   else
; CHECK:        |   {
; CHECK:        |      if ((@B)[0][i1] < 40)
; CHECK:        |      {
; CHECK:        |         %indvars.iv.out = i1;
; CHECK:        |         goto for.end;
; CHECK:        |      }
; CHECK:        |   }
; CHECK:        + END LOOP
; CHECK:           %indvars.iv.out = 39;
; CHECK:     END REGION
;
;Module Before HIR; ModuleID = 't.c'
source_filename = "t.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16
@B = common dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind readonly uwtable
define dso_local i32 @foo() local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc ]
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* @A, i64 0, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !2
  %cmp1 = icmp sgt i32 %0, 0
  br i1 %cmp1, label %for.end, label %if.end

if.end:                                           ; preds = %for.body
  %arrayidx3 = getelementptr inbounds [100 x i32], [100 x i32]* @B, i64 0, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx3, align 4, !tbaa !2
  %cmp4 = icmp slt i32 %1, 40
  br i1 %cmp4, label %for.end, label %for.inc

for.inc:                                          ; preds = %if.end
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp = icmp ult i64 %indvars.iv, 39
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %if.end, %for.body, %for.inc
  %indvars.iv.lcssa = phi i64 [ %indvars.iv, %if.end ], [ %indvars.iv, %for.body ], [ %indvars.iv, %for.inc ]
  %2 = trunc i64 %indvars.iv.lcssa to i32
  ret i32 %2
}

attributes #0 = { norecurse nounwind readonly uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang a53756907774b7d85a523756d285be3e3ac08d1c) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 4ba20539fd9a4fe0ce3c26f371c233abdd35e0f4)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA100_i", !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
