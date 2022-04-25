; RUN: opt < %s -analyze -enable-new-pm=0 -hir-ssa-deconstruction -hir-framework -hir-details 2>&1 | FileCheck %s
; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir>" -hir-details 2>&1 | FileCheck %s

; Verify that ztt of inner loop recognized and deferred by loop formation phase
; for processing due to structural issues is correctly set by framework pass.


; CHECK: + DO i64 i1 = 0, 1023, 1   <DO_LOOP>
; CHECK: |   %0 = (@a)[0][i1];
; CHECK: |   %N = (%Nptr)[0];
; CHECK: |
; CHECK: |   + Ztt: if (%N > 0)
; CHECK: |   + DO i64 i2 = 0, %N + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1024>
; CHECK: |   |   (@a)[0][i2] = 5;
; CHECK: |   + END LOOP
; CHECK: + END LOOP


@a = local_unnamed_addr global [1024 x i32] zeroinitializer, align 16

define void @foo(i64* %Nptr) {
entry:
  br label %for.body

for.body:                                         ; preds = %if.end18, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next.pre-phi, %if.end18 ]
  %arrayidx = getelementptr inbounds [1024 x i32], [1024 x i32]* @a, i64 0, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %1 = sext i32 %0 to i64
  %N = load i64, i64* %Nptr, align 8
  %cmp1 = icmp sle i64 %N, 0
  br i1 %cmp1, label %if.then, label %if.else14

if.then:                                          ; preds = %for.body
  %.pre = add nuw nsw i64 %indvars.iv, 1
  br label %if.end18

if.else14:                                        ; preds = %for.body
   br label %for.inner

for.inner:
  %iv = phi i64 [ 0, %if.else14 ], [ %iv.inc, %for.inner ]
  %iv.inc = add nuw nsw i64 %iv, 1
  %arrayidx1 = getelementptr inbounds [1024 x i32], [1024 x i32]* @a, i64 0, i64 %iv
  store i32 5, i32* %arrayidx1, align 4
  %exit.inner = icmp ne i64 %iv.inc, %N
  br i1 %exit.inner, label %for.inner, label %inner.exit

inner.exit:
  %.pre1 = add nuw nsw i64 %indvars.iv, 1
  br label %if.end18

if.end18:                                         ; preds = %inner.exit, %if.then
  %indvars.iv.next.pre-phi = phi i64 [ %.pre1, %inner.exit ], [ %.pre, %if.then ]
  %exitcond = icmp ne i64 %indvars.iv.next.pre-phi, 1024
  br i1 %exitcond, label %for.body, label %for.end

for.end:                                          ; preds = %if.end18
  ret void
}

