; RUN: opt -enable-intel-advanced-opts -tbaa -mattr=+avx2 -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -debug-only=vplan-vls-analysis -vplan-force-vf=4 -disable-output < %s 2>&1 | FileCheck %s
target datalayout = "e-m:e-p:32:32-p270:32:32-p271:32:32-p272:64:64-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

%struct.S1 = type { i64, i8* }

; Function Attrs: argmemonly nofree norecurse nosync nounwind writeonly uwtable
define dso_local i32 @foo(%struct.S1* nocapture noundef writeonly %sp, i32 noundef %n1) local_unnamed_addr #0 {
; CHECK-COUNT-3: Fixed all OVLSTypes for previously collected memrefs.
; CHECK-NOT: Assertion `InterleaveFactor * (int)ElementSizeInBits == 8 * (*Stride) && "Stride is not a multiple of element size"' failed.
entry:
  %cmp8 = icmp sgt i32 %n1, 0
  br i1 %cmp8, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %l1.09 = phi i32 [ %inc, %for.body ], [ 0, %for.body.preheader ]
  %conv = zext i32 %l1.09 to i64
  %a = getelementptr inbounds %struct.S1, %struct.S1* %sp, i32 %l1.09, i32 0
  store i64 %conv, i64* %a, align 8, !tbaa !1
  %b = getelementptr inbounds %struct.S1, %struct.S1* %sp, i32 %l1.09, i32 1
  %b.bc = bitcast i8** %b to i64*
  store i64 %conv, i64* %b.bc, align 8, !tbaa !7
  %inc = add nuw nsw i32 %l1.09, 1
  %exitcond.not = icmp eq i32 %inc, 1024
  br i1 %exitcond.not, label %for.end.loopexit, label %for.body, !llvm.loop !8

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret i32 12
}

!1 = !{!2, !3, i64 0}
!2 = !{!"struct@S1", !3, i64 0, !6, i64 8}
!3 = !{!"long long", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!"pointer@_ZTSPl", !4, i64 0}
!7 = !{!2, !6, i64 8}
!8 = distinct !{!8, !9}
!9 = !{!"llvm.loop.mustprogress"}
