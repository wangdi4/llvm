; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Check parsing output for the loop verifying that non header phi base '%C.addr.0' is handled correctly.
; CHECK: DO i1 = 0, 9
; CHECK-NEXT: DO i2 = 0, 9
; CHECK-NEXT: {al:4}(%C.addr.0)[i1][i2] = i1 + i2
; CHECK-NEXT: END LOOP
; CHECK-NEXT: END LOOP


declare void @bar(i32)

; Function Attrs: nounwind uwtable
define void @foo([10 x i32]* nocapture %A, [10 x i32]* nocapture %B, [10 x i32]* nocapture readnone %C, i32 %k) #0 {
entry:
  %rem17 = and i32 %k, 1
  %tobool = icmp eq i32 %rem17, 0
  br i1 %tobool, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  tail call void @bar(i32 %k) #3
  br label %if.end

if.end:                                           ; preds = %entry, %if.then
  %C.addr.0 = phi [10 x i32]* [ %A, %if.then ], [ %B, %entry ]
  br label %for.cond.1.preheader

for.cond.1.preheader:                             ; preds = %for.inc.6, %if.end
  %indvars.iv21 = phi i64 [ 0, %if.end ], [ %indvars.iv.next22, %for.inc.6 ]
  br label %for.body.3

for.body.3:                                       ; preds = %for.body.3, %for.cond.1.preheader
  %indvars.iv = phi i64 [ 0, %for.cond.1.preheader ], [ %indvars.iv.next, %for.body.3 ]
  %0 = add nuw nsw i64 %indvars.iv, %indvars.iv21
  %arrayidx5 = getelementptr inbounds [10 x i32], [10 x i32]* %C.addr.0, i64 %indvars.iv21, i64 %indvars.iv
  %1 = trunc i64 %0 to i32
  store i32 %1, i32* %arrayidx5, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 10
  br i1 %exitcond, label %for.inc.6, label %for.body.3

for.inc.6:                                        ; preds = %for.body.3
  %indvars.iv.next22 = add nuw nsw i64 %indvars.iv21, 1
  %exitcond23 = icmp eq i64 %indvars.iv.next22, 10
  br i1 %exitcond23, label %for.end.8, label %for.cond.1.preheader

for.end.8:                                        ; preds = %for.inc.6
  ret void
}
