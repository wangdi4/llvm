; RUN: opt < %s -analyze -enable-new-pm=0 -scalar-evolution | FileCheck %s
; RUN: opt < %s -disable-output "-passes=print<scalar-evolution>" 2>&1 | FileCheck %s

; Verify that we are able to deduce the loop header phi as an AddRec by
; simplifying %indvars.iv.next.pre-phi whose operands have identical Scevs.
; This kind of IR is sometimes created by GVN.

; In addition, loop backedge taken count is recognized as a constant.

; TODO: fix missing no-wrap flags.
; CHECK: %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next.pre-phi, %if.end18 ]
; CHECK-NEXT: -->  {0,+,1}<%for.body>

; CHECK: %.pre = add nuw nsw i64 %indvars.iv, 1
; CHECK-NEXT:  -->  {1,+,1}<%for.body>

; CHECK: %.pre1 = add nuw nsw i64 %indvars.iv, 1
; CHECK-NEXT:  -->  {1,+,1}<%for.body>

; CHECK: Loop %for.body: backedge-taken count is 1023

@a = local_unnamed_addr global [1024 x i32] zeroinitializer, align 16

define void @foo(i32 %N) {
entry:
  %mul21 = shl nsw i32 %N, 1
  br label %for.body

for.body:                                         ; preds = %if.end18, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next.pre-phi, %if.end18 ]
  %arrayidx = getelementptr inbounds [1024 x i32], [1024 x i32]* @a, i64 0, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %1 = sext i32 %0 to i64
  %cmp1 = icmp slt i64 %1, %indvars.iv
  br i1 %cmp1, label %if.then, label %if.else14

if.then:                                          ; preds = %for.body
  %.pre = add nuw nsw i64 %indvars.iv, 1
  br label %if.end18

if.else14:                                        ; preds = %for.body
  %.pre1 = add nuw nsw i64 %indvars.iv, 1
  br label %if.end18

if.end18:                                         ; preds = %if.else14, %if.then
  %indvars.iv.next.pre-phi = phi i64 [ %.pre1, %if.else14 ], [ %.pre, %if.then ]
  %exitcond = icmp ne i64 %indvars.iv.next.pre-phi, 1024
  br i1 %exitcond, label %for.body, label %for.end

for.end:                                          ; preds = %if.end18
  ret void
}
