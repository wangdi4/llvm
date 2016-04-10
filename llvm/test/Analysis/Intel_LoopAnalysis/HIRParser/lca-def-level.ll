; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser -hir-details | FileCheck %s

; Check parsing output for the loop verifying that t1.035 which is defined in first inner loop and used in second inner loop gets the defined at level of 1 in the second loop.

; CHECK: DO i32 i1 = 0, %n + -1

; First i2 loop
; CHECK: DO i64 i2 = 0, zext.i32.i64((-1 + %n))
; CHECK: %t1.035 = %0  +  %t1.035
; CHECK: END LOOP

; Second i2 loop
; CHECK: DO i64 i2 = 0, zext.i32.i64((-1 + %n))
; CHECK: (%B)[i2] = %1 + %t1.035

; Blob t1 should be linear, defined at level 1
; CHECK: <BLOB> LINEAR i32 %t1.035{def@1}
; CHECK: END LOOP
; CHECK: END LOOP


; ModuleID = 'lca.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @foo(i32* nocapture readonly %A, i32* nocapture %B, i32 %n) #0 {
entry:
  %cmp33 = icmp sgt i32 %n, 0
  br i1 %cmp33, label %for.body3.preheader.preheader, label %for.end15

for.body3.preheader.preheader:                    ; preds = %entry
  br label %for.body3.preheader

for.body3.preheader:                              ; preds = %for.body3.preheader.preheader, %for.inc13
  %t1.035 = phi i32 [ %add, %for.inc13 ], [ undef, %for.body3.preheader.preheader ]
  %i.034 = phi i32 [ %inc14, %for.inc13 ], [ 0, %for.body3.preheader.preheader ]
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.body3.preheader
  %indvars.iv = phi i64 [ 0, %for.body3.preheader ], [ %indvars.iv.next, %for.body3 ]
  %t1.130 = phi i32 [ %t1.035, %for.body3.preheader ], [ %add, %for.body3 ]
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %add = add nsw i32 %0, %t1.130
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %n
  br i1 %exitcond, label %for.body6.preheader, label %for.body3

for.body6.preheader:                              ; preds = %for.body3
  br label %for.body6

for.body6:                                        ; preds = %for.body6, %for.body6.preheader
  %indvars.iv36 = phi i64 [ %indvars.iv.next37, %for.body6 ], [ 0, %for.body6.preheader ]
  %arrayidx8 = getelementptr inbounds i32, i32* %B, i64 %indvars.iv36
  %1 = load i32, i32* %arrayidx8, align 4, !tbaa !1
  %add9 = add nsw i32 %1, %add
  store i32 %add9, i32* %arrayidx8, align 4, !tbaa !1
  %indvars.iv.next37 = add nuw nsw i64 %indvars.iv36, 1
  %lftr.wideiv38 = trunc i64 %indvars.iv.next37 to i32
  %exitcond39 = icmp eq i32 %lftr.wideiv38, %n
  br i1 %exitcond39, label %for.inc13, label %for.body6

for.inc13:                                        ; preds = %for.body6
  %inc14 = add nuw nsw i32 %i.034, 1
  %exitcond40 = icmp eq i32 %inc14, %n
  br i1 %exitcond40, label %for.end15.loopexit, label %for.body3.preheader

for.end15.loopexit:                               ; preds = %for.inc13
  br label %for.end15

for.end15:                                        ; preds = %for.end15.loopexit, %entry
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (trunk 2101) (llvm/branches/loopopt 2113)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
