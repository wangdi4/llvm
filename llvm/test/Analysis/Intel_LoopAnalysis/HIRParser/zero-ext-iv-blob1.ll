; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Check parsing output for the loop verifying that the zero extended iv representation of %6: (2 * (zext i31 {-4,+,1}<%for.body> to i64)), is reverse engineered successfully into %6 blob.

; CHECK: + DO i1 = 0, 4, 1   <DO_LOOP>
; CHECK: |   %3 = (%ok)[0][2 * i1 + 1];
; CHECK: |   %0 = %0  -  %3;
; CHECK: |   if (2 * i1 + 1 < 2)
; CHECK: |   {
; CHECK: |      %4 = trunc.i64.i32(2 * i1 + 1);
; CHECK: |      %6 = 2 * i1 + 4294967289  &&  4294967294;
; CHECK: |      %.pre = (%ok)[0][3];
; CHECK: |      (%ok)[0][3] = %.pre + -22;
; CHECK: |      if (%6 >= 2)
; CHECK: |      {
; CHECK: |         %7 = (%ok)[0][2];
; CHECK: |         (%ok)[0][1] = %7;
; CHECK: |      }
; CHECK: |      %8 = (%ok)[0][2];
; CHECK: |      (%ok)[0][2] = %8 + -1 * (%4 * %0);
; CHECK: |      %9 = (%v4)[0][2 * i1 + 1][2 * i1 + 1];
; CHECK: |      %10 = (%up)[0];
; CHECK: |      (%up)[0] = -1 * %9 + %10;
; CHECK: |      (%gb)[0][1][2 * i1 + 2] = (((-1 * %9) + %10) * %0);
; CHECK: |   }
; CHECK: + END LOOP



; Function Attrs: nounwind uwtable
define void @main() {
entry:
  %ok = alloca [64 x i32], align 16
  %ee5 = alloca i32, align 4
  %ee5.promoted = load i32, i32* %ee5, align 4
  %gb = alloca [64 x [64 x i32]], align 16
  %up = alloca i32, align 4
  %v4 = alloca [64 x [64 x i32]], align 16
  %arrayidx8.phi.trans.insert = getelementptr inbounds [64 x i32], [64 x i32]* %ok, i64 0, i64 3
  %arrayidx20 = getelementptr inbounds [64 x i32], [64 x i32]* %ok, i64 0, i64 2
  %arrayidx13 = getelementptr inbounds [64 x i32], [64 x i32]* %ok, i64 0, i64 2
  %arrayidx16 = getelementptr inbounds [64 x i32], [64 x i32]* %ok, i64 0, i64 1
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc35
  %indvars.iv114 = phi i64 [ 1, %entry ], [ %indvars.iv.next115, %for.inc35 ]
  %indvars.iv112 = phi i32 [ 0, %entry ], [ %indvars.iv.next113, %for.inc35 ]
  %0 = phi i32 [ %ee5.promoted, %entry ], [ %sub, %for.inc35 ]
  %1 = sub i32 0, %indvars.iv112
  %2 = or i32 %1, 1
  %arrayidx = getelementptr inbounds [64 x i32], [64 x i32]* %ok, i64 0, i64 %indvars.iv114
  %3 = load i32, i32* %arrayidx, align 4
  %sub = sub i32 %0, %3
  %cmp4100 = icmp ult i64 %indvars.iv114, 2
  br i1 %cmp4100, label %for.body5, label %for.inc35

for.body5:                                        ; preds = %for.body
  %4 = trunc i64 %indvars.iv114 to i32
  %sub10 = add i64 %indvars.iv114, 4294967288
  %mul = mul i32 %sub, %4
  %arrayidx25 = getelementptr inbounds [64 x [64 x i32]], [64 x [64 x i32]]* %v4, i64 0, i64 %indvars.iv114, i64 %indvars.iv114
  %5 = add nuw nsw i64 %indvars.iv114, 1
  %6 = and i64 %sub10, 4294967294
  %.pre = load i32, i32* %arrayidx8.phi.trans.insert, align 4
  %sub9 = add i32 %.pre, -22
  store i32 %sub9, i32* %arrayidx8.phi.trans.insert, align 4
  %cmp11 = icmp ult i64 %6, 2
  br i1 %cmp11, label %for.inc35.loopexit, label %if.then

if.then:                                          ; preds = %for.body5
  %7 = load i32, i32* %arrayidx13, align 8
  store i32 %7, i32* %arrayidx16, align 4
  br label %for.inc35.loopexit

for.inc35.loopexit:                               ; preds = %if.then, %for.body5
  %8 = load i32, i32* %arrayidx20, align 8
  %sub21 = sub i32 %8, %mul
  store i32 %sub21, i32* %arrayidx20, align 8
  %9 = load i32, i32* %arrayidx25, align 4
  %10 = load i32, i32* %up, align 4
  %sub26 = sub i32 %10, %9
  store i32 %sub26, i32* %up, align 4
  %mul27 = mul i32 %sub, %sub26
  %arrayidx33 = getelementptr inbounds [64 x [64 x i32]], [64 x [64 x i32]]* %gb, i64 0, i64 1, i64 %5
  store i32 %mul27, i32* %arrayidx33, align 4
  br label %for.inc35

for.inc35:                                        ; preds = %for.inc35.loopexit, %for.body
  %dec.lcssa105 = phi i32 [ 2, %for.body ], [ %2, %for.inc35.loopexit ]
  %indvars.iv.next115 = add nuw nsw i64 %indvars.iv114, 2
  %cmp = icmp ult i64 %indvars.iv.next115, 11
  %indvars.iv.next113 = add nsw i32 %indvars.iv112, -2
  br i1 %cmp, label %for.body, label %for.end37

for.end37:
  ret void
}
