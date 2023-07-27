; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-post-vec-complete-unroll,print<hir>" -disable-hir-create-fusion-regions 2>&1 < %s | FileCheck %s

; Verify that both loops are unrolled post vectorizer.
; The first loop is unrolled on the optimisitic assumption that the simplifiable alloca store (%B)[0][i1] can be eliminated after unrolling by forwarding the RHS to its uses.
; The second loop is unrolled because alloca load (%B)[0][i1] can be simplified because of the dominating simplifiable store in previous loop.

; CHECK: Function

; CHECK: BEGIN REGION { }
; CHECK: + DO i1 = 0, 9, 1   <DO_LOOP>
; CHECK: |   %0 = (@A)[0][i1];
; CHECK: |   (%B)[0][i1] = %0 + 1;
; CHECK: + END LOOP
; CHECK: END REGION

; CHECK: BEGIN REGION { }
; CHECK: + DO i1 = 0, 9, 1   <DO_LOOP>
; CHECK: |   %1 = (%B)[0][i1];
; CHECK: |   %2 = (@A)[0][i1];
; CHECK: |   (%C)[0][i1] = %1 + %2;
; CHECK: + END LOOP
; CHECK: END REGION

; CHECK: Function

; CHECK-NOT: DO i1


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common local_unnamed_addr global [100 x i32] zeroinitializer, align 16

; Function Attrs: nounwind readonly uwtable
define i32 @foo() local_unnamed_addr #0 {
entry:
  %B = alloca [100 x i32], align 16
  %C = alloca [100 x i32], align 16
  call void @llvm.lifetime.start.p0(i64 400, ptr nonnull %B) #2
  call void @llvm.lifetime.start.p0(i64 400, ptr nonnull %C) #2
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv30 = phi i64 [ 0, %entry ], [ %indvars.iv.next31, %for.body ]
  %arrayidx = getelementptr inbounds [100 x i32], ptr @A, i64 0, i64 %indvars.iv30
  %0 = load i32, ptr %arrayidx, align 4, !tbaa !2
  %add = add nsw i32 %0, 1
  %arrayidx2 = getelementptr inbounds [100 x i32], ptr %B, i64 0, i64 %indvars.iv30
  store i32 %add, ptr %arrayidx2, align 4, !tbaa !2
  %indvars.iv.next31 = add nuw nsw i64 %indvars.iv30, 1
  %exitcond32 = icmp eq i64 %indvars.iv.next31, 10
  br i1 %exitcond32, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  br label %for.body5

for.body5:                                        ; preds = %for.body5, %for.end
  %indvars.iv = phi i64 [ 0, %for.end ], [ %indvars.iv.next, %for.body5 ]
  %arrayidx7 = getelementptr inbounds [100 x i32], ptr %B, i64 0, i64 %indvars.iv
  %1 = load i32, ptr %arrayidx7, align 4, !tbaa !2
  %arrayidx9 = getelementptr inbounds [100 x i32], ptr @A, i64 0, i64 %indvars.iv
  %2 = load i32, ptr %arrayidx9, align 4, !tbaa !2
  %add10 = add nsw i32 %2, %1
  %arrayidx12 = getelementptr inbounds [100 x i32], ptr %C, i64 0, i64 %indvars.iv
  store i32 %add10, ptr %arrayidx12, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 10
  br i1 %exitcond, label %for.end15, label %for.body5

for.end15:                                        ; preds = %for.body5
  %arrayidx16 = getelementptr inbounds [100 x i32], ptr %C, i64 0, i64 2
  %3 = load i32, ptr %arrayidx16, align 8, !tbaa !2
  %arrayidx17 = getelementptr inbounds [100 x i32], ptr %C, i64 0, i64 3
  %4 = load i32, ptr %arrayidx17, align 4, !tbaa !2
  %add18 = add nsw i32 %4, %3
  call void @llvm.lifetime.end.p0(i64 400, ptr nonnull %C) #2
  call void @llvm.lifetime.end.p0(i64 400, ptr nonnull %B) #2
  ret i32 %add18
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0(i64, ptr nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0(i64, ptr nocapture) #1

attributes #0 = { nounwind readonly uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 6.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang e57b98c396b308fa49e45a267412d275bf3d90c4) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 68c5c3743c5d63ebc41f21ffbaf3a2d6ec87e213)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA100_i", !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
