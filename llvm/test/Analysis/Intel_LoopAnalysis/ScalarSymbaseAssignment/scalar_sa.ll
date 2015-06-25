; RUN: opt < %s -loop-simplify | opt -analyze -hir-scalar-sa | FileCheck %s

; Check region liveins/liveouts
; CHECK: Region 1
; CHECK-NEXT: LiveIns:
; CHECK-SAME: a.addr.08(%a)
; CHECK-NEXT: LiveOuts:
; CHECK-SAME: output.1
; CHECK-NOT: Region 

; ModuleID = 'de_ssa.ll'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define i32 @foo(i32* nocapture %A, i32 %a, i32 %b, i32 %n) #0 {
entry:
  %cmp7 = icmp sgt i32 %n, 0
  br i1 %cmp7, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.inc
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.inc ], [ 0, %for.body.preheader ]
  %a.addr.08 = phi i32 [ %a.addr.1, %for.inc ], [ %a, %for.body.preheader ], !hir.de.ssa !1
  %cmp1 = icmp sgt i64 %indvars.iv, 77
  %output.1.de.ssa1 = bitcast i32 %b to i32, !hir.de.ssa !2
  br i1 %cmp1, label %if.then, label %for.inc

if.then:                                          ; preds = %for.body
  %inc = add nsw i32 %a.addr.08, 1
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %indvars.iv
  store i32 %inc, i32* %arrayidx, align 4, !tbaa !3
  %output.1.de.ssa = bitcast i32 %a.addr.08 to i32, !hir.de.ssa !2
  br label %for.inc

for.inc:                                          ; preds = %if.then, %for.body
  %a.addr.1 = phi i32 [ %inc, %if.then ], [ %a.addr.08, %for.body ]
  %output.1 = phi i32 [ %a.addr.08, %if.then ], [ %b, %for.body ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %n
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.inc
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  %output.0.lcssa = phi i32 [ -1, %entry ], [ %output.1, %for.end.loopexit ]
  ret i32 %output.0.lcssa
}

attributes #0 = { nounwind uwtable "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.7.0 (trunk 637) (llvm/branches/loopopt 662)"}
!1 = !{!"a.addr.08.de.ssa"}
!2 = !{!"output.1.de.ssa"}
!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}

