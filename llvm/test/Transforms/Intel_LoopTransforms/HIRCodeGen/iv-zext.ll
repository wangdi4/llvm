
;RUN: opt -hir-ssa-deconstruction -S -hir-cg -force-hir-cg %s | FileCheck %s
; In rhs of <7> we have a 64 bit CE but a i32 iv. Verify we load i32 bit iv
; then zext to 64 before doing the addition

;          BEGIN REGION { }
;<18>         + NumExits: 1
;<18>         + Ztt: No
;<18>         + DO i32 i1 = 0, %N + -1, 1   <DO_LOOP>
;<18>         | <REG> LINEAR i32 %N + -1 {sb:2}
;<18>         | <BLOB> LINEAR i32 %N {sb:4}
;<18>         | 
;<7>          |   (%A)[i1 + 16] = i1 + 8589934591;
;<7>          |   <REG> (LINEAR i64* %A)[LINEAR zext.i32.i64(i1 + 16)] {sb:0}
;<7>          |   <BLOB> LINEAR i64* %A {sb:10}
;<7>          |   <REG> LINEAR i64 i1 + 8589934591 {sb:6}
;<7>          |   
;<12>         |   (%A)[3 * i1 + %N] = 22;
;<12>         |   <REG> (LINEAR i64* %A)[LINEAR zext.i32.i64(3 * i1 + %N)] {sb:0}
;<12>         |   <BLOB> LINEAR i64* %A {sb:10}
;<12>         |   <BLOB> LINEAR i32 %N {sb:4}
;<12>         |   
;<18>         + END LOOP
;          END REGION
;

; CHECK: region.0:

; CHECK: {{loop.[0-9]+:}}
; the interesting load of iv is on rhs, which is cg'd after address
; calculation for lhs
; CHECK: getelementptr

; CHECK: [[IVLOAD1:%.*]] = load i32, i32* %i1.i32
; CHECK: [[ZEXT_IV1:%.*]] = zext i32 [[IVLOAD1]] to i64
; CHECK: [[ADD_IV:%.*]] = add i64 [[ZEXT_IV1]], 8589934591
;

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
; Function Attrs: nounwind uwtable
define i32 @_Z3fooPli(i64* nocapture %A, i32 %N) #0 {
entry:
  %cmp.17 = icmp eq i32 %N, 0
  br i1 %cmp.17, label %for.cond.cleanup, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %entry
  br label %for.body

for.cond.for.cond.cleanup_crit_edge:              ; preds = %for.body
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %entry, %for.cond.for.cond.cleanup_crit_edge
  %idxprom5 = sext i32 %N to i64
  %arrayidx6 = getelementptr inbounds i64, i64* %A, i64 %idxprom5
  %0 = load i64, i64* %arrayidx6, align 8, !tbaa !1
  %conv7 = trunc i64 %0 to i32
  ret i32 %conv7

for.body:                                         ; preds = %for.body.lr.ph, %for.body
  %i.018 = phi i32 [ 0, %for.body.lr.ph ], [ %inc, %for.body ]
  %conv = zext i32 %i.018 to i64
  %add = add nuw nsw i64 %conv, 8589934591
  %add1 = add i32 %i.018, 16
  %idxprom = zext i32 %add1 to i64
  %arrayidx = getelementptr inbounds i64, i64* %A, i64 %idxprom
  store i64 %add, i64* %arrayidx, align 8, !tbaa !1
  %mul = mul i32 %i.018, 3
  %add2 = add i32 %mul, %N
  %idxprom3 = zext i32 %add2 to i64
  %arrayidx4 = getelementptr inbounds i64, i64* %A, i64 %idxprom3
  store i64 22, i64* %arrayidx4, align 8, !tbaa !1
  %inc = add i32 %i.018, 1
  %cmp = icmp ult i32 %inc, %N
  br i1 %cmp, label %for.body, label %for.cond.for.cond.cleanup_crit_edge
}

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (trunk 1456) (llvm/branches/loopopt 1515)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
