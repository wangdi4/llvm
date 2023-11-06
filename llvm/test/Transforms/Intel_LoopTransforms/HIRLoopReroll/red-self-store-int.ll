; XFAIL: *
; RUN: opt -disable-output -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-reroll,print<hir>" -aa-pipeline="basic-aa" -hir-verify-cf-def-level < %s 2>&1 | FileCheck %s

; The code has both StoreInst type seeds and Self Safe Reduction type seeds. Reroll currently doesn't handle multi-type seeds.

; CHECK:Function: foo

; CHECK:        BEGIN REGION { }
; CHECK:              + DO i1 = 0, 49, 1   <DO_LOOP>
; CHECK:              |   %0 = (@C)[0][2 * i1];
; CHECK:              |   %1 = (@B)[0][2 * i1];
; CHECK:              |   (@A)[0][2 * i1] = %0 + %1;
; CHECK:              |   %3 = (@C)[0][2 * i1 + 1];
; CHECK:              |   %4 = (@B)[0][2 * i1 + 1];
; CHECK:              |   (@A)[0][2 * i1 + 1] = %3 + %4;
; CHECK:              |   %S.036 = %0 + %1 + %S.036  +  %3 + %4;
; CHECK:              + END LOOP
; CHECK:        END REGION

; CHECK:Function: foo

; CHECK:        BEGIN REGION { }
; CHECK:              + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:              |   %0 = (@C)[0][i1];
; CHECK:              |   %1 = (@B)[0][i1];
; CHECK:              |   (@A)[0][i1] = %0 + %1;
; CHECK:              |   %S.036 = %0 + %1 + %S.036;
; CHECK:              + END LOOP
; CHECK:        END REGION

;Module Before HIR
; ModuleID = 'store-red-2.c'
source_filename = "store-red-2.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@C = common dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16
@B = common dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16
@A = common dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local i32 @foo() local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  %add21.lcssa = phi i32 [ %add21, %for.body ]
  ret i32 %add21.lcssa

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %S.036 = phi i32 [ 0, %entry ], [ %add21, %for.body ]
  %arrayidx = getelementptr inbounds [100 x i32], ptr @C, i64 0, i64 %indvars.iv, !intel-tbaa !2
  %0 = load i32, ptr %arrayidx, align 8, !tbaa !2
  %arrayidx2 = getelementptr inbounds [100 x i32], ptr @B, i64 0, i64 %indvars.iv, !intel-tbaa !2
  %1 = load i32, ptr %arrayidx2, align 8, !tbaa !2
  %add = add nsw i32 %1, %0
  %arrayidx4 = getelementptr inbounds [100 x i32], ptr @A, i64 0, i64 %indvars.iv, !intel-tbaa !2
  store i32 %add, ptr %arrayidx4, align 8, !tbaa !2
  %add7 = add nsw i32 %add, %S.036
  %2 = or i64 %indvars.iv, 1
  %arrayidx10 = getelementptr inbounds [100 x i32], ptr @C, i64 0, i64 %2, !intel-tbaa !2
  %3 = load i32, ptr %arrayidx10, align 4, !tbaa !2
  %arrayidx13 = getelementptr inbounds [100 x i32], ptr @B, i64 0, i64 %2, !intel-tbaa !2
  %4 = load i32, ptr %arrayidx13, align 4, !tbaa !2
  %add14 = add nsw i32 %4, %3
  %arrayidx17 = getelementptr inbounds [100 x i32], ptr @A, i64 0, i64 %2, !intel-tbaa !2
  store i32 %add14, ptr %arrayidx17, align 4, !tbaa !2
  %add21 = add nsw i32 %add7, %add14
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 2
  %cmp = icmp ult i64 %indvars.iv.next, 100
  br i1 %cmp, label %for.body, label %for.cond.cleanup
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 4d9491ed638c64d173c02b77ca3a29cba2d9d1ef) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 84d2d36f7cbbf8594469caa5e415e2f0c5141e2e)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA100_i", !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
