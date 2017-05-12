; RUN: opt < %s -gvn -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Verify that gvn doesn't perform scalar replacement on @foo marked with "pre_loopopt" attribute but it does on identical @foo1 without the attribute.

; Src code-

; int A[100];
; int B[100];
; void foo () {
;   int i;

;   for(i=0; i< 100; i++) {
;     A[i] = B[i] + B[i+1];
;   }
; }

; CHECK: function 'foo'

; CHECK: + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK: |   %0 = (@B)[0][i1];
; CHECK: |   %1 = (@B)[0][i1 + 1];
; CHECK: |   (@A)[0][i1] = %0 + %1;
; CHECK: + END LOOP

; CHECK: function 'foo1'

; CHECK: + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK: |   %1 = (@B)[0][i1 + 1];
; CHECK: |   (@A)[0][i1] = %1 + %0;
; CHECK: |   %0 = %1;
; CHECK: + END LOOP


@B = common local_unnamed_addr global [100 x i32] zeroinitializer, align 16
@A = common local_unnamed_addr global [100 x i32] zeroinitializer, align 16

define void @foo() #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* @B, i64 0, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %arrayidx2 = getelementptr inbounds [100 x i32], [100 x i32]* @B, i64 0, i64 %indvars.iv.next
  %1 = load i32, i32* %arrayidx2, align 4
  %add3 = add nsw i32 %1, %0
  %arrayidx5 = getelementptr inbounds [100 x i32], [100 x i32]* @A, i64 0, i64 %indvars.iv
  store i32 %add3, i32* %arrayidx5, align 4
  %exitcond = icmp ne i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.body, label %for.end

for.end:                                          ; preds = %for.body
  ret void
}

attributes #0 = { "pre_loopopt" }

define void @foo1() {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* @B, i64 0, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %arrayidx2 = getelementptr inbounds [100 x i32], [100 x i32]* @B, i64 0, i64 %indvars.iv.next
  %1 = load i32, i32* %arrayidx2, align 4
  %add3 = add nsw i32 %1, %0
  %arrayidx5 = getelementptr inbounds [100 x i32], [100 x i32]* @A, i64 0, i64 %indvars.iv
  store i32 %add3, i32* %arrayidx5, align 4
  %exitcond = icmp ne i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.body, label %for.end

for.end:                                          ; preds = %for.body
  ret void
}

