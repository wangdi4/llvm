;RUN: opt -hir-ssa-deconstruction -S -hir-cg -force-hir-cg %s | FileCheck %s

; In subscript expr on <5> and <11>, we have a 64 bit iv in a 32 bit src 
;CE(but 64 bit dest type). Verify that the iv is generated as a truncated
;val and then operated on
;          BEGIN REGION { }
;<18>         + NumExits: 1
;<18>         + Ztt: No
;<18>         + DO i64 i1 = 0, zext.i32.i64((-1 + %N)), 1   <DO_LOOP>
;<18>         | <REG> LINEAR i64 zext.i32.i64((-1 + %N)) {sb:2}
;<18>         | <BLOB> LINEAR i32 %N {sb:4}
;<18>         | 
;<5>          |   (%A)[i1 + 16] = 77;
;<5>          |   <REG> (LINEAR i32* %A)[LINEAR zext.i32.i64(i1 + 16)] {sb:0}
;<5>          |   <BLOB> LINEAR i32* %A {sb:8}
;<5>          |   
;<11>         |   (%A)[3 * i1 + %N] = 22;
;<11>         |   <REG> (LINEAR i32* %A)[LINEAR zext.i32.i64(3 * i1 + %N)] {sb:0}
;<11>         |   <BLOB> LINEAR i32 %N {sb:4}
;<11>         |   <BLOB> LINEAR i32* %A {sb:8}
;<11>         |   
;<18>         + END LOOP

;CHECK: region.0:

; CHECK: {{loop.[0-9]+:}}

; CG for LINEAR zext.i32.i64(i1 + 16)]
; CHECK: [[IVLOAD1:%.*]] = load i64, i64* %i1.i64
; CHECK: [[TRUNC_IV1:%.*]] = trunc i64 [[IVLOAD1]] to i32
; CHECK: [[ADD_IV:%.*]] = add i32 [[TRUNC_IV1]], 16
; CHECK: zext i32 [[ADD_IV]] to i64

; CG for LINEAR zext.i32.i64(3 * i1 + %N)]
; CHECK: [[IVLOAD2:%.*]] = load i64, i64* %i1.i64
; CHECK: [[TRUNC_IV2:%.*]] = trunc i64 [[IVLOAD2]] to i32
; CHECK: [[MUL_IV:%.*]] = mul i32 3, [[TRUNC_IV2]]
; CHECK: [[ADD_IV2:%.*]] = add i32 %N, [[MUL_IV]]
; CHECK: zext i32 [[ADD_IV2]] to i64

; Check wrap flags on IV
; CHECK: [[IV_UPDATE:%.*]] = add nuw nsw i64 {{%.*}}, 1
; CHECK: icmp sle i64 [[IV_UPDATE]]

; ModuleID = 'test.ll'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define i32 @_Z3fooPii(i32* nocapture %A, i32 %N) #0 {
entry:
  %cmp.14 = icmp eq i32 %N, 0
  br i1 %cmp.14, label %for.cond.cleanup, label %for.body.preheader

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  %idxprom4 = sext i32 %N to i64
  %arrayidx5 = getelementptr inbounds i32, i32* %A, i64 %idxprom4
  %0 = load i32, i32* %arrayidx5, align 4, !tbaa !1
  ret i32 %0

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %for.body.preheader ]
  %add = add i64 %indvars.iv, 16
  %idxprom = and i64 %add, 4294967295
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %idxprom
  store i32 77, i32* %arrayidx, align 4, !tbaa !1
  %1 = trunc i64 %indvars.iv to i32
  %mul = mul i32 %1, 3
  %add1 = add i32 %mul, %N
  %idxprom2 = zext i32 %add1 to i64
  %arrayidx3 = getelementptr inbounds i32, i32* %A, i64 %idxprom2
  store i32 22, i32* %arrayidx3, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %N
  br i1 %exitcond, label %for.cond.cleanup.loopexit, label %for.body
}

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (trunk 1456) (llvm/branches/loopopt 1515)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
