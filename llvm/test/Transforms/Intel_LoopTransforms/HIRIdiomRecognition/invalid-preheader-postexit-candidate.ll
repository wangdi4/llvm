; REQUIRES: asserts

; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-idiom" -print-before=hir-idiom -print-after=hir-idiom -debug-only=hir-idiom -disable-output 2>&1 < %s | FileCheck %s

; Verify that copy candidate (@S)[0][i1] = (@T)[0][i1] is considered illegal
; by checking both incoming and outgoing edge even though the edges are legal
; when considered separately.

; CHECK: Dump Before

; CHECK: + DO i1 = 0, 1023, 1   <DO_LOOP>
; CHECK: |   %add = %0  +  (@S)[0][-1 * i1 + 1023];
; CHECK: |   (@R)[0][i1] = %add;
; CHECK: |   (@S)[0][i1] = (@T)[0][i1];
; CHECK: + END LOOP

; CHECK: Analyzing candidate: 
; CHECK: (@S)[0][i1] = (@T)[0][i1];

; CHECK: Edge prevents sinking to postexit:
; CHECK:  (@S)[0][i1] --> (@S)[0][-1 * i1 + 1023] FLOW (<)


; CHECK: Edge prevents hoisting to preheader:
; CHECK: (@S)[0][-1 * i1 + 1023] --> (@S)[0][i1] ANTI (<)


; CHECK: Dump After

; CHECK-NOT: modified
; CHECK-NOT: memcpy


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@i = external dso_local local_unnamed_addr global i32, align 4
@S = external dso_local local_unnamed_addr global [1024 x float], align 16
@r = external dso_local local_unnamed_addr global float, align 4
@R = external dso_local local_unnamed_addr global [1024 x float], align 16
@T = external dso_local local_unnamed_addr global [1024 x float], align 16

define dso_local i32 @test() {
entry:
  %0 = load float, ptr @r, align 4
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %1 = sub nuw nsw i64 1023, %indvars.iv
  %arrayidx = getelementptr inbounds [1024 x float], ptr @S, i64 0, i64 %1
  %2 = load float, ptr %arrayidx, align 4
  %add = fadd fast float %0, %2
  %arrayidx2 = getelementptr inbounds [1024 x float], ptr @R, i64 0, i64 %indvars.iv
  store float %add, ptr %arrayidx2, align 4
  %arrayidx4 = getelementptr inbounds [1024 x float], ptr @T, i64 0, i64 %indvars.iv
  %3 = load float, ptr %arrayidx4, align 4
  %arrayidx6 = getelementptr inbounds [1024 x float], ptr @S, i64 0, i64 %indvars.iv
  store float %3, ptr %arrayidx6, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  store i32 1024, ptr @i, align 4
  ret i32 undef
}

