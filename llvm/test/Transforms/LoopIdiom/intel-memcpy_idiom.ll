; RUN: opt -tbaa -loop-idiom < %s -S | FileCheck %s
;
; The loop in the following is expected to be transformed
; into the intrinsic llvm.memcpy
;
%struct.S = type { [1048576 x i32], [1048576 x i32] }

; Function Attrs: norecurse nounwind uwtable
; CHECK: llvm.memcpy
define void @loop(%struct.S* nocapture %p, i32 %N) #0 {
entry:
  %cmp1 = icmp eq i32 %N, 0
  br i1 %cmp1, label %for.end, label %for.body

for.body:                                         ; preds = %entry, %for.body
  %i.02 = phi i32 [ %inc, %for.body ], [ 0, %entry ]
  %idxprom = zext i32 %i.02 to i64
  %arrayidx = getelementptr inbounds %struct.S, %struct.S* %p, i64 0, i32 1, i64 %idxprom
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %arrayidx2 = getelementptr inbounds %struct.S, %struct.S* %p, i64 0, i32 0, i64 %idxprom
  store i32 %0, i32* %arrayidx2, align 4, !tbaa !7
  %inc = add i32 %i.02, 1
  %cmp = icmp ult i32 %inc, %N
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %entry, %for.body
  ret void

; uselistorder directives
  uselistorder i32 %inc, { 1, 0 }
  uselistorder i32 %i.02, { 1, 0 }
  uselistorder i32 1, { 1, 0 }
  uselistorder i32 0, { 2, 1, 0 }
  uselistorder i32 %N, { 1, 0 }
  uselistorder label %for.end, { 1, 0 }
  uselistorder label %for.body, { 1, 0 }
}



attributes #1 = { argmemonly nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (trunk 2052) (llvm/trunk 2047)"}
!1 = !{!2, !4, i64 4194304}
!2 = !{!"struct@", !3, i64 0, !3, i64 4194304}
!3 = !{!"array@_ZTSA1048576_i", !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = !{!2, !4, i64 0}
