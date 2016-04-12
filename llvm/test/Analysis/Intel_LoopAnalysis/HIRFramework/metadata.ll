; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-framework -hir-details | FileCheck %s

; Verify that the load and store have metadata attached.
; CHECK: <RVAL-REG> {al:4}(LINEAR bitcast.float*.i32*(%B))[LINEAR i64 i1] !tbaa

; CHECK: <LVAL-REG> {al:4}(LINEAR bitcast.float*.i32*(%A))[LINEAR i64 i1] !tbaa


; ModuleID = 'metadata.ll'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @foo(float* nocapture %A, float* nocapture readonly %B, i32 %n) #0 {
entry:
  %cmp.7 = icmp sgt i32 %n, 0
  br i1 %cmp.7, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds float, float* %B, i64 %indvars.iv
  %0 = bitcast float* %arrayidx to i32*
  %1 = load i32, i32* %0, align 4, !tbaa !1
  %arrayidx2 = getelementptr inbounds float, float* %A, i64 %indvars.iv
  %2 = bitcast float* %arrayidx2 to i32*
  store i32 %1, i32* %2, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %n
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { nounwind uwtable "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.7.0 (trunk 814) (llvm/branches/loopopt 832)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"float", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
