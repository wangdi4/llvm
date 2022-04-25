; REQUIRES: asserts
; RUN: opt -analyze -enable-new-pm=0 -tbaa -hir-framework -hir-framework-debug=symbase-assignment -debug-only=hir-framework < %s 2>&1 | FileCheck %s

; Verify that the two struct references obj[0].0[i1] and obj[0].1[i1] are assigned different symbases.

; CHECK: (LINEAR %struct.S* @obj)[i64 0].0[LINEAR i64 i1] inbounds !tbaa !1 {sb:[[BASE1:[0-9]+]]}
; CHECK-NOT: (LINEAR %struct.S* @obj)[i64 0].1[LINEAR i64 i1] inbounds !tbaa !7 {sb:[[BASE1]]}
; CHECK: (LINEAR %struct.S* @obj)[i64 0].1[LINEAR i64 i1] inbounds !tbaa !7 {sb:[[BASE2:[0-9]+]]}

; ModuleID = 'struct.ll'
source_filename = "struct.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.S = type { [100 x i32], [100 x i32] }

@obj = common local_unnamed_addr global %struct.S zeroinitializer, align 4

; Function Attrs: norecurse nounwind uwtable
define void @foo(i32 %n) local_unnamed_addr #0 {
entry:
  %cmp9 = icmp sgt i32 %n, 0
  br i1 %cmp9, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = zext i32 %n to i64
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.preheader
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds %struct.S, %struct.S* @obj, i64 0, i32 0, i64 %indvars.iv
  %0 = trunc i64 %indvars.iv to i32
  store i32 %0, i32* %arrayidx, align 4, !tbaa !1
  %1 = trunc i64 %indvars.iv to i32
  %mul = shl nsw i32 %1, 1
  %add = or i32 %mul, 1
  %arrayidx2 = getelementptr inbounds %struct.S, %struct.S* @obj, i64 0, i32 1, i64 %indvars.iv
  store i32 %add, i32* %arrayidx2, align 4, !tbaa !7
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (trunk 20662) (llvm/branches/loopopt 20715)"}
!1 = !{!2, !4, i64 0}
!2 = !{!"struct@S", !3, i64 0, !3, i64 400}
!3 = !{!"array@_ZTSA100_i", !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = !{!2, !4, i64 400}
