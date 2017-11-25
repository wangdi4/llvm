; RUN: opt < %s -analyze -hir-scc-formation | FileCheck %s

; RUN: opt < %s -analyze -scalar-evolution | FileCheck %s --check-prefix=SCEV

; Verify that %conv20.i has range info
; SCEV: -->  %conv20.i U: [0,256) 


; Verify that we do not form an SCC if the base phi has range info.
; CHECK-NOT: SCC1


@a = external local_unnamed_addr global [200000 x i32], align 4
@b = external local_unnamed_addr global [200000 x i32], align 4

define void @foo() {
entry:
  br label %for.body.i342

for.body.i342:                                    ; preds = %for.body.i342, %entry
  %conv20.i = phi i32 [ %conv.i340, %for.body.i342 ], [ 20, %entry ]
  %i.019.i = phi i32 [ %inc.i339, %for.body.i342 ], [ 0, %entry ]
  %mul.i = shl nuw nsw i32 %conv20.i, 1
  %add.i336 = or i32 %mul.i, 1
  %arrayidx.i337 = getelementptr inbounds [200000 x i32], [200000 x i32]* @a, i32 0, i32 %i.019.i
  store i32 %add.i336, i32* %arrayidx.i337, align 4
  %add2.i = add nuw nsw i32 %conv20.i, 3
  %conv4.i338 = shl nuw nsw i32 %add2.i, 1
  %mul5.i = and i32 %conv4.i338, 510
  %add6.i = or i32 %mul5.i, 1
  %arrayidx7.i = getelementptr inbounds [200000 x i32], [200000 x i32]* @b, i32 0, i32 %i.019.i
  store i32 %add6.i, i32* %arrayidx7.i, align 4
  %inc.i339 = add nuw nsw i32 %i.019.i, 1
  %conv.i340 = and i32 %add2.i, 255
  %exitcond.i341 = icmp eq i32 %inc.i339, 200000
  br i1 %exitcond.i341, label %for.end.i343, label %for.body.i342

for.end.i343:
  %conv.i340.lcssa = phi i32 [ %conv.i340, %for.body.i342 ]
  ret void
}
