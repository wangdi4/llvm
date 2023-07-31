; REQUIRES: asserts

; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-idiom" -print-before=hir-idiom -print-after=hir-idiom -debug-only=hir-idiom -disable-output 2>&1 < %s | FileCheck %s

; CHECK: Dump Before

; CHECK: + DO i1 = 0, 9, 1   <DO_LOOP>
; CHECK: |   + DO i2 = 0, 31, 1   <DO_LOOP>
; CHECK: |   |   (%A)[0][i2] = 0;
; CHECK: |   |   (%dst)[i1 + i2] = (%A)[0][i2];
; CHECK: |   + END LOOP
; CHECK: + END LOOP


; Verify that all backward edges were refined as independant.
; Output edge encountered twice, once for outgoing and one for incoming edge.

; CHECK:      Edge was refined as independant:
; CHECK-NEXT: (%A)[0][i2] --> (%A)[0][i2] OUTPUT (* =)

; CHECK:      Edge was refined as independant:
; CHECK-NEXT: (%A)[0][i2] --> (%A)[0][i2] OUTPUT (* =)

; CHECK:      Edge was refined as independant:
; CHECK-NEXT: (%A)[0][i2] --> (%A)[0][i2] ANTI (* =)


; CHECK: Dump After

; CHECK: + DO i1 = 0, 9, 1   <DO_LOOP>
; CHECK: |   @llvm.memset.p0.i64(&((i8*)(%A)[0][0]),  0,  128,  0);
; CHECK: |   @llvm.memcpy.p0.p0.i64(&((i8*)(%dst)[i1]),  &((i8*)(%A)[0][0]),  128,  0);
; CHECK: + END LOOP


target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(ptr nocapture noundef writeonly %dst) {
entry:
  %A = alloca [320 x i32], align 16
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.inc9
  %indvars.iv24 = phi i64 [ 0, %entry ], [ %indvars.iv.next25, %for.inc9 ]
  br label %for.body3

for.body3:                                        ; preds = %for.cond1.preheader, %for.body3
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body3 ]
  %arrayidx = getelementptr inbounds [320 x i32], ptr %A, i64 0, i64 %indvars.iv
  store i32 0, ptr %arrayidx, align 4
  %0 = add nuw nsw i64 %indvars.iv, %indvars.iv24
  %1 = load i32, ptr %arrayidx, align 4
  %arrayidx8 = getelementptr inbounds i32, ptr %dst, i64 %0
  store i32 %1, ptr %arrayidx8, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 32
  br i1 %exitcond.not, label %for.inc9, label %for.body3

for.inc9:                                         ; preds = %for.body3
  %indvars.iv.next25 = add nuw nsw i64 %indvars.iv24, 1
  %exitcond26.not = icmp eq i64 %indvars.iv.next25, 10
  br i1 %exitcond26.not, label %for.end11, label %for.cond1.preheader

for.end11:                                        ; preds = %for.inc9
  ret void
}
