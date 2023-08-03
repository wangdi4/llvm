; RUN: opt -hir-cost-model-throttling=0 -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-scalarrepl-array,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; Verify that scalar replacement triggers in the presence of calls with inaccessiblememonly or inaccessiblemem_or_argmemonly attribute.

; HIR-
; + DO i1 = 0, 98, 1   <DO_LOOP>
; |   %add3 = (@A)[0][i1]  +  (@A)[0][i1 + 1];
; |   (@B)[0][i1] = %add3;
; |   @bar1();
; |   @bar2();
; + END LOOP

; CHECK: %scalarepl = (@A)[0][0];
; CHECK: + DO i1 = 0, 98, 1   <DO_LOOP>
; CHECK: |   %scalarepl1 = (@A)[0][i1 + 1];
; CHECK: |   %add3 = %scalarepl  +  %scalarepl1;
; CHECK: |   (@B)[0][i1] = %add3;
; CHECK: |   @bar1();
; CHECK: |   @bar2();
; CHECK: |   %scalarepl = %scalarepl1;
; CHECK: + END LOOP

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16
@B = common dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16

define dso_local void @foo() local_unnamed_addr {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [100 x float], ptr @A, i64 0, i64 %indvars.iv
  %0 = load float, ptr %arrayidx, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %arrayidx2 = getelementptr inbounds [100 x float], ptr @A, i64 0, i64 %indvars.iv.next
  %1 = load float, ptr %arrayidx2, align 4, !tbaa !2
  %add3 = fadd float %0, %1
  %arrayidx5 = getelementptr inbounds [100 x float], ptr @B, i64 0, i64 %indvars.iv
  store float %add3, ptr %arrayidx5, align 4, !tbaa !2
  tail call void (...) @bar1() #0
  tail call void (...) @bar2() #1
  %exitcond = icmp eq i64 %indvars.iv.next, 99
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

declare dso_local void @bar1(...) local_unnamed_addr #0
declare dso_local void @bar2(...) local_unnamed_addr #1

attributes #0 = { inaccessiblemem_or_argmemonly }
attributes #1 = { inaccessiblememonly }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 7.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 75251c447951a5a8c1526f5e9b69dfb5d68bce8e) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 805a7d6acb9458e85cc94f93823971a3d5f0e5fb)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA100_f", !4, i64 0}
!4 = !{!"float", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
