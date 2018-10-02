; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-scalarrepl-array -print-before=hir-scalarrepl-array -print-after=hir-scalarrepl-array -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-scalarrepl-array,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; Verify that we do not handle memref groups whose preheader load cannot be deduced to be in the dimension range. Here, the preheader load of A[0][-1] which may be invalid memory.

; CHECK: + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>
; CHECK: |   %0 = (@A)[0][i1];
; CHECK: |   %t.1 = %0;
; CHECK: |   if (i1 != 0)
; CHECK: |   {
; CHECK: |      %2 = (@A)[0][i1 + -1];
; CHECK: |      %t.1 = %0 + %2;
; CHECK: |   }
; CHECK: + END LOOP

; CHECK-NOT: modified

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind readonly uwtable
define dso_local i32 @foo(i32 %n) local_unnamed_addr #0 {
entry:
  %cmp11 = icmp sgt i32 %n, 0
  br i1 %cmp11, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = sext i32 %n to i64
  br label %for.body

for.body:                                         ; preds = %for.inc, %for.body.preheader
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.inc ]
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* @A, i64 0, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !2
  %cmp1 = icmp eq i64 %indvars.iv, 0
  br i1 %cmp1, label %for.inc, label %if.then

if.then:                                          ; preds = %for.body
  %1 = add nsw i64 %indvars.iv, -1
  %arrayidx3 = getelementptr inbounds [100 x i32], [100 x i32]* @A, i64 0, i64 %1
  %2 = load i32, i32* %arrayidx3, align 4, !tbaa !2
  %add = add nsw i32 %2, %0
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %t.1 = phi i32 [ %add, %if.then ], [ %0, %for.body ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.inc
  %t.1.lcssa = phi i32 [ %t.1, %for.inc ]
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  %t.0.lcssa = phi i32 [ 0, %entry ], [ %t.1.lcssa, %for.end.loopexit ]
  ret i32 %t.0.lcssa
}

attributes #0 = { norecurse nounwind readonly uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang a66ec9ba1f7107c4d02c79535caf41af08db3679) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm e1446331525733fbafd5fad1067b510abbcb3d16)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA100_i", !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
