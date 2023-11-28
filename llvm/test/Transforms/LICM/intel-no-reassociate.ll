; RUN: opt -passes='licm' -S < %s | FileCheck %s

; When loopopt is enabled, we want to avoid reassociation in LICM, even if
; it can improve hoisting. Below, LICM would like to reassociate the fmuls
; to (%d1 * %delta) * %cell.1 and hoist %d1 * %delta.
; For well-structured incoming code (such as hand-tuned matrix multiply),
; the reassociation may cause pattern misses in loop opt and SLP.
; It also seems to be causing a precision issue in OMP2021/370.

; CHECK-NOT: fmul{{.*}}%d1{{.*}}%delta
; CHECK-NOT: fmul{{.*}}%delta{{.*}}%d1

define void @foo(i32 %i, double %d1, double %delta, ptr %cells) #0 {
entry:
  br label %for.cond

for.cond:
  %j = phi i32 [ 0, %entry ], [ %add.j.1, %for.body ]
  %cmp.not = icmp sgt i32 %j, %i
  br i1 %cmp.not, label %for.end, label %for.body

for.body:
  %add.j.1 = add nuw nsw i32 %j, 1
  %idxprom.j.1 = zext i32 %add.j.1 to i64
  %arrayidx.j.1 = getelementptr inbounds double, ptr %cells, i64 %idxprom.j.1
  %cell.1 = load double, ptr %arrayidx.j.1, align 8
  %fmul.1 = fmul fast double %d1, %cell.1
  %fmul.2 = fmul fast double %fmul.1, %delta
  %idxprom.j = zext i32 %j to i64
  %arrayidx.j = getelementptr inbounds double, ptr %cells, i64 %idxprom.j
  store double %fmul.2, ptr %arrayidx.j, align 8
  br label %for.cond

for.end:
  ret void
}

attributes #0 = { "loopopt-pipeline"="full" }
