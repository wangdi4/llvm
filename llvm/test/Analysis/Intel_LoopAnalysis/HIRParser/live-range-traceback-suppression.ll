; RUN: opt < %s -hir-ssa-deconstruction -hir-complete-unroll -print-before=hir-complete-unroll 2>&1 | FileCheck %s

; Check parsing output for the loop verifying that traceback for the instruction %add which is marked as live range indicator by ssa deconstruction is suppressed when parsing %sub59.
; CHECK: DO i1 = 0, 15
; CHECK-NEXT: %index.4184.out = %index.4184
; CHECK-NEXT: %0 = {al:1}(%p)[i1 + 1];
; CHECK-NEXT: %.sink = -1;
; CHECK-NEXT: if (%0 == 0)
; CHECK-NEXT: {
; CHECK-NEXT: }
; CHECK-NEXT: else
; CHECK-NEXT: {
; CHECK-NEXT: %1 = {al:4}(%huffcode)[0][%index.4184.out];
; CHECK-NEXT: (%q)[i1 + 1] = -1 * %1 + %index.4184.out;
; CHECK-NEXT: %2 = {al:1}(%p)[i1 + 1];
; CHECK-NEXT: %index.4184 = %2  +  %index.4184;
; CHECK-NEXT: %3 = {al:4}(%huffcode)[0][%index.4184 + -1];
; CHECK-NEXT: %.sink = %3;
; CHECK-NEXT: }
; CHECK-NEXT: (%r)[i1 + 1] = %.sink;
; CHECK-NEXT: END LOOP


define void @DeriveHuffmanTable(i8* %p, i32* %q, i32* %r) {
entry:
  %huffcode = alloca [257 x i32], align 4
  br label %for.body.52

for.body.52:                                      ; preds = %entry, %for.inc.64
  %length_index.1185 = phi i32 [ %inc65, %for.inc.64 ], [ 1, %entry ]
  %index.4184 = phi i32 [ %index.5, %for.inc.64 ], [ 0, %entry ]
  %arrayidx53 = getelementptr inbounds i8, i8* %p, i32 %length_index.1185
  %0 = load i8, i8* %arrayidx53, align 1
  %tobool54 = icmp eq i8 %0, 0
  br i1 %tobool54, label %for.inc.64, label %if.then.55

if.then.55:                                       ; preds = %for.body.52
  %arrayidx56 = getelementptr inbounds [257 x i32], [257 x i32]* %huffcode, i32 0, i32 %index.4184
  %1 = load i32, i32* %arrayidx56, align 4
  %sub = sub nsw i32 %index.4184, %1
  %arrayidx57 = getelementptr inbounds i32, i32* %q, i32 %length_index.1185
  store i32 %sub, i32* %arrayidx57, align 4
  %2 = load i8, i8* %arrayidx53, align 1
  %conv = zext i8 %2 to i32
  %add = add nsw i32 %conv, %index.4184
  %sub59 = add nsw i32 %add, -1
  %arrayidx60 = getelementptr inbounds [257 x i32], [257 x i32]* %huffcode, i32 0, i32 %sub59
  %3 = load i32, i32* %arrayidx60, align 4
  br label %for.inc.64

for.inc.64:                                       ; preds = %for.body.52, %if.then.55
  %.sink = phi i32 [ %3, %if.then.55 ], [ -1, %for.body.52 ]
  %index.5 = phi i32 [ %add, %if.then.55 ], [ %index.4184, %for.body.52 ]
  %4 = getelementptr inbounds i32, i32* %r, i32 %length_index.1185
  store i32 %.sink, i32* %4, align 4
  %inc65 = add nuw nsw i32 %length_index.1185, 1
  %exitcond204 = icmp eq i32 %inc65, 17
  br i1 %exitcond204, label %for.end.66, label %for.body.52

for.end.66:
  ret void
}

